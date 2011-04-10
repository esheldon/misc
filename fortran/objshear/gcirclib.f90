! vim:set ft=fortran:
module gcirclib

    implicit none

    integer, parameter, private :: dp  = SELECTED_REAL_KIND(12,200)
    real*8, parameter, private :: PI = 3.141592653589793238462643383279502884197_dp
    real*8, parameter, private :: HALFPI= 1.570796326794896619231321691639751442099_dp
    real*8, parameter, private :: D2R = PI / 180.0_DP

contains

    subroutine gcirc(lon1, lat1, lon2, lat2, dis, theta)
        ! inputs in degrees
        ! outputs in radians
        real*8, intent(in) :: lon1, lat1, lon2, lat2
        real*8, intent(inout) :: dis, theta

        real*8 lat_rad1, lat_rad2, sinlat1, coslat1, sinlat2, coslat2
        real*8 londiff, coslondiff, cosdis

        lat_rad1 = lat1*D2R
        lat_rad2 = lat2*D2R

        sinlat1 = sin(lat_rad1)
        coslat1 = cos(lat_rad1)

        sinlat2 = sin(lat_rad2)
        coslat2 = cos(lat_rad2)

        ! should we reverse this to lon2-lon1?  This produced
        ! a minus sign in the angle theta below
        londiff = (lon2 - lon1)*D2R
        coslondiff = cos( londiff )

        cosdis = sinlat1*sinlat2 + coslat1*coslat2*coslondiff

        if (cosdis < -1.0) cosdis = -1.0
        if (cosdis >  1.0) cosdis =  1.0

        dis = acos(cosdis)

        theta = atan2( sin(londiff), &
                       sinlat1*coslondiff - coslat1*sinlat2/coslat2 ) - HALFPI

    end subroutine gcirc

end module gcirclib
