! vim:set ft=fortran:

! this doesn't work
subroutine testopt(n, arr)
    use arrlib
    integer*4, intent(in) :: n
    real*8, intent(inout), dimension(:), optional, allocatable :: arr

    integer*4 i

    if (present(arr)) then
        print *,"re-allocating"
        call reallocatef8(arr, n)
    else
        print *,"allocating",n
        allocate(arr(n))
    endif

    print *,"filling"
    do i=1,n
        arr(i) = i
    end do
    arr = arr + 0.25

end subroutine testopt

program main

    real*8, dimension(:), allocatable :: arr
    integer*4 n,i

    n=20

    call testopt(n, arr)

    print *,"printing"
    do i=1,n
        print *,i,arr(i)
    end do

end program


