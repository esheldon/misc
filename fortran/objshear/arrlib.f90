! vim:set ft=fortran:
module arrlib

    implicit none

contains

    subroutine reallocatei4(array, num)
        integer*4, allocatable, dimension(:) :: array
        integer*4 num

        if (allocated(array)) then
            if (size(array) /= num) then
                deallocate(array)
                allocate(array(num))
            end if
        else
            allocate(array(num))
        end if

    end subroutine reallocatei4

    subroutine reallocatef8(array, num)
        real*8, allocatable, dimension(:) :: array
        integer*4 num

        if (allocated(array)) then
            if (size(array) /= num) then
                deallocate(array)
                allocate(array(num))
            end if
        else
            allocate(array(num))
        end if

    end subroutine reallocatef8


end module arrlib
