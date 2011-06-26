! vim:set ft=fortran:
module srclib

    implicit none

    type source_cat

        ! number of sources in the catalog
        integer*8 nel

        ! the style of sigmacrit: 1 for true z, 2 for interpolate scinv
        integer*8 sigmacrit_style


        real*8, dimension(:), allocatable :: ra
        real*8, dimension(:), allocatable :: dec
        real*8, dimension(:), allocatable :: g1
        real*8, dimension(:), allocatable :: g2
        real*8, dimension(:), allocatable :: err

        integer*8, dimension(:), allocatable :: hpixid

        ! only used for true z
        real*8, dimension(:), allocatable :: z
        real*8, dimension(:), allocatable :: dc

        ! only used if interpolating scinv
        integer*8 nzl
        real*8 zlmin
        real*8 zlmax
        real*8, dimension(:), allocatable :: zlinterp
        real*8, dimension(:,:), allocatable :: scinv


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
            scat%nzl = nz
            print '(a,i0)',"      nz: ",nz

            allocate(scat%zlinterp(nz))
            print '(a,$)',"      reading zl: "
            read (lun)scat%zlinterp
            do i=1,nz 
                print '(F6.3," ",$)',scat%zlinterp(i)
            end do
            print *

            scat%zlmin = scat%zlinterp(1)
            scat%zlmax = scat%zlinterp(nz)

        else
            nz=0
            scat%zlmin = 0_8
            scat%zlmax = 9999_8
        endif

        read (lun) nsource
        print '(a,i0,a,$)',"    Found ",nsource," sources, reading..."

        scat%nel = nsource

        allocate(scat%ra(nsource))
        allocate(scat%dec(nsource))
        allocate(scat%g1(nsource))
        allocate(scat%g2(nsource))
        allocate(scat%err(nsource))
        allocate(scat%hpixid(nsource))

        if (scat%sigmacrit_style == 2) then
            allocate(scat%scinv(nsource,nz))
        else
            allocate(scat%z(nsource))
            allocate(scat%dc(nsource))
        endif

        do i=1,nsource
            read (lun), scat%ra(i)
            read (lun), scat%dec(i)
            read (lun), scat%g1(i)
            read (lun), scat%g2(i)
            read (lun), scat%err(i)
            read (lun), scat%hpixid(i)

            if (scat%sigmacrit_style == 2) then
                read (lun), scat%scinv(i,:)
            else
                read (lun), scat%z(i)
                read (lun), scat%dc(i)
            endif
        enddo

        print '(a)',"Done"

        close(lun)

    end subroutine read_source_cat

    subroutine add_source_dc(scat)
        use cosmolib
        type(source_cat), intent(inout) :: scat
        integer*8 i

        print '(a,i0)',"Adding dc to sources"
        do i=1,scat%nel
            scat%dc(i) = cdist(0.0_8, scat%z(i))
        end do
    end subroutine add_source_dc


    subroutine add_source_hpixid(nside, scat)

        use healpix, only : RAD2DEG, npix, pixarea, eq2pix
        integer*8, intent(in) :: nside
        type(source_cat), intent(inout) :: scat

        integer*8 i
        integer*8 id

        print '(a,i0)',"Adding source healpix id, nside=",nside
        print '(a,i0)',"    number of pixels: ",npix(nside)
        print '(a,f15.8)',"    pixel area:   ",pixarea(nside)*RAD2DEG**2
        print '(a,f15.8)',"    linear scale arcmin: ",sqrt(pixarea(nside))*RAD2DEG*60
        print '(a,f15.8)',"    linear scale arcsec: ",sqrt(pixarea(nside))*RAD2DEG*3600

        
        do i=1,scat%nel
            call eq2pix(nside, scat%ra(i), scat%dec(i), id)

            scat%hpixid(i) = id

        end do

    end subroutine add_source_hpixid




    subroutine print_source_firstlast(scat)
        type(source_cat), intent(in) :: scat

        print '(a15,$)',"ra"
        print '(a15,$)',"dec"
        print '(a15,$)',"g1"
        print '(a15,$)',"g2"
        print '(a15,$)',"err"
        print '(a10,$)',"hpixid"
        if (scat%sigmacrit_style == 1) then
            print '(a15,$)',"z"
            print '(a15)',"dc"
        else
            print '(a25,$)',"scinv(1)"
            print '(a25)',"scinv(-1)"
        endif

        call print_source_row(scat, 1)
        call print_source_row(scat, scat%nel)
    end subroutine print_source_firstlast

    subroutine print_source_row(scat, row)
        type(source_cat), intent(in) :: scat
        integer*8 row
        real*8 val1, val2

        if (scat%sigmacrit_style == 1) then
            val1 = scat%z(row)
            val2 = scat%dc(row)
        else
            val1 = scat%scinv(row,1)
            val2 = scat%scinv(row,size(scat%zlinterp))
        endif

        print '(5F15.8,i10,2E)',    &
            scat%ra(row),       &
            scat%dec(row),      &
            scat%g1(row),       &
            scat%g2(row),       &
            scat%err(row),      &
            scat%hpixid(row),   &
            val1, val2

    end subroutine print_source_row


end module srclib
