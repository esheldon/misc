! vim:set ft=fortran:
module intlib

    integer, parameter, public :: dp  = SELECTED_REAL_KIND(12,200)
    real*8, parameter, public :: M_PI    = 3.141592653589793238462643383279502884197_dp

contains

    ! from numerical recipes
    subroutine gauleg(x1, x2, npts, x, w)
        use arrlib

        real*8, intent(in) :: x1, x2
        integer*4, intent(in) :: npts

        real*4, intent(inout), dimension(:), allocatable :: x, w
        

        integer*4 :: i, j, m
        real*8 :: xm, xl, z1, z, p1, p2, p3, pp, EPS, abszdiff


        call reallocatef4(x, npts)
        call reallocatef4(w, npts)

        pp = 0.0
        EPS = 4.e-11

        m = (npts + 1)/2

        xm = (x1 + x2)/2.0
        xl = (x2 - x1)/2.0
        z1 = 0.0

        do i=1,m

            z=cos( M_PI*(i-0.25)/(npts+.5) )

            abszdiff = abs(z-z1)

            do while (abszdiff > EPS) 

                p1 = 1.0
                p2 = 0.0
                do j=1,npts
                    p3 = p2
                    p2 = p1
                    p1 = ( (2.0*j - 1.0)*z*p2 - (j-1.0)*p3 )/j
                end do
                pp = npts*(z*p1 - p2)/(z*z -1.)
                z1=z
                z=z1 - p1/pp

                abszdiff = abs(z-z1)

            end do

            x(i) = xm - xl*z
            x(npts+1-i) = xm + xl*z
            w(i) = 2.0*xl/( (1.-z*z)*pp*pp )
            w(npts+1-i) = w(i)


        end do

    end subroutine gauleg

end module intlib
