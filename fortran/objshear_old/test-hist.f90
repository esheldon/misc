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

program main
    use radec_tools
    use histogram
    use sortlib

    integer*4 i, j, k, n_in_bin
    integer*4, allocatable, dimension(:) :: sort_ind
    type(radecs), allocatable, dimension(:) :: radec

    integer*4, allocatable, dimension(:) :: h
    integer*4, allocatable, dimension(:) :: rev
    real*8 binsize

    call read_radec(radec, "rand-radec.bin")

    call qsortf8(radec%dec, sort_ind)

    do i=1,size(radec)
        print *,i,radec(i)%dec,sort_ind(i),radec(sort_ind(i))%dec
    end do

    binsize=10.0
    print *,"Calling histf8"
    call histf8(radec%dec, sort_ind, binsize, h, rev, &
                binmin=-100.0_8, binmax=100.0_8)

    do i=1,size(h)
        write (*,'("h(",i0,") = ",i0)'), i, h(i)
    end do

    do i=1,size(rev)
        write (*,'("rev(",i0,") = ",i0)'),i,rev(i)
    end do

    do i=1,size(h)
        write (*,'("h(",i0,") = ",i0)'), i, h(i)
        print *,rev(i),rev(i+1)

        if (rev(i) /= rev(i+1)) then
            n_in_bin = rev(i+1) - rev(i)
            do j=1,n_in_bin
                k = rev( rev(i) + j -1 )
                write (*,'("    ",i03," ",i03," ",f0.8)'), j, k, radec(k)%dec
            end do
        endif

    end do

end program
