! vim:set ft=fortran:
program main

    use cosmolib

    real*4 z,zl,zs,ezinv,ezint,dcl,dcs, dl, ds, dls,scinv

    call print_cosmo_pars(.true.) 
    print '(a)','init to 100 0.25 3'
    call cosmo_init(100.0, 0.27, 3)
    call print_cosmo_pars(.true.)
    call cosmo_init(100.0, 0.3, 5)
    call print_cosmo_pars(.true.)

    z = 0.7
    ezinv = ez_inverse(z)
    print '("ez_inverse(",f6.2,"): ",E15.8)',z,ezinv

    ezint = ez_inverse_integral(0.0, z)
    print '("ez_inverse(0.0, ",f6.2,"): ",E15.8)',z,ezint

    zl = 0.4
    zs = 0.9
    dls = angdist(zl, zs)

    print '("angdist(",f6.2,", ",f6.2,"): ",F15.8," Mpc")',zl,zs,dls

    dcl = cdist(0.0, zl)
    dcs = cdist(0.0, zs)
    dls = angdist(dcl, dcs, zs)
    print '("angdist(",f15.8,", ",f15.8,", ",f6.2,"): ",F15.8," Mpc")',dcl,dcs,zs,dls

    scinv = sigmacritinv(zl, zs)
    print '("scinv(",f6.2,", ",f6.2,"): ",E15.8," pc^2/Msun")',zl,zs,scinv

    scinv = sigmacritinv(zl, dcl, dcs)
    print '("scinv(",f6.2,", ",f15.8,", ",f15.8,"): ",E15.8," pc^2/Msun")',zl,dcl,dcs,scinv

end program
