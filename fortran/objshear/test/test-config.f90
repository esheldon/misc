! vim:set ft=fortran:
program main

    use configlib

    character*256 filename
    type(config) pars


    if (iargc() /= 1) then
        print *,"Usage: test-config filename"
        stop 45
    end if


    call getarg(1,filename)

    call read_config(filename, pars)
    call print_config(pars)

end program
