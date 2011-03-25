! vim:set ft=fortran:
program main

    use shearlib

    implicit none

    type(sheardata) shdata
    type(lens_sum), dimension(:), allocatable :: lensums

    character*256 config_file

    if (iargc() /= 1) then
        print *,"Usage: objshear config_file"
        stop 45
    end if

    call getarg(1,config_file)

    call load_shear_data(config_file, shdata)

    call calc_shear(shdata, lensums)

    call write_lens_sums(shdata % pars % output_file, lensums)

    print'(a)','Done'

end program main
