! vim:set ft=fortran:
module histogram

    implicit none

contains

    subroutine histf8(array, sort_index, binsize, h, rev, binmin, binmax)
        !   usage:
        !       histf8(array, sort_index, binsize, h, rev [, binmin, binmax])
        !   inputs:
        !       array: A 1-d array
        !       sort_index: A sort index into the array
        !       binsize: The bin size for the histogram
        !   optional inputs:
        !       binmin, binmax: The min,max values to consider. Default is
        !           the min and max of the input array
        !   outputs:
        !       h: The histogram
        !   optional outputs:
        !       rev: The reverse indices.
        use arrlib
        use errlib

        real*8,    intent(in), dimension(:) :: array
        integer*4, intent(in), dimension(:) :: sort_index
        real*8,    intent(in)               :: binsize

        integer*4, intent(inout), dimension(:), allocatable            :: h
        integer*4, intent(inout), dimension(:), allocatable, optional  :: rev

        real*8, optional :: binmin
        real*8, optional :: binmax
        real*8 mbinmin, mbinmax

        integer*4 nbin
        logical dorev

        integer*4 binnum, binnum_old, array_index, tbin, offset
        integer*4 i

        real*8 bininv

        if (size(array) /= size(sort_index)) then
            call fatal_error("array and sort index must be same size")
        endif

        dorev = .false. 
        if (present(rev)) then
            dorev=.true.
        endif

        if (present(binmin)) then
            mbinmin = binmin
        else
            mbinmin = array(sort_index(1))
        endif
        if (present(binmax)) then
            mbinmax = binmax
        else
            mbinmax = array(sort_index(size(array)))
        endif

        bininv = 1.0/binsize
        nbin = int( (mbinmax-mbinmin)*bininv ) + 1

        ! allocate the outputs
        call alloc(h, nbin, 0)
        h=0
        if (dorev) then
            call alloc(rev, size(array) + nbin + 1, 0);
            rev=(size(rev)+1)
        endif

        binnum_old = 0
        do i=1,size(array)
            array_index = sort_index(i)

            ! offset into rev
            offset = i + nbin + 1
            if (dorev) then
                rev(offset) = array_index
            endif

            binnum = int( (array(array_index)-mbinmin)*bininv ) + 1

            if ( (binnum >= 1) .and. (binnum <= nbin) ) then

                ! should we update the reverse indices?
                if (dorev .and. (binnum > binnum_old)) then
                    tbin = binnum_old + 1
                    do while(tbin <= binnum)
                        rev(tbin) = offset
                        tbin = tbin + 1
                    end do
                endif

                ! update the histogram
                h(binnum) = h(binnum) + 1
                binnum_old = binnum

            endif
        end do

    end subroutine histf8


    subroutine histi4(array, sort_index, binsize, h, rev, binmin, binmax)
        !   usage:
        !       histi4(array, sort_index, binsize, h, rev [, binmin, binmax])
        !   inputs:
        !       array: A 1-d array
        !       sort_index: A sort index into the array
        !       binsize: The bin size for the histogram.
        !   optional inputs:
        !       binmin, binmax: The min,max values to consider. Default is
        !           the min and max of the input array
        !   outputs:
        !       h: The histogram
        !   optional outputs:
        !       rev: The reverse indices.
        use arrlib
        use errlib

        integer*4, intent(in), dimension(:) :: array
        integer*4, intent(in), dimension(:) :: sort_index
        integer*4, intent(in)               :: binsize

        integer*4, intent(inout), dimension(:), allocatable            :: h
        integer*4, intent(inout), dimension(:), allocatable, optional  :: rev

        integer*4, optional :: binmin
        integer*4, optional :: binmax
        integer*4 mbinmin, mbinmax

        integer*4 nbin
        logical dorev

        integer*4 binnum, binnum_old, array_index, tbin, offset
        integer*4 i

        real*8 bininv

        if (size(array) /= size(sort_index)) then
            call fatal_error("array and sort index must be same size")
        endif

        dorev = .false. 
        if (present(rev)) then
            dorev=.true.
        endif

        if (present(binmin)) then
            mbinmin = binmin
        else
            mbinmin = array(sort_index(1))
        endif
        if (present(binmax)) then
            mbinmax = binmax
        else
            mbinmax = array(sort_index(size(array)))
        endif

        bininv = 1.0/dfloat(binsize)
        nbin = int( dfloat(mbinmax-mbinmin)*bininv ) + 1

        ! allocate the outputs
        call alloc(h, nbin, 0)
        h=0
        if (dorev) then
            call alloc(rev, size(array) + nbin + 1, 0);
            rev=dfloat(size(rev)+1)
        endif

        binnum_old = 0
        do i=1,size(array)
            array_index = sort_index(i)

            ! offset into rev
            offset = i + nbin + 1
            if (dorev) then
                rev(offset) = array_index
            endif

            binnum = int( dfloat(array(array_index)-mbinmin)*bininv ) + 1

            if ( (binnum >= 1) .and. (binnum <= nbin) ) then

                ! should we update the reverse indices?
                if (dorev .and. (binnum > binnum_old)) then
                    tbin = binnum_old + 1
                    do while(tbin <= binnum)
                        rev(tbin) = offset
                        tbin = tbin + 1
                    end do
                endif

                ! update the histogram
                h(binnum) = h(binnum) + 1
                binnum_old = binnum

            endif
        end do

    end subroutine histi4




end module histogram
