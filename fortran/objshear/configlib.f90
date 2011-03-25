! vim:set ft=fortran:
module configlib

    implicit none

    type config
        character*256 lens_file
        character*256 source_file
        character*256 output_file

        real*8 H0
        real*8 omega_m
        integer*4 npts  ! for cosmo integration

        integer*4 nside ! hpix

        integer*4 sigmacrit_style

        integer*4 nbin
        real*8 rmin
        real*8 rmax

        ! we fill these in
        real*8 log_rmin
        real*8 log_rmax
        real*8 log_binsize

    end type config

    !interface read_conf
    !    module procedure read_conf_F8
    !    module procedure read_conf_I4
    !    module procedure read_conf_I8
    !    module procedure read_conf_string
    !end interface

contains

    real*8 function read_conf_F8(lun) result(val)
        integer, intent(in) :: lun
        character(255) key_val_pair

        read(lun,'(a)') key_val_pair

        val = conf_extractF8(key_val_pair)
    end function

    integer*8 function read_conf_I8(lun) result(val)
        integer, intent(in) :: lun
        character(255) key_val_pair

        read(lun,'(a)') key_val_pair

        val = conf_extractI8(key_val_pair)
    end function
    integer*4 function read_conf_I4(lun) result(val)
        integer, intent(in) :: lun
        character(255) key_val_pair

        read(lun,'(a)') key_val_pair

        val = conf_extractI4(key_val_pair)
    end function

    character(255) function read_conf_string(lun) result(val)
        integer, intent(in) :: lun
        character(255) key_val_pair

        read(lun,'(a)') key_val_pair

        call conf_extract_string(key_val_pair, val)
    end function


    real*8 function conf_extractF8(pair) result(val)
        character(len=*), intent(inout) :: pair

        character(100) key

        pair=adjustl(pair)

        read (pair(scan(pair,' ')+1:),*) val

    end function

    integer*8 function conf_extractI8(pair) result(val)
        character(len=*), intent(inout) :: pair

        character(100) key

        pair=adjustl(pair)

        read (pair(scan(pair,' ')+1:),*) val

    end function
    integer*4 function conf_extractI4(pair) result(val)
        character(len=*), intent(inout) :: pair

        character(100) key

        pair=adjustl(pair)

        read (pair(scan(pair,' ')+1:),*) val

    end function



    subroutine conf_extract_string(pair, val)
        character(len=*), intent(inout) :: pair
        character(len=*), intent(inout) :: val

        character(100) key

        pair=adjustl(pair)

        key=pair(1:scan(pair,' '))
        val=trim(adjustl(pair(scan(pair,' ')+1:)))

    end subroutine



    subroutine read_config(filename, pars)
        use fileutil

        character(len=*) :: filename
        type(config) pars

        integer :: lun
        lun = get_lun()


        print '("Reading config file (",i0,"): ",a)',lun,trim(filename)
        open(unit=lun,file=filename,status='OLD')

        pars%lens_file = read_conf_string(lun)
        pars%source_file = read_conf_string(lun)
        pars%output_file = read_conf_string(lun)

        pars%h0 = read_conf_F8(lun)
        pars%omega_m = read_conf_F8(lun)
        pars%npts = read_conf_I4(lun)
        pars%nside = read_conf_I4(lun)
        pars%sigmacrit_style = read_conf_I4(lun)
        pars%nbin = read_conf_I4(lun)
        pars%rmin = read_conf_F8(lun)
        pars%rmax = read_conf_F8(lun)

        close(lun)

        pars%log_rmin = log10(pars%rmin)
        pars%log_rmax = log10(pars%rmax)
        pars%log_binsize = ( pars%log_rmax - pars%log_rmin )/pars%nbin

    end subroutine read_config


    subroutine print_config(pars)
        type(config), intent(in) :: pars

        print '("    ",a,a)',"lens_file: ",trim(pars%lens_file)
        print '("    ",a,a)',"source_file: ",trim(pars%source_file)
        print '("    ",a,a)',"output_file: ",trim(pars%output_file)
        print '("    ",a,f6.2)',"H0: ",pars%H0
        print '("    ",a,f7.3)',"omega_m: ",pars%omega_m
        print '("    ",a,i0)',"npts: ",pars%npts
        print '("    ",a,i0)',"nside: ",pars%nside
        print '("    ",a,i0)',"sigmacrit_style: ",pars%sigmacrit_style
        print '("    ",a,i0)',"nbin: ",pars%nbin
        print '("    ",a,f15.8)',"rmin: ",pars%rmin
        print '("    ",a,f15.8)',"rmax: ",pars%rmax
        print '("    ",a,f15.8)',"log_rmin: ",pars%log_rmin
        print '("    ",a,f15.8)',"log_rmax: ",pars%log_rmax
        print '("    ",a,f15.8)',"log_binsize: ",pars%log_binsize
    end subroutine print_config

end module configlib
