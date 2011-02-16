! vim:set ft=fortran:
program main

    use gcirclib
    real*8 :: ra1 = 200.0
    real*8 :: dec1 = 0.0
    real*8 :: ra2 = 201.0
    real*8 :: dec2 = 1.0

    real*8 dis, theta

    call gcirc(ra1, dec1, ra2, dec2, dis, theta)

    print '("ra1: ",f15.8," dec1: ",f15.8)',ra1,dec1
    print '("ra2: ",f15.8," dec2: ",f15.8)',ra2,dec2
    print '("dis: ",f15.8," theta: ",f15.8)',dis*(180.0/3.14159),theta*(180.0/3.14159)

end program
