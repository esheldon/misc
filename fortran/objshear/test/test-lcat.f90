! vim:set ft=fortran:
program main

    use lenslib
    use cosmolib

    character*256 filename
    type(lens), dimension(:), allocatable :: lenses


    if (iargc() /= 1) then
        print *,"Usage: test-lcat filename"
        stop 45
    end if


    call getarg(1,filename)

    call cosmo_init(100.0, 0.3, 5)

    call read_lens_cat(filename, lenses)

    call add_lens_dc(lenses)

    call print_lens_firstlast(lenses)


end program
