! vim:set ft=fortran:
module source_module

    ! should be factor of two so the struct will pack
    integer, parameter :: NZVALS = 10
    type source
        sequence
        real*8 ra
        real*8 dec

        real*4 e1
        real*4 e2
        real*4 err

        integer*4 htmid10

        real*4 scinv(NZVALS)

    end type source

    ! hold all the info in a source catalog, including
    ! the zvals

    type source_cat

        real*4 zlvals(NZVALS)
        type(source), allocatable, dimension(:) :: sources

    end type source_cat

contains

    subroutine realloc_sources(sources, nsource)
        type(source), allocatable, dimension(:) :: sources
        integer*4 nsource

        if (allocated(sources)) then
            write (*,'(a)')"sources is already allocated"
            if (size(sources) /= nsource) then
                write (*,'(a,i0)')"sources must be re-allocated to: ",nsource
                deallocate(sources)
                allocate(sources(nsource))
            end if
        else
            write (*,'(a,i0)')"allocating: ",nsource
            allocate(sources(nsource))
        end if

    end subroutine realloc_sources

    subroutine read_ascii_source_cat(scat, filename)
        ! read the ascii version
        type(source_cat) scat 
        character(len=*):: filename

        integer*4 nsource
        integer*4 i

        integer :: lun=87

        write (*,'("Reading source cat file: ",a)')filename

        open(unit=lun,file=filename,status='OLD')

        read (lun,*)nsource

        write (*,'("Found ",i0," sources")'),nsource
        call realloc_sources(scat%sources, nsource)
    
        do i=1,nsource
            read(lun,*)scat%sources(i)%ra, scat%sources(i)%dec, scat%sources(i)%scinv
        end do

        close(lun)

    end subroutine read_ascii_source_cat

    subroutine write_source_cat(scat, filename)
        type(source_cat) scat
        character(len=*):: filename

        integer*4 nsource, i

        integer :: lun=88

        nsource=size(scat%sources)

        open(unit=lun, file=filename, access='STREAM', status='REPLACE')

        write(lun)nsource
        write(lun)scat%sources

        close(lun)

    end subroutine write_source_cat


    subroutine read_source_cat(scat, filename)
        ! read the binary version
        type(source_cat) scat 
        character(len=*):: filename

        integer*4 nsource
        integer*4 i

        integer :: lun=89

        write (*,'("Reading source cat file: ",a)')filename

        open(unit=lun,file=filename,access='STREAM')

        read (lun)nsource
        write (*,'("Found ",i0," sources")'),nsource

        call realloc_sources(scat%sources, nsource)

        read(lun)scat%sources

        close(lun)

    end subroutine read_source_cat

 


    subroutine print_sources(sources, unit)
        type(source), dimension(:) :: sources
        integer, optional, intent(in) :: unit
        integer*4 i,j

        integer lun

        if (present(unit)) then
            lun = unit
        else
            ! stdout
            lun = 6
        end if

        write (lun,'(A," ",A," ",A)')"ind","ra","dec"
        do i=1, size(sources)
            ! might want E16.9 for general double, but for ra,dec
            ! we want F15.8 I think
            write (lun,'(I0," ",F0.8," ",F0.8, $)')i,sources(i)%ra,sources(i)%dec
            do j=1,NZVALS
                write (lun,'(" ",F0.6,$)') sources(i)%scinv(j)
            end do
            write (*,*)
        end do
    end subroutine print_sources

end module source_module
