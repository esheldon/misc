! vim:set ft=fortran:
module cosmolib
    ! class to calculate distances in a flat universe. 
    ! This uses gauss-legendre integration
    ! for extremely fast ad accurate code code.
    !
    ! For integration, 5 points is essentially exact and very fast.
    ! for more generic routines, see the cosmology class in esutil
    ! or cosmology package in esheldon github

    implicit none

    ! class variables
    integer*8, save, private :: has_been_init = 0
    integer*8, save :: npts
    real*8, private, save, dimension(:), allocatable :: xxi, wwi

    real*8, save :: H0
    real*8, save :: omega_m
    real*8, save :: omega_l

    ! The hubble distance c/H0
    real*8, save :: DH

    ! use in scinv for dlens in Mpc
    real*8, private, parameter :: four_pi_G_over_c_squared = 6.0150504541630152e-07


    ! for integral calculations
    real*8, private :: f1,f2,z,ezinv;

    interface angdist
        module procedure angdist_2z
        module procedure angdist_pre
    end interface

    interface sigmacritinv
        module procedure scinv_2z
        module procedure scinv_pre
    end interface
contains

    ! you must initialize
    subroutine cosmo_init(H0_new, omega_m_new, npts_new)

        real*8, intent(in) :: H0_new, omega_m_new
        integer*8, intent(in) :: npts_new

        H0      = H0_new
        omega_m = omega_m_new
        omega_l = 1.0-omega_m
        npts    = npts_new

        DH = 2.99792458e5/100.0

        call set_cosmo_weights(npts)

        has_been_init = 1

    end subroutine cosmo_init

    subroutine set_cosmo_weights(npts_new)

        use intlib
        integer*8, intent(in) :: npts_new
        npts = npts_new
        call gauleg(-1.0_dp, 1.0_dp, npts, xxi, wwi)

    end subroutine set_cosmo_weights

    real*8 function ez_inverse_integral(zmin, zmax) result(val)
        real*8, intent(in) :: zmin, zmax
        integer*8 i


        f1 = (zmax-zmin)/2.
        f2 = (zmax+zmin)/2.

        val = 0.0

        do i=1,npts
            z = xxi(i)*f1 + f2
            ezinv = ez_inverse(z)

            val = val + f1*ezinv*wwi(i);
        end do

    end function ez_inverse_integral

    real*8 function ez_inverse(z)
        real*8, intent(in) :: z

        real*8 oneplusz3

        oneplusz3 = (1.0+z)**3
        ez_inverse = omega_m*oneplusz3 + omega_l;
        ez_inverse = sqrt(1.0/ez_inverse)
    end function ez_inverse



    real*8 function cdist(zmin, zmax)
        ! comoving distance
        real*8, intent(in) :: zmin, zmax
        cdist = DH*ez_inverse_integral(zmin, zmax)
    end function cdist

    real*8 function angdist_2z(zmin, zmax) result(angdist)
        ! angular diameter distance for flat universe is simple in dc
        real*8, intent(in) :: zmin, zmax
        angdist = DH*ez_inverse_integral(zmin, zmax)/(1+zmax)
    end function angdist_2z

    real*8 function angdist_pre(dcmin, dcmax, zmax) result(angdist)
        ! angular diameter distance for flat universe is simple in dc
        real*8, intent(in) :: dcmin, dcmax, zmax
        angdist = (dcmax-dcmin)/(1+zmax)
    end function angdist_pre


    real*8 function scinv_2z(zl, zs) result(scinv)
        ! flat universe, simple in dc
        real*8, intent(in) :: zl,zs
        real*8 dcl, dcs

        if (zs <= zl) then 
            scinv=0.0
            return
        end if

        dcl = cdist(0.0_8, zl)
        dcs = cdist(0.0_8, zs)

        scinv = dcl/(1.+zl)*(dcs-dcl)/dcs * four_pi_G_over_c_squared
    end function scinv_2z

    real*8 function scinv_pre(zl, dcl, dcs) result(scinv)
        ! flat universe, simple in dc
        real*8, intent(in) :: zl,dcl,dcs
        if (dcs <= dcl) then 
            scinv=0.0
            return
        end if

        scinv = dcl/(1.+zl)*(dcs-dcl)/dcs * four_pi_G_over_c_squared
    end function scinv_pre


    subroutine print_cosmo_pars(do_printall)
        logical, optional :: do_printall
        logical printall

        integer*8 i

        if (has_been_init == 0) then
            print '(a)',"cosmo has not yet been init"
            return
        end if

        printall = .false.
        if (present(do_printall)) then
            printall = do_printall
        end if

        write(*,'(a)')"Cosmo parameters: "
        write(*,'("    H0      = ",f6.2)')H0
        write(*,'("    omega_m = ",f6.2)')omega_m
        write(*,'("    npts    = ",i0)')npts
        write(*,'("    DH      = ",f15.8)')DH

        if (printall) then
            print *,'printing all'
            do i=1,size(xxi)
                write(*,'(2f15.8)')xxi(i),wwi(i)
            end do
        end if
    end subroutine print_cosmo_pars

end module cosmolib
