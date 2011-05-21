! vim:set ft=fortran:


program main
    use histogram
    use sortlib

    integer*8 i, j, k, n_in_bin
    integer*8, allocatable, dimension(:) :: sort_ind

    integer*8, allocatable, dimension(:) :: dat
    integer*8, allocatable, dimension(:) :: h
    integer*8, allocatable, dimension(:) :: rev

    integer*8 :: binsize = 1

    character*256  fname

    integer*8 num
    integer*8 :: lun=75

    if (iargc() /= 1) then
        print *,"Usage: test-hist filename"
        stop 45
    end if

    call getarg(1,fname)




    open(unit=lun,file=fname,status="OLD")
    read(lun,*)num
    print '(a,i0,a,a)',"Reading ",num," from file: ",fname

    allocate(dat(num))
    do i=1,size(dat)
        read(lun,*)dat(i)
    end do

    call qsorti8(dat, sort_ind)

    call histi8(dat, sort_ind, binsize, h, rev)

    do i=1,size(h)
        write (*,'("h(",i0,") = ",i0)'), i, h(i)
        n_in_bin = rev(i+1) - rev(i)

        if (n_in_bin /= h(i) ) then
            print '(a)',"n_in_bin and h do not match"
        end if


        if (n_in_bin > 0) then
            print '("Data in bin ",i0,":")'
            do j=1,n_in_bin
                k = rev( rev(i) + j -1 )
                print '("    index: ",i0,"    val: ",i0)',k,dat(k)
            end do
        endif

    end do

end program
