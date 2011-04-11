! vim:set ft=fortran:
module sortlib

    implicit none

contains


! pseudocode
! function quicksort(array, left, right)
!     var pivot, leftIdx = left, rightIdx = right
!     if right - left + 1 greater than 1
!         pivot = (left + right) / 2
!         while leftIdx less than or equal to pivot and rightIdx greater than or equal to pivot
!             while array[leftIdx] less than array[pivot] and leftIdx less than or equal to pivot
!                 leftIdx = leftIdx + 1
!             while array[rightIdx] greater than array[pivot] and rightIdx greater than or equal to pivot
!                 rightIdx = rightIdx - 1;
!             swap array[leftIdx] with array[rightIdx]
!             leftIdx = leftIdx + 1
!             rightIdx = rightIdx - 1
!             if leftIdx - 1 equal to pivot
!                 pivot = rightIdx = rightIdx + 1
!             else if rightIdx + 1 equal to pivot
!                 pivot = leftIdx = leftIdx - 1
!         quicksort(array, left ,pivot - 1)
!         quicksort(array, pivot + 1, right)

    subroutine qsortf8(arr, ind)

        real*8, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:), allocatable :: ind

        integer*8 left, right, i

        allocate(ind(size(arr))); ind=0
        do i=1,size(arr)
            ind(i) = i
        end do

        left = 1
        right = size(arr)

        call qsortf8_recurse(arr, ind, left, right)

    end subroutine qsortf8


    subroutine qsorti4(arr, ind)

        integer*4, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:), allocatable :: ind

        integer*8 left, right, i

        allocate(ind(size(arr))); ind=0
        do i=1,size(arr)
            ind(i) = i
        end do

        left = 1
        right = size(arr)

        call qsorti4_recurse(arr, ind, left, right)

    end subroutine qsorti4

    subroutine qsorti8(arr, ind)

        integer*8, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:), allocatable :: ind

        integer*8 left, right, i

        allocate(ind(size(arr))); ind=0
        do i=1,size(arr)
            ind(i) = i
        end do

        left = 1
        right = size(arr)

        call qsorti8_recurse(arr, ind, left, right)

    end subroutine qsorti8



    recursive subroutine qsortf8_recurse(arr, ind, left, right)
        real*8, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:) :: ind
        integer*8, intent(in) :: left, right

        integer*8 pivot, leftidx, rightidx

        leftidx=left
        rightidx=right
        
        if ( (right-left+1) > 1 ) then
            pivot = (left+right)/2

            do while (leftidx <= pivot .and. rightidx >= pivot)

                do while (arr(ind(leftidx)) < arr(ind(pivot)) & 
                          .and. leftidx <= pivot)
                    leftidx = leftidx + 1
                end do
                do while (arr(ind(rightidx)) > arr(ind(pivot)) &
                          .and. rightidx >= pivot)
                    rightidx = rightidx - 1
                end do

                call swapi8( ind(leftidx), ind(rightidx) )

                leftidx = leftidx+1
                rightidx = rightidx-1
                if (leftidx-1 == pivot) then
                    rightidx = rightidx + 1
                    pivot = rightidx
                elseif (rightidx+1 == pivot) then
                    leftidx = leftidx-1
                    pivot = leftidx
                endif
            end do

            call qsortf8_recurse(arr, ind, left, pivot-1)
            call qsortf8_recurse(arr, ind, pivot+1, right)
            
        endif
    end subroutine qsortf8_recurse 



    recursive subroutine qsorti4_recurse(arr, ind, left, right)
        integer*4, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:) :: ind
        integer*8, intent(in) :: left, right

        integer*8 pivot, leftidx, rightidx

        leftidx=left
        rightidx=right
        
        if ( (right-left+1) > 1 ) then
            pivot = (left+right)/2

            do while (leftidx <= pivot .and. rightidx >= pivot)

                do while (arr(ind(leftidx)) < arr(ind(pivot)) & 
                          .and. leftidx <= pivot)
                    leftidx = leftidx + 1
                end do
                do while (arr(ind(rightidx)) > arr(ind(pivot)) &
                          .and. rightidx >= pivot)
                    rightidx = rightidx - 1
                end do

                call swapi8( ind(leftidx), ind(rightidx) )

                leftidx = leftidx+1
                rightidx = rightidx-1
                if (leftidx-1 == pivot) then
                    rightidx = rightidx + 1
                    pivot = rightidx
                elseif (rightidx+1 == pivot) then
                    leftidx = leftidx-1
                    pivot = leftidx
                endif
            end do

            call qsorti4_recurse(arr, ind, left, pivot-1)
            call qsorti4_recurse(arr, ind, pivot+1, right)
            
        endif
    end subroutine qsorti4_recurse 

    recursive subroutine qsorti8_recurse(arr, ind, left, right)
        integer*8, intent(in), dimension(:) :: arr
        integer*8, intent(inout), dimension(:) :: ind
        integer*8, intent(in) :: left, right

        integer*8 pivot, leftidx, rightidx

        leftidx=left
        rightidx=right
        
        if ( (right-left+1) > 1 ) then
            pivot = (left+right)/2

            do while (leftidx <= pivot .and. rightidx >= pivot)

                do while (arr(ind(leftidx)) < arr(ind(pivot)) & 
                          .and. leftidx <= pivot)
                    leftidx = leftidx + 1
                end do
                do while (arr(ind(rightidx)) > arr(ind(pivot)) &
                          .and. rightidx >= pivot)
                    rightidx = rightidx - 1
                end do

                call swapi8( ind(leftidx), ind(rightidx) )

                leftidx = leftidx+1
                rightidx = rightidx-1
                if (leftidx-1 == pivot) then
                    rightidx = rightidx + 1
                    pivot = rightidx
                elseif (rightidx+1 == pivot) then
                    leftidx = leftidx-1
                    pivot = leftidx
                endif
            end do

            call qsorti8_recurse(arr, ind, left, pivot-1)
            call qsorti8_recurse(arr, ind, pivot+1, right)
            
        endif
    end subroutine qsorti8_recurse 




    subroutine swapi4(a, b)
        integer*4 a
        integer*4 b
        integer*4 tmp

        tmp = a
        a = b
        b = tmp
    end subroutine swapi4

    subroutine swapi8(a, b)
        integer*8 a
        integer*8 b
        integer*8 tmp

        tmp = a
        a = b
        b = tmp
    end subroutine swapi8



end module sortlib
