!--------------------------------------------------
!     This program :
!        Calculates weights for training set objects such
!        that the weighted training set distributions match
!        the photometric sample distributions.
!     INPUTS:
!        Reads in:
!           1) Training set filename
!           2) Number of letters in the filename
!           3) Photometric sample filename
!           4) Number of letters in the filename
!           5) variable: dotrain = 0 or 1
!             * If 0 -> will NOT find neighbors of training set, but simply
!               read their distances from a file.
!             * If 1 -> will go through the calculation for the training set.
!           6) variable: docolor = 0 or 1
!             * If 0 -> will compute/read distances using magnitudes.
!             * If 1 -> will compute/read distances using colors.           
!           7) variable: fixneigh = 0 or 1
!             * If 0 -> fixed volume. Will use volume defined by nneighbors
!               to find number of objects in photometric sample.
!             * If 1 -> fixed number of neighbors. Will use the same number
!               of neighbors both in the training set and photometric sample.
!           8) Number of nearest neighbors 
!     OUTPUTS:
!         (1)File with distances of training set objects to furtherest neighbor.
!            This can be used for future computations setting dotrain=0.
!         (2)File with zphot, zspec, weights, magnitudes for training set 
!            objects.
!--------------------------------------------------      
      
Program weightgal

    ! Function declarations
    integer strlen

    ! parameters
    parameter (n=1000000)

    ! variable declarations
    character trainfile*200,trainfilev*200
    character photofile*200,photofilev*200
    character outfile*200,outfilev*200
    character nameaux*200,nameaux1*200,nameaux2*200
    character name*200,namedist*200

    integer ndat,ntot,nt,auxint
    integer nfilters
    integer nlettrain,nlettrainv,nletphot,nletphotv,nletout,nletoutv 
    integer nletphotboot,nletphotvboot 
    integer ntrain, nphot,ntotal,ntotalt,ntotalp

    real*8 mag1_t(n),mag2_t(n),mag3_t(n),mag4_t(n),mag5_t(n)
    real*8 mag1_p(n),mag2_p(n),mag3_p(n),mag4_p(n),mag5_p(n)
    real*8 mag1_n(n),mag2_n(n),mag3_n(n),mag4_n(n),mag5_n(n)

    real*8 nada(n)

    !      real*8 u(n,ncoefmax),v(ncoefmax,ncoefmax),w(ncoefmax)
    real*8 no

    integer i,j,k,l,o,oaux,ind

    real*8 zphot(n),zphot_t(n),zphot_p(n),zphot_n(n)
    real*8 zspec(n),zspec_t(n),zspec_p(n),zspec_n(n)

    real*8 num,ntotalr

    INTEGER iwksp(n)
    REAL*8 wksp(n)
    integer index(n)

    integer nneighbor,nneighborfile
    real*8 nneighborreal

    real*8 distance(n),heap(n),distance_n(n)
    integer aux
    real*8 zspec_n_max,zspec_n_min

    real*8 dist_max_t(n),dist_max_p(n)
    real*8 dens_t(n),dens_p(n)
    real*8 weight(n),weight_tot

    real*8 number1(10000),number2(10000),number3(10000),number4(10000)
    real*8 number5(10000),numberz(10000)
    real*8 numberreal(n)
    real*8 nbins, binmin,binmax,binsize,bin
    real*8 nbinsz,binminz,binmaxz,binsizez,binz

    integer dotrain,docolor,docolorfile,fixvol
    real*8 jreal


    ! Temporary holders for numerical command line arguments
    character cdt*4, cdc*4, cfv*4, cnn*4
    integer narg




    !cccccccccccccccccccccccccccc
    !     READING IN PARAMETERS
    !cccccccccccccccccccccccccccc

    narg=iargc()
    if (narg.ne.7) then
        write(*,'(a,$)')"usage: weightgal trainfile photofile dotrain"
        write(*,*)" docolor fixvol neighbor outfile"
        stop
    endif


    write(*,'(A,$)')'Name of the file where training set is : '
    call getarg(1, trainfile)
    nlettrain = strlen(trainfile)
    write(*,*)trainfile(1:nlettrain)

    write(*,'(A,$)')'Name of the file where photometric set is : '
    call getarg(2,photofile)
    nletphot=strlen(photofile)
    write(*,*)photofile(1:nletphot)

    write(*,'(A,$)')'COMPUTING training distance? 1 or 0 : '
    call getarg(3,cdt)
    read(cdt, *)dotrain
    write(*,*)dotrain
    if (dotrain.ne.0.and.dotrain.ne.1) then
        write(*,'(A,$)')'Invalid entry. Has to be either 1 or 0.'
        write(*,'(A,$)')'Please start over...'
        stop
    end if

    write(*,'(A,$)')'Computing/reading distances in COLOR space?'
    write(*,'(A,$)')' 1 or 0 : '
    call getarg(4,cdc)
    read(cdc, *)docolor
    write(*,*)docolor
    if (docolor.ne.0.and.docolor.ne.1) then
        write(*,'(A,$)')'Invalid entry. Has to be either 1 or 0.'
        write(*,'(A,$)')'Please start over...'
        stop
    end if

    write(*,'(A,$)')'FIXED VOLUME from training neighbors? 1 or 0 : '
    call getarg(5,cfv)
    read(cfv,*)fixvol
    write(*,*)fixvol
    if (fixvol.ne.0.and.fixvol.ne.1) then
        write(*,'(A,$)')'Invalid entry. Has to be either 1 or 0.'
        write(*,'(A,$)')'Please start over...'
        stop
    end if

    write(*,'(A,$)')'NUMBER of Nearest Neighbors : '
    call getarg(6,cnn)
    read(cnn,*)nneighbor
    write(*,*)nneighbor
    if (nneighbor.le.0) then
        write(*,'(A,$)')'Invalid entry. Has to be a positive number.'
        write(*,'(A,$)')'Please start over...'
        stop
    end if
    if (nneighbor.gt.100) then
        write(*,'(A,$)')'Careful...'
        write(*,'(A,$)')'Number of neighbors (>100) might be too big'
        write(*,'(A,$)')'Computation will proceed, but check results.'
    end if


    write(*,'(A,$)')'Output File: '
    call getarg(7,outfile)
    nletout=strlen(outfile)
    write(*,*)outfile(1:nletout)

    nfilters=5  ! Standard SDSS

    !cccccccccccccccccccccccccccc
    ! Assigning file names to variables. Not sure why marcos did this.
    !cccccccccccccccccccccccccccc

    trainfilev=trainfile(1:nlettrain)
    nlettrainv=nlettrain
    photofilev=photofile(1:nletphot)
    nletphotv=nletphot

    outfilev=outfile(1:nletout)
    nletoutv=nletout

    !cccccccccccccccccccccccccccc
    ! Assigning name to file where training set distances will be 
    ! written to/read from.
    !cccccccccccccccccccccccccccc

    if (docolor.eq.0) then
        !        namedist=outfilev(1:nletout)//".mag-train.dat"
        namedist=trainfile(1:nlettrain)//".mag-train.dat"
    else
        !        namedist=outfilev(1:nletout)//".col-train.dat"
        namedist=trainfile(1:nlettrain)//".col-train.dat"
    end if

    write(*,*)"train file: ",namedist

    !ccccccccccccccccccccccccccccccccccccccccccccccccccc
    !ccccccccccccccccccccccccccccccccccccccccccccccccccc
    ! Start computation/reading of neighbor distances for TRAINING SET
    !ccccccccccccccccccccccccccccccccccccccccccccccccccc
    !ccccccccccccccccccccccccccccccccccccccccccccccccccc      

    name=trainfilev(1:nlettrainv)
    call reading(name,nlettrainv,n,ntrain,zspec_t,zphot_t,mag1_t,mag2_t,mag3_t,mag4_t,mag5_t)

    !      write(*,*)zspec_t(1),zphot_t(1),
    !     *mag1_t(1),mag2_t(1),mag3_t(1),mag4_t(1),mag5_t(1)
    !      write(*,*)ntrain,nneighbor

    !----------------------------
    ! Check if calculating or just reading from file
    !----------------------------
    IF (dotrain.eq.1) THEN
        write(*,'(A,$)')'Computing distances for training set'
        write(*,*)
        !cccccc
        ! if calculating 
        ! open file where distances will be written
        ! record number of neighbors being used
        !cccccc       
        open(1,file=namedist)
        !        open(1,file='dist_train.dat')
        write(1,*)nneighbor,docolor 

        auxint=0
        DO i=1,ntrain         ! Loop over training set objects
            !----------------------------
            ! Showing progress
            !----------------------------
            por=i
            por=(por/ntrain)

            if (por.gt.0.01.and.auxint.le.0) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.05.and.auxint.le.1) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.1.and.auxint.le.2) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.25.and.auxint.le.3) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.5.and.auxint.le.4) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.75.and.auxint.le.5) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.95.and.auxint.le.6) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            if (por.gt.0.99.and.auxint.le.7) then
                auxint=auxint+1
                write(*,*) "percentage done = ", por*100, " %" 
                call flush()
            end if

            !----------------------------
            ! calculating distances
            !----------------------------

            !ccccc
            ! Initializing
            !ccccc

            do j=1,ntrain
                distance(j)=0.0
            end do
            !ccccc
            ! going through
            !ccccc
            do j=1,ntrain           ! Loop over training neighbor candidates
                if (j.ne.i) then      ! checking that it is not the self.
                    if (docolor.eq.0) then ! checking if magnitude or color
                        distance(j)= &
                             +(mag1_t(i)-mag1_t(j))**2 + (mag2_t(i)-mag2_t(j))**2 &
                             +(mag3_t(i)-mag3_t(j))**2 + (mag4_t(i)-mag4_t(j))**2 &
                             +(mag5_t(i)-mag5_t(j))**2
                    else
                        distance(j)=  &
                             (mag1_t(i)-mag2_t(i) - mag1_t(j)+mag2_t(j))**2   &
                            +(mag2_t(i)-mag3_t(i) - mag2_t(j)+mag3_t(j))**2  &
                            +(mag3_t(i)-mag4_t(i) - mag3_t(j)+mag4_t(j))**2  &
                            +(mag4_t(i)-mag5_t(i) - mag4_t(j)+mag5_t(j))**2  
                    end if
                    distance(j)=1./distance(j)
                else
                    distance(j)=200000000.0
                end if
            end do

            !ccccc
            ! finding neighbors
            !ccccc
            call hpsel(nneighbor,ntrain,distance,heap,index)
            !ccccc
            ! writing who they are
            !ccccc
            do j=1,nneighbor
                mag1_n(j)=mag1_t(index(j))
                mag2_n(j)=mag2_t(index(j))
                mag3_n(j)=mag3_t(index(j))
                mag4_n(j)=mag4_t(index(j))
                mag5_n(j)=mag5_t(index(j))
                zspec_n(j)=zspec_t(index(j))
                zphot_n(j)=zphot_t(index(j))
                distance_n(j)=1./distance(index(j))
            end do

            !ccccc
            ! sorting them
            !ccccc
            call sort6(nneighbor,distance_n,mag1_n,mag2_n,mag3_n,mag4_n,mag5_n,wksp,iwksp)

            !cccccc
            ! computing distance and density and writing file
            !cccccc

            dist_max_t(i)=distance_n(nneighbor)**0.5
            nneighborreal=nneighbor
            if (docolor.eq.0) then
                dens_t(i)=nneighborreal/(dist_max_t(i)**(nfilters))
            else
                dens_t(i)=nneighborreal/(dist_max_t(i)**(nfilters-1))
            end if
            write(1,50)i,dist_max_t(i)
        END DO

        close(1) 
    ELSE
        !--------------------------
        ! If no computation, just read from file
        !--------------------------
        write(*,'(A,$)')'Will just read file for training'
        write(*,*)

        !        open(2,file='dist_train.dat')
        open(2,file=namedist)
        read(2,*)nneighborfile,docolorfile
        !ccccc
        ! Check that the number of neighbors used in the distance of training
        !    set objects is the same you are using...
        !
        ! Check that the distance of training set objects is calculated in
        !    the same space (magnitude/color) you are using...
        !ccccc
        if (nneighborfile.ne.nneighbor) then
            write(*,'(A,$)')'Number of neighbors in training set' 
            write(*,'(A,$)')'distance file does not match current' 
            write(*,'(A,$)')'number of neighbors!'
            write(*,'(A,$)')'Please start over...'
            stop
        end if

        if (docolorfile.ne.docolor) then
            write(*,'(A,$)')'Space (magnitude/color) where distances'
            write(*,'(A,$)')'were calculated in training set does not' 
            write(*,'(A,$)')'match current space!'
            write(*,'(A,$)')'Please start over...'
            stop
        end if

        DO i=1,ntrain
            read(2,*)no,dist_max_t(i)
        END DO
        close(2)


        nneighborreal=nneighborfile
        DO i=1,ntrain
            if (docolor.eq.0) then
                dens_t(i)=nneighborreal/(dist_max_t(i)**(nfilters))
            else
                dens_t(i)=nneighborreal/(dist_max_t(i)**(nfilters-1))
            end if
        END DO

        !        write(*,*)'check no problems distance', 
        !     *dist_max_t(1),dist_max_t(ntrain)
        !        write(*,*)'check no problems density', 
        !     *dens_t(1),dens_t(ntrain)

    END IF

