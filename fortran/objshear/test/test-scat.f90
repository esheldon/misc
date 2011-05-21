! vim:set ft=fortran:
program main

    use srclib
    use cosmolib

    type(source_cat) :: scat

    character*256 filename

    integer*4, parameter :: nest=0
    integer*4, parameter :: nside=256

    if (iargc() /= 1) then
        print *,"Usage: test-scat filename"
        stop 45
    end if

    call cosmo_init(100.0_8, 0.3_8, 5_8)

    call getarg(1,filename)

    call read_source_cat(filename, scat)

    write (*,'(a)')"Before filling in hpix"
    call print_source_firstlast(scat)

    call add_source_hpixid(nside, scat)
    call add_source_dc(scat)

    write (*,'(a)')"After filling in hpix and dc"
    call print_source_firstlast(scat)

end program
