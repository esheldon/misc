/*

   Read and execute commands from standard input, one per line.

   Examples for 4 processes

   Read commands from a file
     mpirun -np 4 mpibatch < list-of-commands.txt

   Run all shell scripts in the current directory
     ls *.sh | mpirun -np 4 mpibatch

*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_NAME_SIZE 255
#define SEND_DATA_TAG 2001
#define RETURN_DATA_TAG 2002
#define DONE_TAG 0
#define MASTER 0

int run_master(int n_workers)
{
    //char command[MAX_NAME_SIZE]={0};
    char *command=NULL;
    size_t size=0;
    int worker=0, ierr=0, imess=0; // dummy
    MPI_Status status;

    int nrunning=0;
    int nread=getline(&command,&size,stdin);
    int have_command=(nread>0);
    while (have_command || nrunning > 0) {

        if (nread > MAX_NAME_SIZE) {
            fprintf(stderr,"command too long: %d\n", nread);
            break;
        }
        command[nread-1]='\0';  // nread-1 is the newline
        if (!have_command || nrunning >= (n_workers)) {
            // either no commands left or the queue is full
            ierr = MPI_Recv(&imess, 1, MPI_INT, MPI_ANY_SOURCE, 
                            MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            worker = status.MPI_SOURCE;
            nrunning--;

            // if we are out of commands, tell this worker we are done
            if (!have_command) {
                ierr = MPI_Send("nothing", 8, MPI_CHAR, worker, DONE_TAG,
                                MPI_COMM_WORLD);
            }
        } else {
            // yet to use all workers: send to next in order
            worker += 1;
        }
        if (have_command) {
            ierr = MPI_Send(command, nread, MPI_CHAR, worker,
                            SEND_DATA_TAG, MPI_COMM_WORLD);
            nrunning++;

            nread=getline(&command,&size,stdin);
            have_command=(nread>0);
        }
    }
    free(command);
    return ierr;
}

int run_worker() {
    char command[MAX_NAME_SIZE]={0};
    MPI_Status status;
    int ierr=0, imess=0; // imess is dummy
    while (1) {
        ierr = MPI_Recv(command, MAX_NAME_SIZE, MPI_CHAR, MASTER,
                MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG==DONE_TAG) {
            break;
        }

        system(command);

        ierr = MPI_Send(&imess, 1, MPI_INT, MASTER, DONE_TAG,
                MPI_COMM_WORLD);
    }
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