50  format(i10,1e15.5)

    !      stop


    !cccccccccccccccccccccccccccccccccccccccccccccc
    !cccccccccccccccccccccccccccccccccccccccccccccc
    !     Start computation for PHOTOMETRIC SAMPLE
    !cccccccccccccccccccccccccccccccccccccccccccccc
    !cccccccccccccccccccccccccccccccccccccccccccccc      

    write(*,'(A,$)')'Computing distances for photometric sample'
    write(*,*)

    name=photofilev(1:nletphotv)
    call reading(name,nletphotv,n,nphot,zspec_p,zphot_p,mag1_p,mag2_p,mag3_p,mag4_p,mag5_p)


    !      write(*,*)zspec_p(1),zphot_p(1),
    !     *mag1_p(1),mag2_p(1),mag3_p(1),mag4_p(1),mag5_p(1)
    !      write(*,*)nphot,nneighbor

    !----------------------------
    !     Start calculation
    !----------------------------  
    auxint=0
    DO i=1,ntrain           ! Loop over training set objects
        !----------------------------
        ! Showing progress
        !----------------------------
        por=i
        por=(por/ntrain)


        if (por.gt.0.01.and.auxint.le.0) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.05.and.auxint.le.1) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %"
            call flush()
        end if

        if (por.gt.0.1.and.auxint.le.2) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.25.and.auxint.le.3) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.5.and.auxint.le.4) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.75.and.auxint.le.5) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.95.and.auxint.le.6) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        if (por.gt.0.99.and.auxint.le.7) then
            auxint=auxint+1
            write(*,*) "percentage done = ", por*100, " %" 
            call flush()
        end if

        !------------------------------
        ! calculating distance to photo object j
        !------------------------------
        !ccccc
        ! Initializing
        !ccccc
        do j=1,nphot
            distance(j)=0.0
        end do
        !ccccc
        ! going through
        !ccccc
        do j=1,nphot           ! Loop over photometric neighbor candidates
            if (docolor.eq.0) then  ! checking if magnitude or colors
                distance(j)= &
                     +(mag1_t(i)-mag1_p(j))**2 + (mag2_t(i)-mag2_p(j))**2 &
                     +(mag3_t(i)-mag3_p(j))**2 + (mag4_t(i)-mag4_p(j))**2 &
                     +(mag5_t(i)-mag5_p(j))**2
            else
                distance(j)= &
                     (mag1_t(i)-mag2_t(i)-mag1_p(j)+mag2_p(j))**2  &
                     +(mag2_t(i)-mag3_t(i)-mag2_p(j)+mag3_p(j))**2 &
                     +(mag3_t(i)-mag4_t(i)-mag3_p(j)+mag4_p(j))**2 &
                     +(mag4_t(i)-mag5_t(i)-mag4_p(j)+mag5_p(j))**2 
            end if
            distance(j)=1./distance(j)
        end do

        !ccccc
        ! Check if fixed number of neighbors or fixed volume
        !ccccc
        IF (fixvol.eq.0) THEN
            !ccccc
            ! finding neighbors
            !ccccc
            call hpsel(nneighbor,nphot,distance,heap,index)
            !ccccc
            ! writing who they are
            !ccccc
            do j=1,nneighbor
                mag1_n(j)=mag1_p(index(j))
                mag2_n(j)=mag2_p(index(j))
                mag3_n(j)=mag3_p(index(j))
                mag4_n(j)=mag4_p(index(j))
                mag5_n(j)=mag5_p(index(j))
                zspec_n(j)=zspec_p(index(j))
                zphot_n(j)=zphot_p(index(j))
                distance_n(j)=1./distance(index(j))
            end do

            !ccccc
            ! sorting them
            !ccccc
            call sort6(nneighbor,distance_n,mag1_n,mag2_n,mag3_n,mag4_n,mag5_n,wksp,iwksp)

            !cccccc
            ! computing distance and density
            !cccccc

            dist_max_p(i)=distance_n(nneighbor)**0.5
            nneighborreal=nneighbor
            if (docolor.eq.0) then
                dens_p(i)=nneighborreal/(dist_max_p(i)**(nfilters))
            else
                dens_p(i)=nneighborreal/(dist_max_p(i)**(nfilters-1))
            end if

        ELSE
            !cccccc
            ! just finding number in fixed volume/distance determined by training neighbors
            !cccccc

            dist_max_p(i)=dist_max_t(i)
            numberreal(i)=0.0
            do j=1,nphot
                distance(j)=1.0/distance(j)**0.5 
                if (distance(j).lt.dist_max_p(i)) then
                    numberreal(i)=numberreal(i)+1.0
                end if
            end do
            if (docolor.eq.0) then
                dens_p(i)=numberreal(i)/(dist_max_p(i)**(nfilters))
            else
                dens_p(i)=numberreal(i)/(dist_max_p(i)**(nfilters-1))
            end if

        END IF

        !cccccc
        ! computing weights
        !cccccc
        nphotrel=nphot
        weight(i)=1./nphot*dens_p(i)/dens_t(i)

    END DO

    !      write(*,*)'frango1',dist_max_p(1)
    !      write(*,*)'frango2',distance(1),distance(2)
    !      write(*,*)'frango3',numberreal(1)

    !      write(*,*)'check no problems number', 
    !     *numberreal(1),numberreal(ntrain)
    !      write(*,*)'check no problems density', 
    !     *dens_p(1),dens_p(ntrain)  



    !---------
    !    scaling back magnitudes to normal values and 
    !    creating table file with zphot, zspec, weight and magnitudes
    !---------

    call scalebackall(n,nphot,mag1_p,mag2_p,mag3_p,mag4_p,mag5_p)

    call etable(outfile, n,ntrain, zphot_t,zspec_t,weight, mag1_t,mag2_t,mag3_t,mag4_t,mag5_t)



