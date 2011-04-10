! vim:set ft=fortran:
module arrlib

    implicit none

    interface alloc
        module procedure alloci4, alloci8, allocf8
    end interface

contains

    subroutine alloci4(array, num, initval)
        integer*4, allocatable, dimension(:) :: array
        integer*8 num
        integer*4 initval

        if (allocated(array)) then
            if (size(array) /= num) then
                deallocate(array)
                allocate(array(num))
            end if
        else
            allocate(array(num))
        end if
        array=initval

    end subroutine alloci4

    subroutine alloci8(array, num, initval)
        integer*8, allocatable, dimension(:) :: array
        integer*8 num
        integer*8 initval

        if (allocated(array)) then
            if (size(array) /= num) then
                deallocate(array)
                allocate(array(num))
            end if
        else
            allocate(array(num))
        end if
        array=initval

    end subroutine alloci8



    subroutine allocf8(array, num, initval)
        real*8, allocatable, dimension(:) :: array
        integer*8 num
        real*8 initval

        if (allocated(array)) then
            if (size(array) /= num) then
                deallocate(array)
                allocate(array(num))
            end if
        else
            allocate(array(num))
        end if

        array=initval

    end subroutine allocf8


end module arrlib
