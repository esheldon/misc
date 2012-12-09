#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_NAME_SIZE 255
#define SEND_DATA_TAG 2001
#define RETURN_DATA_TAG 2002
#define DONE_TAG 0

int main(int argc, char **argv)
{

    char command[MAX_NAME_SIZE]={0};
    int ierr=0, this_id=0, n_procs=0;
    int worker=0;
    int nrunning=0;
    int imess=0; // dummy
    int master=0;
    MPI_Status status;

    ierr=MPI_Init(&argc, &argv);

    ierr=MPI_Comm_rank(MPI_COMM_WORLD, &this_id);
    ierr=MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    fprintf(stderr,"I'm process %d of %d\n", this_id, n_procs);
    if (n_procs == 1) {
        fprintf(stderr,"need > 1 processes\n");
        goto _finish;
    }

    if (this_id == master) {
        int have_command=(1==fscanf(stdin,"%s", command));
        nrunning=0;
        while (have_command || nrunning > 0) {
            if (have_command) {
                fprintf(stderr,"master: command: '%s'\n", command);
            }

            if (!have_command || nrunning >= (n_procs-1)) {
                // either draining because no commands left
                // or the queue is full
                fprintf(stderr,"master: waiting\n");
                ierr = MPI_Recv(&imess, 
                                1, 
                                MPI_INT,
                                MPI_ANY_SOURCE, 
                                MPI_ANY_TAG, 
                                MPI_COMM_WORLD, 
                                &status);
                worker = status.MPI_SOURCE;
                fprintf(stderr,"master: recv from %d\n", worker);
                nrunning--;

                // if we are out of commands, tell this worker
                // we are done
                if (!have_command) {
                    ierr = MPI_Send("done", 
                                    5, 
                                    MPI_CHAR,
                                    worker, 
                                    DONE_TAG,
                                    MPI_COMM_WORLD);
                }
            } else {
                // yet to use all workers: send to next in order
                worker += 1;
            }
            if (have_command) {
                fprintf(stderr,"master: sending to %d\n", worker);
                ierr = MPI_Send(command, 
                                MAX_NAME_SIZE, 
                                MPI_CHAR,
                                worker, 
                                SEND_DATA_TAG, 
                                MPI_COMM_WORLD);
                nrunning++;
                have_command=(1==fscanf(stdin,"%s", command));
            }
        }

    } else {
        while (1) {
            ierr = MPI_Recv(command, 
                    MAX_NAME_SIZE, 
                    MPI_CHAR,
                    master, 
                    MPI_ANY_TAG, 
                    MPI_COMM_WORLD, 
                    &status);

            if (status.MPI_TAG==DONE_TAG) {
                goto _finish;
            }

            system(command);

            ierr = MPI_Send(&imess, // dumy
                            1, 
                            MPI_INT,
                            master, 
                            DONE_TAG,
                            MPI_COMM_WORLD);
        }
    }

_finish:
    ierr=MPI_Finalize();

    if (ierr != 0) {
        // for the warnings
        fprintf(stderr,"Encountered error: %d\n", ierr);
    }
}

