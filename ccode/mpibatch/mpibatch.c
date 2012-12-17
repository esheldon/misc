/*

   Use mpi to read and execute commands from standard input, one per line.

   Examples using 4 processes

     Read commands from a file
       mpirun -np 4 mpibatch < list-of-commands.txt

     Run all shell scripts in the current directory
       ls *.sh | mpirun -np 4 mpibatch

   The commands are executed using 
   
     system(command)  

   This is equivalent to running

     /bin/sh -c "command" 

   from the shell.  Because of this, blank lines or fully commented lines are
   perfectly valid but no useful work is done.

   Requirements
     getline() - this is present in gcc with the -std=gnu99 but 
       not for -std=c99 because it is not part of the standard.

   Author Erin Sheldon, BNL
   Inspired by mpibatch.f by Peter Nugent, LBL

*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SEND_DATA_TAG 2001
#define RETURN_DATA_TAG 2002
#define MASTER 0
#define DONE 0

int run_master(int n_workers)
{
    char *command=NULL;
    size_t size=0;
    int done=0, worker=0, ierr=0, imess=0; // dummy
    MPI_Status status;

    int nrunning=0;
    int nread=getline(&command,&size,stdin); // includes newline
    int have_command=(nread>0);
    while (have_command || nrunning > 0) {
        command[nread-1]='\0';  // to cut junk left at end of string
        if (!have_command || nrunning >= n_workers) {
            // no commands left or queue is full: wait for a worker
            ierr = MPI_Recv(&imess, 1, MPI_INT, MPI_ANY_SOURCE, 
                            MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            worker = status.MPI_SOURCE;
            nrunning--;

            if (!have_command) {
                // are out of commands, tell this worker we are done
                ierr = MPI_Send(&done, 1, MPI_INT, worker,
                                SEND_DATA_TAG, MPI_COMM_WORLD);
            }
        } else {
            worker += 1; // first run through workers: call in order
        }
        if (have_command) {
            ierr = MPI_Send(&nread, 1, MPI_INT, worker,
                            SEND_DATA_TAG, MPI_COMM_WORLD);
            ierr = MPI_Send(command, nread, MPI_CHAR, worker,
                            SEND_DATA_TAG, MPI_COMM_WORLD);
            nrunning++;

            nread=getline(&command,&size,stdin); // includes newline
            have_command=(nread>0);
        }
    }
    free(command);
    return ierr;
}

int run_worker() {
    char *command=NULL;
    MPI_Status status;
    int size=0, cmdsize=0, ierr=0, imess=0; // imess is dummy
    while (1) {
        ierr = MPI_Recv(&cmdsize, 1, MPI_INT, MASTER,
                        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (cmdsize==DONE) {
            break;    // 0 is a signal we are done
        }
        if (cmdsize > size) {
            command=realloc(command, 2*cmdsize);
            size=2*cmdsize;
        }
        ierr = MPI_Recv(command, size, MPI_CHAR, MASTER,
                        MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        system(command);

        ierr = MPI_Send(&imess, 1, MPI_INT, MASTER, SEND_DATA_TAG,
                        MPI_COMM_WORLD);
    }
    free(command);
    return ierr;
}

int main(int argc, char **argv)
{
    int this_id=0, n_procs=0, n_workers=0;
    int status=0, ierr=0; // dummy

    ierr=MPI_Init(&argc, &argv);

    ierr=MPI_Comm_rank(MPI_COMM_WORLD, &this_id);
    ierr=MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    n_workers = n_procs-1;

    if (n_procs > 1) {
        if (this_id == MASTER) {
            ierr = run_master(n_workers);
        } else {
            ierr = run_worker();
        }
    } else {
        status=EXIT_FAILURE;
        fprintf(stderr,"need > 1 processes (master+workers)\n");
    }

    ierr=MPI_Finalize();

    if (ierr != 0) {
        status=EXIT_FAILURE;
        fprintf(stderr,"Encountered error: %d\n", ierr);
    }
    return status;
}

