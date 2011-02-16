! vim:set ft=fortran:
program main

    use srclib
    use cosmolib

    type(source),dimension(:), allocatable :: sources

    character*256 filename

    integer*4, parameter :: nest=0
    integer*4, parameter :: nside=256

    if (iargc() /= 1) then
        print *,"Usage: test-scat filename"
        stop 45
    end if

    call cosmo_init(100.0, 0.3, 5)

    call getarg(1,filename)

    call read_source_cat(filename, sources)

    write (*,'(a)')"Before filling in hpix"
    call print_source_firstlast(sources)

    call add_source_hpixid(nside, nest, sources)
    call add_source_dc(sources)

    write (*,'(a)')"After filling in hpix and dc"
    call print_source_firstlast(sources)

end program
