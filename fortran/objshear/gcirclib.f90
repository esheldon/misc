! vim:set ft=fortran:
module gcirclib

    implicit none

    integer, parameter, private :: dp  = SELECTED_REAL_KIND(12,200)
    real*8, parameter, private :: PI = 3.141592653589793238462643383279502884197_dp
    real*8, parameter, private :: HALFPI= 1.570796326794896619231321691639751442099_dp
    real*8, parameter, private :: D2R = PI / 180.0_DP

contains

    ! to match faster code in shearlib, lon2,lat2 should be
    ! the sources because of lon2-lon1 in londiff: changes sign of theta
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

        ! should we reverse this to lon2-lon1?  This produces
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



    ! The returned angle theta works properly with how I transformed from
    ! pixels to lambda,eta in the SDSS
    !  
    ! lam2,eta2 was the sources
    subroutine gcirc_survey(lam1, eta1, lam2, eta2, dis,  theta)
        real*8 lam1, eta1, lam2, eta2, dis, theta
        real*8 tlam1, coslam1, sinlam1
        real*8 tlam2, coslam2, sinlam2
        real*8 etadiff, cosetadiff, sinetadiff, cosdis

        tlam1 = lam1*D2R
        coslam1 = cos(tlam1)
        !sinlam1 = sign(tlam1)*sqrt(1.-coslam1**2)
        sinlam1 = sqrt(1.-coslam1**2)
        if (tlam1 < 0.) then
            sinlam1 = -sinlam1
        endif

        tlam2 = lam2*D2R
        coslam2 = cos(tlam2)
        !sinlam2 = sign(tlam2)*sqrt(1.-coslam2**2)
        sinlam2 = sqrt(1.-coslam2**2)
        if (tlam2 < 0.) then
            sinlam2 = -sinlam2
        endif


        etadiff = (eta2-eta1)*D2R
        cosetadiff = cos(etadiff)
        !sinetadiff = sign(etadiff)*sqrt(1.0-cosetadiff*cosetadiff);
        sinetadiff = sqrt(1.0-cosetadiff*cosetadiff);
        if (etadiff < 0.) then
            sinetadiff = -sinetadiff
        endif

        cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff

        if (cosdis < -1.) then
            cosdis=-1.0;
        else if (cosdis >  1.) then
            cosdis= 1.0;
        endif

        dis = acos(cosdis);

        theta = atan2( sinetadiff, (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - HALFPI;

    end subroutine gcirc_survey


end module gcirclib
