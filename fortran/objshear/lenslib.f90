! vim:set ft=fortran:
module lenslib

    implicit none

    ! should be factor of two so the struct will pack
    !integer, parameter :: NZVALS = 10
    type lens
        sequence
        real*8 ra
        real*8 dec

        real*8 z
        real*8 dc

        integer*4 zindex

        integer*4 padding;

    end type lens


contains

    subroutine read_lens_cat(filename, lenses)
        use fileutil

        type(lens), dimension(:), allocatable :: lenses 
        character(len=*):: filename

        integer*4 nlens
        integer*4 i

        integer :: lun

        lun = get_lun()

        write (*,'("Reading lens cat (",i0,"): ",a)')lun,trim(filename)

        open(unit=lun,file=filename,access='STREAM')

        read (lun)nlens
        write (*,'("    Found ",i0," lenses, reading...",$)'),nlens

        allocate(lenses(nlens))

        read(lun)lenses

        print *,"Done"

        close(lun)

    end subroutine read_lens_cat

    subroutine add_lens_dc(lenses)
        ! add comoving distance
        use cosmolib
        type(lens), dimension(:) :: lenses
        integer*4 i

        print '(a,i0)',"Adding dc to lenses"
        do i=1,size(lenses)
            lenses(i)%dc = cdist(0.0_8, lenses(i)%z)
        end do
    end subroutine add_lens_dc



    subroutine print_lens_firstlast(lenses)
        type(lens), dimension(:) :: lenses

        print '(a15,$)',"ra"
        print '(a15,$)',"dec"
        print '(a15,$)',"z"
        print '(a15,$)',"dc"
        print '(a10,$)',"zindex"
        print '(a10)',"padding"

        call print_lens_row(lenses, 1)
        call print_lens_row(lenses, size(lenses))
    end subroutine print_lens_firstlast

    subroutine print_lens_row(lenses, row)
        type(lens), dimension(:) :: lenses
        integer*4 row

        write (*,'(4F15.8,i10,i10)') &
            lenses(row)%ra, lenses(row)%dec, &
            lenses(row)%z, lenses(row)%dc, &
            lenses(row)%zindex, lenses(row)%padding
    end subroutine print_lens_row


end module lenslib
