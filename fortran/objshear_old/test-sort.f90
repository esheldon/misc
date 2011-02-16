! vim:set ft=fortran:

program main
    use sortlib

    integer*4, dimension(:), allocatable :: iarr
    real*8, dimension(:), allocatable :: farr
    integer*4, dimension(:), allocatable :: ind
    integer*4 i

    allocate(iarr(9))
    iarr = (/8, 3, 15, 17, 5, 1, 22, 68, 11/)
    call qsorti4(iarr, ind)

    do i=1,size(iarr)
        print *,i,iarr(i),ind(i),iarr(ind(i))
    end do


    allocate(farr(9))
    farr = (/8., 3., 15., 17., 5., 1., 22., 68., 11./)
    call qsortf8(farr, ind)

    do i=1,size(farr)
        print *,i,farr(i),ind(i),farr(ind(i))
    end do


end program
