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


contains

    subroutine read_config(filename, pars)
        use fileutil

        character(len=*) :: filename
        type(config) pars

        character(255) lensin_dir
        character(255) lensout_dir
        character(255) tmp

        integer :: lun
        lun = get_lun()


        print '("Reading config file (",i0,"): ",a)',lun,trim(filename)
        open(unit=lun,file=filename,status='OLD')

        read(lun,'(a)') tmp
        read(lun,'(a)') tmp
        pars%lens_file = trim(adjustl(tmp))

        read(lun,'(a)') tmp
        read(lun,'(a)') tmp
        pars%source_file = trim(adjustl(tmp))

        read(lun,'(a)') tmp
        read(lun,'(a)') tmp
        pars%output_file = trim(adjustl(tmp))

        read(lun,'(a)')tmp
        read(lun,*)pars%h0
        read(lun,'(a)')tmp
        read(lun,*)pars%omega_m

        read(lun,'(a)')tmp
        read(lun,*)pars%npts


        read(lun,'(a)')tmp
        read(lun,*)pars%nside


        read(lun,'(a)')tmp
        read(lun,*)pars%sigmacrit_style

        read(lun,'(a)')tmp
        read(lun,*)pars%nbin

        read(lun,'(a)')tmp
        read(lun,*)pars%rmin
        read(lun,'(a)')tmp
        read(lun,*)pars%rmax


        close(lun)

        pars%log_rmin = log10(pars%rmin);
        pars%log_rmax = log10(pars%rmax);
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