67  format(100f15.8)
end program weightgal


!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
! SUBROUTINES
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc


! Get the length of a string, where blanks at the end are not
! counted as part of the string.  C is petter with the nulls.

integer function strlen(st)
    character st*(*)
    integer i
    
    i=len(st)
    
    do while(st(i:i) .eq. ' ')
        i = i-1
    enddo
    strlen=i
    return
end function strlen

!----------------------------------------------------------------------
!     Reading in file
!----------------------------------------------------------------------
subroutine reading(name,nlet,n,ntrain,zspec2,zphot2, mag1,mag2,mag3,mag4,mag5)

    character name*200
    integer n,ntrain,i,nlet
    external scale,scale2
    real*8 zphot2(n)
    real*8 zspec2(n)

    real*8 mag1(n),mag2(n),mag3(n),mag4(n),mag5(n)
    
    
    real*8 auxreal    
    open(1,file=name)
    do i=1,n
        read(1,*,end=3)zspec2(i),zphot2(i),mag1(i),mag2(i),mag3(i),mag4(i),mag5(i)
        
    end do

3   ntrain=i-1
    
    write(*,*)'Number in file: ',ntrain
    close(1)
    
    !cccc
    ! scaling to work with  smaller magnitude numbers
    ! see subroutine "scale" bellow to see how magnitudes are scaled.
    !cccc

    do i=1,ntrain

        auxreal=mag1(i)
        call scale(auxreal,auxreal)
        mag1(i)=auxreal 

        auxreal=mag2(i)
        call scale(auxreal,auxreal)
        mag2(i)=auxreal 

        auxreal=mag3(i)
        call scale(auxreal,auxreal)
        mag3(i)=auxreal 

        auxreal=mag4(i)
        call scale(auxreal,auxreal)
        mag4(i)=auxreal 
        
        auxreal=mag5(i)
        call scale(auxreal,auxreal)
        mag5(i)=auxreal 

    end do

    return
