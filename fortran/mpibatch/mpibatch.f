      program callit

      use mpi

      implicit none
!
!     Code to parallelize a set of independent serial commands
!     via an mpi-wrapper. Assumes that the commands are < 150 
!     characters and a list of commands that are < 100000 long
!     Commands are in an ascii file and get converted to a
!     temporary namelist file via the perlscript mknamelist.pl
!
!
      integer, parameter :: nmax = 50000, ncr=150
      character(len=ncr) :: cmd(nmax), blk, cmdex
      character(len=4) :: done
      character(len=120) :: filename
      integer :: ncmd, i, j, k, istat

!
      integer ::  ierr, mytid, nproc, master, numsent, sender
      integer :: status(MPI_STATUS_SIZE)
!
      namelist /listcmd/ cmd

!
!       Obtain number of tasks and task ID
!
      call mpi_init(ierr)
      call mpi_comm_rank(MPI_COMM_WORLD, mytid, ierr)
      call mpi_comm_size(MPI_COMM_WORLD, nproc, ierr)
      master = 0
      done = 'done'
!
!
!
!
      if (mytid == master) then
!
!        master initializes and then dispatches         
!
         do i = 1, ncr
          blk(i:i) = ' '
         end do  

         cmd = blk

         !read(*,*) filename
         read(*,'(a)') filename

         write(*,*)"filename: ",filename
      
         open(unit=10,file=trim(filename),status='old')
         read(10, NML=listcmd)
         close(10)

         ncmd = 0

         do i = 1, nmax
            if (cmd(i) /= blk) then
               ncmd = ncmd + 1
            end if
         end do

         write(*,*)"ncmd: ",ncmd

         call MPI_BCAST(ncmd, 1, MPI_INTEGER, master, MPI_COMM_WORLD, ierr)

         numsent = 0

         do i = 1, min(nproc-1,ncmd)

            call MPI_SEND(cmd(i), ncr, MPI_CHARACTER, i, &
            i, MPI_COMM_WORLD, ierr)
            numsent = numsent+1

         end do

         do i = 1,ncmd

            call MPI_RECV(done, 4, MPI_CHARACTER, &
            MPI_ANY_SOURCE, MPI_ANY_TAG, &
            MPI_COMM_WORLD, status, ierr)
            sender     = status(MPI_SOURCE)    

            if (numsent < ncmd) then ! send another command

               call MPI_SEND(cmd(numsent+1), ncr, MPI_CHARACTER, &
               sender, numsent+1, MPI_COMM_WORLD, ierr)
               numsent = numsent+1

            else                ! Tell sender that there is no more work

               call MPI_SEND(done, 4, MPI_CHARACTER, &
               sender, 0, MPI_COMM_WORLD, ierr)

            endif

         end do

      else

!
!        Slaves spit out the commands until done message received
!        First lets get the total number of commands
!
         call MPI_BCAST(ncmd, 1, MPI_INTEGER, master, MPI_COMM_WORLD, ierr)
         
!        skip if more processes than work

         if (mytid > ncmd) goto 200  

 90      call MPI_RECV(cmdex, ncr, MPI_CHARACTER, master, &
         MPI_ANY_TAG, MPI_COMM_WORLD, status, ierr)
         

         if (status(MPI_TAG) == 0) then
            go to 200

         else

	    j = status(MPI_TAG)

            call system(trim(cmdex))

            call MPI_SEND(done, 4, MPI_CHARACTER, master, &
                          j, MPI_COMM_WORLD, ierr)
            go to 90
         endif

 200     continue

       endif
        
       call MPI_FINALIZE(ierr)

      end
      
