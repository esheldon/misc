! vim:set ft=fortran:
program main

    use shearlib

    type(sheardata) shdata
    character*256 config_file

    if (iargc() /= 1) then
        print *,"Usage: countpairs config_file"
        stop 45
    end if

    call getarg(1,config_file)

    call load_shear_data(config_file, shdata)
    !call count_pairs(shdata)

end program main