end subroutine reading

!----------------------------------------------------------------------
!     Scaling
!----------------------------------------------------------------------
subroutine scale(gin,gout)
    real*8 gin,gout
    real*8 magmin,magmax

    magmin=10.0
    magmax=32.0

    gout=(gin-magmin)/(magmax-magmin)                ! 0 at magmin 1 at magmax
    !      gout=2./(magmax-magmin)*(gin-(magmax+magmin)/2.) !-1 at magmin 1 at magmax

    return
end subroutine scale

!----------------------------------------------------------------------
!     Scaling back
!----------------------------------------------------------------------
subroutine scaleback(gin,gout)
    real*8 gin,gout    
    real*8 magmin,magmax

    magmin=10.0
    magmax=32.0

    gout=gin*(magmax-magmin)+magmin                  !  0 and 1
    !      gout=gin*(magmax-magmin)/2. +(magmax+magmin)/2.  ! -1 and 1

    return
end subroutine scaleback

!----------------------------------------------------------------------
!     Another possible scaling
!----------------------------------------------------------------------
subroutine scale2(gin,gout)
    real*8 gin,gout
    real*8 magmin,magmax

    magmin=0.0
    magmax=10.0

    gout=(gin-magmin)/(magmax-magmin)                ! 0 at magmin 1 at magmax      
    !      gout=2./(magmax-magmin)*(gin-(magmax+magmin)/2.) !-1 at magmin 1 at magmax

    return
