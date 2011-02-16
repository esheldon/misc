! vim:set ft=fortran:
module fileutil

contains

    function get_lun() result( lun )
 
        integer :: lun

        lun = 9

        lun_search: do

            lun = lun + 1

            ! -- If file unit does not exist we have to exit
            if ( .not. lun_exists( lun ) ) then
                write (*,'(a)')"All file units are in use"
                stop 45
            end if

            ! -- If the file is not open, we're done.
            if ( .not. lun_open(lun)) exit lun_search

        end do lun_search

    end function get_lun

    function lun_exists(lun) result ( existence )
 
        integer, intent( in ) :: lun
        logical :: existence

        inquire(unit=lun, exist=existence)

    end function lun_exists

    function lun_open(lun) result ( isopen )
 
        integer, intent( in ) :: lun
        logical :: isopen 

        inquire(unit=lun, opened=isopen)

    end function lun_open



  end module fileutil
