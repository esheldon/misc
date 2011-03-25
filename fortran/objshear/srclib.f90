! vim:set ft=fortran:
module srclib

    implicit none

    type source
        sequence
        real*8 ra
        real*8 dec

        real*8 g1
        real*8 g2
        real*8 err

        !real*8 scinv(NZVALS)
        real*8 z
        real*8 dc

        integer*8 hpixid

    end type source

contains

    subroutine read_source_cat(filename, sources)
        use fileutil

        ! read the binary version
        type(source), dimension(:), allocatable :: sources
        character(len=*):: filename

        integer*8 nsource
        integer*8 i

        integer :: lun

        lun = get_lun()

        print '("Reading source cat file (",i0,"): ",a)',lun,trim(filename)

        open(unit=lun,file=filename,access='STREAM')

        read (lun)nsource
        write (*,'("    Found ",i0," sources, reading...",$)'),nsource

        allocate(sources(nsource))

        read(lun)sources

        print *,"Done"

        close(lun)

    end subroutine read_source_cat



    subroutine add_source_dc(sources)
        ! add comoving distance
        use cosmolib
        type(source), dimension(:) :: sources
        integer*8 i

        print '(a,i0)',"Adding dc to sources"
        do i=1,size(sources)
            sources(i)%dc = cdist(0.0_8, sources(i)%z)
        end do
    end subroutine add_source_dc


    subroutine add_source_hpixid(nside, sources)

        use healpix, only : RAD2DEG, npix, pixarea, eq2pix
        integer*8, intent(in) :: nside
        type(source), dimension(:) :: sources

        integer*8 i
        integer*8 id

        print '(a,i0)',"Adding source healpix id, nside=",nside
        print '(a,i0)',"    number of pixels: ",npix(nside)
        print '(a,f15.8)',"    pixel area:   ",pixarea(nside)*RAD2DEG**2
        print '(a,f15.8)',"    linear scale arcmin: ",sqrt(pixarea(nside))*RAD2DEG*60
        print '(a,f15.8)',"    linear scale arcsec: ",sqrt(pixarea(nside))*RAD2DEG*3600

        
        do i=1,size(sources)
            call eq2pix(nside, sources(i)%ra, sources(i)%dec, id)

            sources(i)%hpixid = id

        end do

    end subroutine add_source_hpixid




    subroutine print_source_firstlast(sources)
        type(source), dimension(:) :: sources

        print '(a15,$)',"ra"
        print '(a15,$)',"dec"
        print '(a15,$)',"g1"
        print '(a15,$)',"g2"
        print '(a15,$)',"err"
        print '(a10,$)',"hpixid"
        print '(a15,$)',"z"
        print '(a15)',"dc"

        call print_source_row(sources, 1)
        call print_source_row(sources, size(sources,kind=8))
    end subroutine print_source_firstlast

    subroutine print_source_row(sources, row)
        type(source), dimension(:) :: sources
        integer*8 row

        write (*,'(5F15.8,i10,2F15.8)') &
            sources(row)%ra, sources(row)%dec, &
            sources(row)%g1, sources(row)%g2, sources(row)%err, &
            sources(row)%hpixid, &
            sources(row)%z, &
            sources(row)%dc
        !do j=1,NZVALS
        !    write (lun,'(" ",F0.6,$)') sources(i)%scinv(j)
        !end do
    end subroutine print_source_row


end module srclib