end subroutine scale2

!----------------------------------------------------------------------
!     Another scaling back
!----------------------------------------------------------------------
subroutine scale2back(gin,gout)
    real*8 gin,gout    
    real*8 magmin,magmax

    magmin=0.0
    magmax=10.0

    gout=gin*(magmax-magmin)+magmin                  !  0 and 1
    !      gout=gin*(magmax-magmin)/2. +(magmax+magmin)/2.  ! -1 and 1

    return
end subroutine scale2back


!----------------------------------------------------------------------
!     Scaling bad
!----------------------------------------------------------------------
subroutine scalebad(gin,gout)
    real*8 gin,gout
    real*8 magmin,magmax

    magmin=10.0
    magmax=100.0

    gout=(gin-magmin)/(magmax-magmin)                ! 0 at magmin 1 at magmax      
    !      gout=2./(magmax-magmin)*(gin-(magmax+magmin)/2.) !-1 at magmin 1 at magmax

    return
end subroutine scalebad

!----------------------------------------------------------------------
!     Scaling bad back
!----------------------------------------------------------------------
subroutine scalebadback(gin,gout)
    real*8 gin,gout    
    real*8 magmin,magmax,factor

    magmin=10.0
    magmax=100.0

    gout=gin*(magmax-magmin)+magmin                  !  0 and 1
    !      gout=gin*(magmax-magmin)/2. +(magmax+magmin)/2.  ! -1 and 1
    return
end subroutine scalebadback

!---------------------------------------------------------------
!     Subroutine to find "m" nearest neighbors from vector arr(n)
!     Returns the index vector index(m) with the neighbors index.
!     Neighbors are not sorted tough, but you know who they are.
!     From Numerical Recipes.
!---------------------------------------------------------------
      SUBROUTINE hpsel(m,n,arr,heap,index)
      INTEGER m,n
      REAL*8 arr(n),heap(m),wksp(m),indexreal(m)
!U    USES sort
      INTEGER i,j,k,index(m),iwksp(m),swapi
      REAL*8 swap

      if (m.gt.n/2.or.m.lt.1) write(*,*) 'probable misuse of hpsel'
      do 11 i=1,m
        heap(i)=arr(i)
        indexreal(i)=i
11    continue
 
      call sort2(m,heap,indexreal,wksp,iwksp)

      do i=1,m
        index(i)=indexreal(i)
      end do

      do 12 i=m+1,n
        if(arr(i).gt.heap(1))then
          heap(1)=arr(i)
          index(1)=i
          j=1
1         continue
            k=2*j
            if(k.gt.m)goto 2
            if(k.ne.m)then
              if(heap(k).gt.heap(k+1))k=k+1
            endif
            if(heap(j).le.heap(k))goto 2
            swap=heap(k)
            heap(k)=heap(j)
            heap(j)=swap
            swapi=index(k)
            index(k)=index(j)
            index(j)=swapi
            j=k
          goto 1
2         continue
        endif
12    continue
      return
      end
  
!----------------------------------------------------
!     this converts and integer to a character
!----------------------------------------------------
      subroutine int_charac(int,charac)
      integer int
      character*32 charac

      open(22,file='conversion')
      write(22,*)int
      close(22)

      open(22,file='conversion')
      read(22,*)charac
      close(22)

      return
        
      end
!----------------------------------------------------
!      this returns the factorial of a number
!----------------------------------------------------
      function fact(n)

      fact=1
      do 10 j=2,n
         fact=fact*j
10    continue
      return

      end

!---------------------------------------------------------------------
!     find number of digits of a number
!---------------------------------------------------------------------
      subroutine findn(number, aux)
      integer number, aux, numberaux

      aux=0
      numberaux=number
      do while (numberaux.ne.0)
        numberaux=numberaux/10
        aux=aux+1
      end do
      return
      end
!---------------------------------------------------------------------
!     find number of digits of a string
!---------------------------------------------------------------------
      subroutine finds(string,aux)
      character string*5, stringaux*5
      integer number, aux, numberaux,i

      stringaux=string
      aux=1
      do i=1,80
        if (string(1:aux).eq.stringaux) then
          goto 20
        else
          aux=aux+1
        end if
      end do
 20   return
      end

!----------------------------------------------------------------------
!     This writes tablefile with zphot, zspec, weights and magnitudes.
!----------------------------------------------------------------------
      subroutine etable(outfile,n,ntotalt,zphot2,zspec2,weight,mag1,mag2,mag3,mag4,mag5)

        character outfile*200
        integer n,i,ntotalt

        real*8 zphot2(n)
        real*8 zspec2(n)
        real*8 weight(n)     
        real*8 mag1(n),mag2(n),mag3(n),mag4(n),mag5(n)

        real*8 auxreal


        write(*,*)ntotalt

        do i=1,ntotalt

            auxreal=mag1(i)
            call scaleback(auxreal,auxreal)
            mag1(i)=auxreal 

            auxreal=mag2(i)
            call scaleback(auxreal,auxreal)
            mag2(i)=auxreal 

            auxreal=mag3(i)
            call scaleback(auxreal,auxreal)
            mag3(i)=auxreal 

            auxreal=mag4(i)
            call scaleback(auxreal,auxreal)
            mag4(i)=auxreal

            auxreal=mag5(i)
            call scaleback(auxreal,auxreal)
            mag5(i)=auxreal  

        end do


        open(3,file=outfile)
        do i=1,ntotalt
            write(3,66)zphot2(i),zspec2(i),weight(i),mag1(i),mag2(i),mag3(i),mag4(i),mag5(i)
        end do
        close(3)

