! vim:set ft=fortran:
module srclib

    implicit none

    ! we can't have the allocatable in here or else we can get a huge overhead
    ! in memory usage. Not sure why
    type source
        !sequence
        real*8 ra
        real*8 dec

        real*8 g1
        real*8 g2
        real*8 err

        integer*8 hpixid

#ifdef NINTERP
        real*8 scinv(NINTERP)
#else
        real*8 z
        real*8 dc
#endif

    end type source

    type source_cat
        integer*8 sigmacrit_style
        real*8, dimension(:), allocatable :: zlinterp
        real*8, dimension(:,:), allocatable :: scinv

        type(source), dimension(:), allocatable :: sources
    end type source_cat

contains

    subroutine read_source_cat(filename, scat)
        use fileutil

        ! read the binary version
        character(len=*):: filename
        type(source_cat) scat

        integer*8 nsource
        integer*8 i
        integer*8 nz

        integer :: lun

        lun = get_lun()

        print '("Reading source cat file (",i0,"): ",a)',lun,trim(filename)

        open(unit=lun,file=filename,access='STREAM')

        read (lun) scat%sigmacrit_style
        print '(a,i0)',"    Found sigmacrit_style: ",scat%sigmacrit_style

        if (scat%sigmacrit_style == 2) then
            read (lun) nz
            print '(a,i0)',"      nz: ",nz

#ifdef NINTERP
            if (nz /= NINTERP) then
                print '(a)','Mismatch in number of zl interpolation points'
                print '(a,i0,a,i0)','Expected ',NINTERP,' but found ',nz,' in the file'
                exit(45)
            endif
#endif
            allocate(scat%zlinterp(nz))
            print '(a)',"      reading zl"
            read (lun)scat%zlinterp

        endif

        read (lun) nsource
        print '(a,i0,a,$)',"    Found ",nsource," sources, reading..."

        if (scat%sigmacrit_style == 2) then
            allocate(scat%scinv(nsource,nz))
        endif

        allocate(scat%sources(nsource))

        !read(lun) scat%sources

        do i=1,nsource
            read (lun), scat%sources(i)%ra
            read (lun), scat%sources(i)%dec
            read (lun), scat%sources(i)%g1
            read (lun), scat%sources(i)%g2
            read (lun), scat%sources(i)%err
            read (lun), scat%sources(i)%hpixid

            if (scat%sigmacrit_style == 2) then
                read (lun), scat%scinv(i,:)
            else
                read (lun), scat%sources(i)%z
                read (lun), scat%sources(i)%dc
            endif
        enddo

        print '(a)',"Done"

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
