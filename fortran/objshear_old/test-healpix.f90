! vim:set ft=fortran:

module radec_tools

    type radecs
        sequence
        real*8 ra
        real*8 dec
    end type

    type radecpix
        integer*4, allocatable, dimension(:) :: pix
        type(radecs), allocatable, dimension(:) :: radec
    end type

contains
    subroutine read_radec(radec, filename)
        type(radecs), allocatable, dimension(:) :: radec 
        character(len=*):: filename

        integer :: lun=77
        integer*4 n

        print *,"Reading ra/dec from file: ",filename
        open(unit=77, file=filename,access='STREAM')

        read (lun) n

        print *,"Reading ",n," from file"
        allocate(radec(n))

        read(lun) radec

        close(lun)
    end subroutine read_radec

    subroutine print_radec(radec, n)
        type(radecs), allocatable, dimension(:) :: radec 
        integer*4, intent(in), optional :: n

        integer*4 nprint, i

        if (present(n)) then
            nprint=n
        else
            nprint=size(radec)
        end if

        do i=1,nprint
            print *,radec(i)%ra," ",radec(i)%dec
        end do
       
    end subroutine print_radec

    subroutine print_radecpix(rdp, n)
        type(radecpix) :: rdp
        integer*4, intent(in), optional :: n

        integer*4 nprint, i

        if (present(n)) then
            nprint=n
        else
            nprint=size(rdp%radec)
        end if

        do i=1,nprint
            print *,rdp%radec(i)%ra," ",rdp%radec(i)%dec," ",rdp%pix(i)
        end do
       
    end subroutine print_radecpix



end module radec_tools

module histpixmod
contains

    subroutine histpix(pix)
        use sortlib
        use histogram

        integer*4, intent(in), dimension(:) :: pix

        type hist_structi4
            integer*4 amin
            integer*4 amax
            integer*4, dimension(:), allocatable :: h
            integer*4, dimension(:), allocatable :: rev
        end type

        type(hist_structi4) histst

        integer*4, dimension(:), allocatable :: sort_ind

        !integer*4, dimension(:), allocatable :: h, rev
        integer*4 binsize
        integer*4 i

        print *,"Getting sort index"
        call qsorti4(pix, sort_ind)

        histst%amin = pix(sort_ind(1))
        histst%amax = pix(sort_ind(size(pix)))

        binsize=1
        print *,"Getting reverse indices"
        call histi4(pix, sort_ind, binsize, histst%h, rev=histst%rev)

        return
        do i=1,size(histst%h)
            if (histst%h(i) > 0) then
                write (*,'("h(",i0,") = ",i0)'), i, histst%h(i)
            endif
        end do
        return

    end subroutine histpix


end module histpixmod


program main
    use radec_tools
    use healpix
    use histpixmod

    integer*4 i, j
    integer*4, parameter :: nest=1
    integer*4, parameter :: nside=256
    integer*4, parameter :: maxpix=1000
    ! all pixels *overlapping* the disc are output
    integer*4, parameter :: inclusive=1

    real*8 rad
    integer*4 npixfound
    integer*4, allocatable, dimension(:) :: listpix
    integer*4 nlist

    ! note odd convention
    ! 0 < phi < 2*pi
    ! 0 < theta < pi
    real*8 phi, theta

    type(radecpix) :: rdp

    allocate(listpix(maxpix))

    print *,"Working with healpix pixels:"
    print *,"  nside:              ",nside
    print *,"  npix:               ",npix(nside)
    print *,"  area (srad):        ",area(nside)
    print *,"  area (deg2):        ",area(nside)*(180.0/PI)**2
    print *,"  area (arcmin2):     ",area(nside)*(180.0/PI*60.0)**2
    print *,"  linsize (deg):      ",sqrt(area(nside)*(180.0/PI)**2)
    print *,"  linsize (arcmin):   ",sqrt(area(nside)*(180.0/PI*60.0)**2)

    call read_radec(rdp%radec, "rand-radec.bin")
    allocate(rdp%pix(size(rdp%radec)))

    print *,"Calculating pixel number"
    do i=1,size(rdp%pix)

        if (nest==1) then
            call eq2pix_nest(nside, rdp%radec(i)%ra, rdp%radec(i)%dec, rdp%pix(i))
        else
            call eq2pix_ring(nside, rdp%radec(i)%ra, rdp%radec(i)%dec, rdp%pix(i))
        endif
    end do

    call print_radecpix(rdp,10)

    print *,"sending pix to histpix: ",size(rdp%pix)
    call histpix(rdp%pix)

    return

    !rad=100.0/3600.0*DEG2RAD
    rad=1.0*DEG2RAD
    print *,"Finding pixels within ",rad
    do i=1,size(rdp%pix)
        !print *,"Finding pixels within ",rad," radians of position ",&
        !rdp%radec(i)%ra, rdp%radec(i)%dec
        call query_disc(nside, rdp%radec(i)%ra, rdp%radec(i)%dec, rad, listpix, &
                        nlist, inclusive)
        !print *,"Found ",nlist," pixels"
        !do j=1,nlist
        !    print *,listpix(j)
        !end do
    end do
end program