! 66     FORMAT(2f15.8,e14.6,100f15.8)
 66     FORMAT(100e14.6)
      end


!----------------------------------------------------------------------
!     Scaleback all
!----------------------------------------------------------------------
      subroutine scalebackall(n,ntotalp,mag1,mag2,mag3,mag4,mag5)
      character name*250,name1*200,name2*200 
      character nameaux*200, nameaux1*200,nameaux2*200,nameaux3*200
      character nameaux4*200
      character photofile*200, trainfile*200,filters*100,cut*5
      integer n,i,nletphot,nlettrain,ntotalt,ntotalp,nfilters,order
      integer aux1,aux2,aux3,aux4,aux5,nneighbor,l
!      integer pos2(n) 
      integer sed2(n)

      real*8 mag1(n),mag2(n),mag3(n),mag4(n),mag5(n)
      real*8 auxreal


      do i=1,ntotalp

        auxreal=mag1(i)
        call scaleback(auxreal,auxreal)
        mag1(i)=auxreal 

        auxreal=mag2(i)
        call scaleback(auxreal,auxreal)
        mag2(i)=auxreal 

        auxreal=mag3(i)
        call scaleback(auxreal,auxreal)
        mag3(i)=auxreal 

        auxreal=mag4(i)
        call scaleback(auxreal,auxreal)
        mag4(i)=auxreal

        auxreal=mag5(i)
        call scaleback(auxreal,auxreal)
        mag5(i)=auxreal  

      end do

      end

!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "arr(n)"
!     From Numerical Recipes
!-----------------------------------------------
      SUBROUTINE sort(n,arr)
      INTEGER n,M,NSTACK
      REAL*8 arr(n)
      PARAMETER (M=7,NSTACK=50)
      INTEGER i,ir,j,jstack,k,l,istack(NSTACK)
      REAL*8 a,temp
      jstack=0
      l=1
      ir=n
1     if(ir-l.lt.M)then
        do 12 j=l+1,ir
          a=arr(j)
          do 11 i=j-1,l,-1
            if(arr(i).le.a)goto 2
            arr(i+1)=arr(i)
11        continue
          i=l-1
2         arr(i+1)=a
12      continue
        if(jstack.eq.0)return
        ir=istack(jstack)
        l=istack(jstack-1)
        jstack=jstack-2
      else
        k=(l+ir)/2
        temp=arr(k)
        arr(k)=arr(l+1)
        arr(l+1)=temp
        if(arr(l).gt.arr(ir))then
          temp=arr(l)
          arr(l)=arr(ir)
          arr(ir)=temp
        endif
        if(arr(l+1).gt.arr(ir))then
          temp=arr(l+1)
          arr(l+1)=arr(ir)
          arr(ir)=temp
        endif
        if(arr(l).gt.arr(l+1))then
          temp=arr(l)
          arr(l)=arr(l+1)
          arr(l+1)=temp
        endif
        i=l+1
        j=ir
        a=arr(l+1)
3       continue
          i=i+1
        if(arr(i).lt.a)goto 3
4       continue
          j=j-1
        if(arr(j).gt.a)goto 4
        if(j.lt.i)goto 5
        temp=arr(i)
        arr(i)=arr(j)
        arr(j)=temp
        goto 3
5       arr(l+1)=arr(j)
        arr(j)=a
        jstack=jstack+2
        if(jstack.gt.NSTACK) write(*,*) 'NSTACK too small in sort'
        if(ir-i+1.ge.j-l)then
          istack(jstack)=ir
          istack(jstack-1)=i
          ir=j-1
        else
          istack(jstack)=j-1
          istack(jstack-1)=l
          l=i
        endif
      endif
      goto 1
      END


