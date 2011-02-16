! vim:set ft=fortran:
module errlib

contains
    subroutine fatal_error(msg)
        character (len=*), intent(in), optional :: msg
        integer, save :: code = 1

        if (present(msg)) print *,trim(msg)
        print *,'program exits with exit code ', code

        call exit(code)
    end subroutine fatal_error
end module errlib