!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort1(n,r1,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

      return
      END

!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2(n)"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort2(n,r1,r2,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------

      do 13 j=1,n
        wksp(j)=r2(j)
13    continue

      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue

      return
      END

!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort3(n,r1,r2,r3,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------

      do 13 j=1,n
        wksp(j)=r2(j)
13    continue

      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue

!------

      do 15 j=1,n
        wksp(j)=r3(j)
15    continue

      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue

      return
      END

!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3,..r6"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort6(n,r1,r2,r3,r4,r5,r6,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),r4(n),r5(n),r6(n)
      REAL*8 wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------
      do 13 j=1,n
        wksp(j)=r2(j)
13    continue
      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue
!------

!------
      do 15 j=1,n
        wksp(j)=r3(j)
15    continue
      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue
!------

!------
      do 17 j=1,n
        wksp(j)=r4(j)
17    continue
      do 18 j=1,n
        r4(j)=wksp(iwksp(j))
18    continue
!------

!------
      do 19 j=1,n
        wksp(j)=r5(j)
19    continue
      do 20 j=1,n
        r5(j)=wksp(iwksp(j))
20    continue
!------

!------
      do 21 j=1,n
        wksp(j)=r6(j)
21    continue
      do 22 j=1,n
        r6(j)=wksp(iwksp(j))
22    continue
!------

 
      return
      END


!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3,..r11"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort11(n,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),r4(n),r5(n),r6(n),r7(n),r8(n),r9(n)
      REAL*8 r10(n),r11(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------
      do 13 j=1,n
        wksp(j)=r2(j)
13    continue
      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue
!------

!------
      do 15 j=1,n
        wksp(j)=r3(j)
15    continue
      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue
!------

!------
      do 17 j=1,n
        wksp(j)=r4(j)
17    continue
      do 18 j=1,n
        r4(j)=wksp(iwksp(j))
18    continue
!------

!------
      do 19 j=1,n
        wksp(j)=r5(j)
19    continue
      do 20 j=1,n
        r5(j)=wksp(iwksp(j))
20    continue
!------

!------
      do 21 j=1,n
        wksp(j)=r6(j)
21    continue
      do 22 j=1,n
        r6(j)=wksp(iwksp(j))
22    continue
!------

!------
      do 23 j=1,n
        wksp(j)=r7(j)
23    continue
      do 24 j=1,n
        r7(j)=wksp(iwksp(j))
24    continue
!------

!------
      do 25 j=1,n
        wksp(j)=r8(j)
25    continue
      do 26 j=1,n
        r8(j)=wksp(iwksp(j))
26    continue
!------

!------
      do 27 j=1,n
        wksp(j)=r9(j)
27    continue
      do 28 j=1,n
        r9(j)=wksp(iwksp(j))
28    continue
!------

!------
      do 29 j=1,n
        wksp(j)=r10(j)
29    continue
      do 30 j=1,n
        r10(j)=wksp(iwksp(j))
30    continue
!------

!------
      do 31 j=1,n
        wksp(j)=r11(j)
31    continue
      do 32 j=1,n
        r11(j)=wksp(iwksp(j))
32    continue
!------

 
      return
      END


!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3,..r12"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort12(n,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),r4(n),r5(n),r6(n),r7(n),r8(n),r9(n)
      REAL*8 r10(n),r11(n),r12(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------
      do 13 j=1,n
        wksp(j)=r2(j)
13    continue
      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue
!------

!------
      do 15 j=1,n
        wksp(j)=r3(j)
15    continue
      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue
!------

!------
      do 17 j=1,n
        wksp(j)=r4(j)
17    continue
      do 18 j=1,n
        r4(j)=wksp(iwksp(j))
18    continue
!------

!------
      do 19 j=1,n
        wksp(j)=r5(j)
19    continue
      do 20 j=1,n
        r5(j)=wksp(iwksp(j))
20    continue
!------

!------
      do 21 j=1,n
        wksp(j)=r6(j)
21    continue
      do 22 j=1,n
        r6(j)=wksp(iwksp(j))
22    continue
!------

!------
      do 23 j=1,n
        wksp(j)=r7(j)
23    continue
      do 24 j=1,n
        r7(j)=wksp(iwksp(j))
24    continue
!------

!------
      do 25 j=1,n
        wksp(j)=r8(j)
25    continue
      do 26 j=1,n
        r8(j)=wksp(iwksp(j))
26    continue
!------

!------
      do 27 j=1,n
        wksp(j)=r9(j)
27    continue
      do 28 j=1,n
        r9(j)=wksp(iwksp(j))
28    continue
!------

!------
      do 29 j=1,n
        wksp(j)=r10(j)
29    continue
      do 30 j=1,n
        r10(j)=wksp(iwksp(j))
30    continue
!------

!------
      do 31 j=1,n
        wksp(j)=r11(j)
31    continue
      do 32 j=1,n
        r11(j)=wksp(iwksp(j))
32    continue
!------

!------
      do 33 j=1,n
        wksp(j)=r12(j)
33    continue
      do 34 j=1,n
        r12(j)=wksp(iwksp(j))
34    continue
!------
 
      return
      END


!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3,..r13"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort13(n,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),r4(n),r5(n),r6(n),r7(n),r8(n),r9(n)
      REAL*8 r10(n),r11(n),r12(n),r13(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------
      do 13 j=1,n
        wksp(j)=r2(j)
13    continue
      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue
!------

!------
      do 15 j=1,n
        wksp(j)=r3(j)
15    continue
      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue
!------

!------
      do 17 j=1,n
        wksp(j)=r4(j)
17    continue
      do 18 j=1,n
        r4(j)=wksp(iwksp(j))
18    continue
!------

!------
      do 19 j=1,n
        wksp(j)=r5(j)
19    continue
      do 20 j=1,n
        r5(j)=wksp(iwksp(j))
20    continue
!------

!------
      do 21 j=1,n
        wksp(j)=r6(j)
21    continue
      do 22 j=1,n
        r6(j)=wksp(iwksp(j))
22    continue
!------

!------
      do 23 j=1,n
        wksp(j)=r7(j)
23    continue
      do 24 j=1,n
        r7(j)=wksp(iwksp(j))
24    continue
!------

!------
      do 25 j=1,n
        wksp(j)=r8(j)
25    continue
      do 26 j=1,n
        r8(j)=wksp(iwksp(j))
26    continue
!------

!------
      do 27 j=1,n
        wksp(j)=r9(j)
27    continue
      do 28 j=1,n
        r9(j)=wksp(iwksp(j))
28    continue
!------

!------
      do 29 j=1,n
        wksp(j)=r10(j)
29    continue
      do 30 j=1,n
        r10(j)=wksp(iwksp(j))
30    continue
!------

!------
      do 31 j=1,n
        wksp(j)=r11(j)
31    continue
      do 32 j=1,n
        r11(j)=wksp(iwksp(j))
32    continue
!------

!------
      do 33 j=1,n
        wksp(j)=r12(j)
33    continue
      do 34 j=1,n
        r12(j)=wksp(iwksp(j))
34    continue
!------

!------
      do 35 j=1,n
        wksp(j)=r13(j)
35    continue
      do 36 j=1,n
        r13(j)=wksp(iwksp(j))
36    continue
!------
 
      return
      END


!-------------------------------------------------
!     Subroutine to sort "n" objects in vector "r1(n)"
!     Simultaneously update vector "r2,r3,..r14"
!     From Numerical Recipes
!-------------------------------------------------
      SUBROUTINE sort14(n,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,wksp,iwksp)
      INTEGER n,iwksp(n)
      REAL*8 r1(n),r2(n),r3(n),r4(n),r5(n),r6(n),r7(n),r8(n),r9(n)
      REAL*8 r10(n),r11(n),r12(n),r13(n),r14(n),wksp(n)

!U    USES indexx
      INTEGER j
      call indexx(n,r1,iwksp)
!------

      do 11 j=1,n
        wksp(j)=r1(j)
11    continue

      do 12 j=1,n
        r1(j)=wksp(iwksp(j))
12    continue

!------
      do 13 j=1,n
        wksp(j)=r2(j)
13    continue
      do 14 j=1,n
        r2(j)=wksp(iwksp(j))
14    continue
!------

!------
      do 15 j=1,n
        wksp(j)=r3(j)
15    continue
      do 16 j=1,n
        r3(j)=wksp(iwksp(j))
16    continue
!------

!------
      do 17 j=1,n
        wksp(j)=r4(j)
17    continue
      do 18 j=1,n
        r4(j)=wksp(iwksp(j))
18    continue
!------

!------
      do 19 j=1,n
        wksp(j)=r5(j)
19    continue
      do 20 j=1,n
        r5(j)=wksp(iwksp(j))
20    continue
!------

!------
      do 21 j=1,n
        wksp(j)=r6(j)
21    continue
      do 22 j=1,n
        r6(j)=wksp(iwksp(j))
22    continue
!------

!------
      do 23 j=1,n
        wksp(j)=r7(j)
23    continue
      do 24 j=1,n
        r7(j)=wksp(iwksp(j))
24    continue
!------

!------
      do 25 j=1,n
        wksp(j)=r8(j)
25    continue
      do 26 j=1,n
        r8(j)=wksp(iwksp(j))
26    continue
!------

!------
      do 27 j=1,n
        wksp(j)=r9(j)
27    continue
      do 28 j=1,n
        r9(j)=wksp(iwksp(j))
28    continue
!------

!------
      do 29 j=1,n
        wksp(j)=r10(j)
29    continue
      do 30 j=1,n
        r10(j)=wksp(iwksp(j))
30    continue
!------

!------
      do 31 j=1,n
        wksp(j)=r11(j)
31    continue
      do 32 j=1,n
        r11(j)=wksp(iwksp(j))
32    continue
!------

!------
      do 33 j=1,n
        wksp(j)=r12(j)
33    continue
      do 34 j=1,n
        r12(j)=wksp(iwksp(j))
34    continue
!------

!------
      do 35 j=1,n
        wksp(j)=r13(j)
35    continue
      do 36 j=1,n
        r13(j)=wksp(iwksp(j))
36    continue
!------

!------
      do 37 j=1,n
        wksp(j)=r14(j)
37    continue
      do 38 j=1,n
        r14(j)=wksp(iwksp(j))
38    continue
!------
 
      return
      END

!----------------------------------------
!     Used by sort subroutines above
!     From Numerical Recipes
!----------------------------------------
      SUBROUTINE indexx(n,arr,indx)
      INTEGER n,indx(n),M,NSTACK
      REAL*8 arr(n)
      PARAMETER (M=7,NSTACK=50)
      INTEGER i,indxt,ir,itemp,j,jstack,k,l,istack(NSTACK)
      REAL*8 a
      do 11 j=1,n
        indx(j)=j
11    continue
      jstack=0
      l=1
      ir=n
1     if(ir-l.lt.M)then
        do 13 j=l+1,ir
          indxt=indx(j)
          a=arr(indxt)
          do 12 i=j-1,l,-1
            if(arr(indx(i)).le.a)goto 2
            indx(i+1)=indx(i)
12        continue
          i=l-1
2         indx(i+1)=indxt
13      continue
        if(jstack.eq.0)return
        ir=istack(jstack)
        l=istack(jstack-1)
        jstack=jstack-2
      else
        k=(l+ir)/2
        itemp=indx(k)
        indx(k)=indx(l+1)
        indx(l+1)=itemp
        if(arr(indx(l)).gt.arr(indx(ir)))then
          itemp=indx(l)
          indx(l)=indx(ir)
          indx(ir)=itemp
        endif
        if(arr(indx(l+1)).gt.arr(indx(ir)))then
          itemp=indx(l+1)
          indx(l+1)=indx(ir)
          indx(ir)=itemp
        endif
        if(arr(indx(l)).gt.arr(indx(l+1)))then
          itemp=indx(l)
          indx(l)=indx(l+1)
          indx(l+1)=itemp
        endif
        i=l+1
        j=ir
        indxt=indx(l+1)
        a=arr(indxt)
3       continue
          i=i+1
        if(arr(indx(i)).lt.a)goto 3
4       continue
          j=j-1
        if(arr(indx(j)).gt.a)goto 4
        if(j.lt.i)goto 5
        itemp=indx(i)
        indx(i)=indx(j)
        indx(j)=itemp
        goto 3
5       indx(l+1)=indx(j)
        indx(j)=indxt
        jstack=jstack+2
        if(jstack.gt.NSTACK) write(*,*) 'NSTACK too small in indexx'
        if(ir-i+1.ge.j-l)then
          istack(jstack)=ir
          istack(jstack-1)=i
          ir=j-1
        else
          istack(jstack)=j-1
          istack(jstack-1)=l
          l=i
        endif
      endif
      goto 1
      END




