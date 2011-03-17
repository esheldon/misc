! vim:set ft=fortran:

module bit_manipulation

    implicit none

    private

    integer*4, parameter :: oddbits=89478485,evenbits=178956970

    public :: swapLSBMSB, invswapLSBMSB, invLSB, invMSB

contains

    !! Returns i with even and odd bit positions interchanged.
    function swapLSBMSB(i)
        integer*4 :: swapLSBMSB
        integer*4, intent(in) :: i

        swapLSBMSB = IAND(i,evenbits)/2 + IAND(i,oddbits)*2
    end function swapLSBMSB

    !! Returns NOT(i) with even and odd bit positions interchanged.
    function invswapLSBMSB(i)
        integer*4 :: invswapLSBMSB
        integer*4, intent(in) :: i

        invswapLSBMSB = NOT(swapLSBMSB(i))
    end function invswapLSBMSB

    !! Returns i with odd (1,3,5,...) bits inverted.
    function invLSB(i)
        integer*4 :: invLSB
        integer*4, intent(in) :: i

        invLSB = IEOR(i,oddbits)
    end function invLSB

    !! Returns i with even (0,2,4,...) bits inverted.
    function invMSB(i)
        integer*4 :: invMSB
        integer*4, intent(in) :: i

        invMSB = IEOR(i,evenbits)
    end function invMSB

end module bit_manipulation


module healpix

    integer, parameter, public :: lgt = KIND(.TRUE.)
    integer, parameter, public :: dp  = SELECTED_REAL_KIND(12,200)
    real*8, parameter, public :: TWOTHIRD = 0.6666666666666666666666666666666666666666_dp
    real*8, parameter, public :: HALFPI= 1.570796326794896619231321691639751442099_dp
    real*8, parameter, public :: PI    = 3.141592653589793238462643383279502884197_dp
    real*8, parameter, public :: TWOPI = 6.283185307179586476925286766559005768394_dp

    real*8, parameter, public :: RAD2DEG = 180.0_DP / PI
    real*8, parameter, public :: DEG2RAD = PI / 180.0_DP
    integer*4, parameter, private :: ns_max=8192 ! 2^13 : largest nside available

contains

    integer*4 function npix(nside)
        integer*4, intent(in) :: nside
        npix = 12*nside*nside
    end function npix

    real*8 function pixarea(nside)
        integer*4, intent(in) :: nside

        integer*4 np
        np = npix(nside)
        pixarea = 2.0*TWOPI/np
    end function pixarea


    subroutine eq2pix_ring(nside, ra, dec, ipix)
        !=======================================================================
        !     renders the pixel number ipix (RING scheme) for a pixel which contains
        !     a point on a sphere at coordinates theta and phi, given the map
        !     resolution parameter nside
        !=======================================================================

        integer*4, intent(in) :: nside
        real*8, intent(in) :: ra, dec

        integer*4, intent(out) :: ipix

        real*8 :: theta, phi

        integer*4 ::  nl4, jp, jm
        real*8 ::  z, za, tt, tp, tmp, temp1, temp2
        integer*4 ::  ir, ip, kshift

        call radec_degrees_to_thetaphi_radians(ra, dec, theta, phi)

        !-----------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error ("nside out of range")
        if (theta<0.0_dp .or. theta>pi)  then
            print *,"eq2pix_ring: theta : ",theta," is out of range [0, Pi]"
            call fatal_error
        endif

        z = COS(theta)
        za = ABS(z)
        tt = MODULO( phi, twopi) / halfpi  ! in [0,4)


        if ( za <= twothird ) then ! Equatorial region ------------------
            temp1 = nside*(.5_dp+tt)
            temp2 = nside*.75_dp*z
            jp = int(temp1-temp2) ! index of  ascending edge line
            jm = int(temp1+temp2) ! index of descending edge line

            ir = nside + 1 + jp - jm ! in {1,2n+1} (ring number counted from z=2/3)
            kshift = 1 - modulo(ir,2) ! kshift=1 if ir even, 0 otherwise

            nl4 = 4*nside
            ip = INT( ( jp+jm - nside + kshift + 1 ) / 2 ) ! in {0,4n-1}
            if (ip >= nl4) ip = ip - nl4

            ipix = 2*nside*(nside-1) + nl4*(ir-1) + ip

        else ! North & South polar caps -----------------------------

            tp = tt - INT(tt)      !MODULO(tt,1.0_dp)
            tmp = nside * SQRT( 3.0_dp*(1.0_dp - za) )

            jp = INT(tp          * tmp ) ! increasing edge line index
            jm = INT((1.0_dp - tp) * tmp ) ! decreasing edge line index

            ir = jp + jm + 1        ! ring number counted from the closest pole
            ip = INT( tt * ir )     ! in {0,4*ir-1}
            if (ip >= 4*ir) ip = ip - 4*ir

            if (z>0._dp) then
                ipix = 2*ir*(ir-1) + ip
            else
                ipix = 12*nside**2 - 2*ir*(ir+1) + ip
            endif

        endif

        return
    end subroutine eq2pix_ring


    subroutine query_disc ( nside, ra, dec, radius, listpix, nlist, inclusive)
        !=======================================================================
        !
        !      query_disc (Nside, Vector0, Radius, Listpix, Nlist[, Inclusive])
        !      ----------
        !      routine for pixel query in the RING scheme
        !      all pixels within an angular distance Radius of the center
        !
        !     Nside    = resolution parameter (a power of 2)
        !     ra,dec   = central point vector position (x,y,z in double precision)
        !     Radius   = angular radius in RADIAN (in double precision)
        !     Listpix  = list of pixel closer to the center (angular dist) than Radius
        !     Nlist    = number of pixels in the list
        !     inclusive (OPT) , :0 by default, only the pixels whose center
        !                       lie in the triangle are listed on output
        !                  if set to 1, all pixels overlapping the triangle are output
        !
        !      * all pixel numbers are in {0, 12*Nside*Nside - 1}
        !     NB : the dimension of the listpix array is fixed in the calling
        !     routine and should be large enough for the specific configuration
        !
        !      lower level subroutines called by getdisc_ring :
        !       (you don't need to know them)
        !      ring_num (nside, ir)
        !      --------
        !      in_ring(nside, iz, phi0, dphi, listir, nir)
        !      -------
        !
        ! v1.0, EH, TAC, ??
        ! v1.1, EH, Caltech, Dec-2001
        ! v1.2, EH, IAP, 2008-03-30: fixed bug appearing when disc centered on 
        !           either pole
        !=======================================================================
        integer*4, intent(in)                 :: nside
        real*8,    intent(in)                 :: ra,dec
        real*8,    intent(in)                 :: radius
        integer*4, intent(inout), dimension(:) :: listpix
        integer*4, intent(out)                :: nlist
        integer*4, intent(in), optional       :: inclusive

        real*8, dimension(3)  :: vector0

        integer*4 :: irmin, irmax, ilist, iz, ip, nir, npix
        real*8 :: norm_vect0
        real*8 :: x0, y0, z0, radius_eff, fudge
        real*8 :: a, b, c, cosang
        real*8 :: dth1, dth2
        real*8 :: phi0, cosphi0, cosdphi, dphi
        real*8 :: rlat0, rlat1, rlat2, zmin, zmax, z
        integer*4, DIMENSION(:),   ALLOCATABLE  :: listir
        integer*4 :: status
        character(len=*), parameter :: code = "QUERY_DISC"
        integer*4 :: list_size, nlost
        logical(LGT) :: do_inclusive

        !=======================================================================

        call eq2vec(ra, dec, vector0)

        list_size = size(listpix)
        !     ---------- check inputs ----------------
        npix = 12 * nside * nside

        if (radius < 0.0_dp .or. radius > PI) then
           write(unit=*,fmt="(a)") code//"> the angular radius is in RADIAN "
           write(unit=*,fmt="(a)") code//"> and should lie in [0,Pi] "
           call fatal_error("> program abort ")
        endif

        do_inclusive = .false.
        if (present(inclusive)) then
           if (inclusive == 1) do_inclusive = .true.
        endif

        !     --------- allocate memory -------------
        ALLOCATE( listir(0: 4*nside-1), STAT = status)
        if (status /= 0) then
           write(unit=*,fmt="(a)") code//"> can not allocate memory for listir :"
           call fatal_error("> program abort ")
        endif

        dth1 = 1.0_dp / (3.0_dp*real(nside,kind=dp)**2)
        dth2 = 2.0_dp / (3.0_dp*real(nside,kind=dp))

        radius_eff = radius
        if (do_inclusive) then
        !        fudge = PI / (4.0_dp*nside) ! increase radius by half pixel size
           fudge = acos(TWOTHIRD) / real(nside,kind=dp) ! 1.071* half pixel size
           radius_eff = radius + fudge
        endif
        cosang = COS(radius_eff)

        !     ---------- circle center -------------
        norm_vect0 =  SQRT(DOT_PRODUCT(vector0,vector0))
        x0 = vector0(1) / norm_vect0
        y0 = vector0(2) / norm_vect0
        z0 = vector0(3) / norm_vect0

        phi0=0.0_dp
        if ((x0/=0.0_dp).or.(y0/=0.0_dp)) phi0 = ATAN2 (y0, x0)  ! in ]-Pi, Pi]
        cosphi0 = COS(phi0)
        a = x0*x0 + y0*y0

        !     --- coordinate z of highest and lowest points in the disc ---
        rlat0  = ASIN(z0)    ! latitude in RAD of the center
        rlat1  = rlat0 + radius_eff
        rlat2  = rlat0 - radius_eff
        if (rlat1 >=  halfpi) then
           zmax =  1.0_dp
        else
           zmax = SIN(rlat1)
        endif
        irmin = ring_num(nside, zmax)
        irmin = MAX(1, irmin - 1) ! start from a higher point, to be safe

        if (rlat2 <= -halfpi) then
           zmin = -1.0_dp
        else
           zmin = SIN(rlat2)
        endif
        irmax = ring_num(nside, zmin)
        irmax = MIN(4*nside-1, irmax + 1) ! go down to a lower point

        ilist = -1

        !     ------------- loop on ring number ---------------------
        do iz = irmin, irmax

           if (iz <= nside-1) then      ! north polar cap
              z = 1.0_dp  - real(iz,kind=dp)**2 * dth1
           else if (iz <= 3*nside) then    ! tropical band + equat.
              z = real(2*nside-iz,kind=dp) * dth2
           else
              z = - 1.0_dp + real(4*nside-iz,kind=dp)**2 * dth1
           endif

           !        --------- phi range in the disc for each z ---------
           b = cosang - z*z0
           c = 1.0_dp - z*z
           if ((x0==0.0_dp).and.(y0==0.0_dp)) then
              dphi=PI
              if (b > 0.0_dp) goto 1000 ! out of the disc, 2008-03-30
              goto 500
           endif
           cosdphi = b / SQRT(a*c)
           if (ABS(cosdphi) <= 1.0_dp) then
              dphi = ACOS (cosdphi) ! in [0,Pi]
           else
              if (cosphi0 < cosdphi) goto 1000 ! out of the disc
              dphi = PI ! all the pixels at this elevation are in the disc
           endif
        500    continue

           !        ------- finds pixels in the disc ---------
           call in_ring(nside, iz, phi0, dphi, listir, nir)

           !        ----------- merge pixel lists -----------
           nlost = ilist + nir + 1 - list_size
           if ( nlost > 0 ) then
              print*,code//"> listpix is too short, it will be truncated at ",nir
              print*,"                         pixels lost : ", nlost
              nir = nir - nlost
           endif
           do ip = 0, nir-1
              ilist = ilist + 1
              listpix(ilist) = listir(ip)
           enddo

        1000   continue
        enddo

        !     ------ total number of pixel in the disc --------
        nlist = ilist + 1


        !     ------- deallocate memory and exit ------
        DEALLOCATE(listir)

        return
    end subroutine query_disc



    !=======================================================================
    function ring_num (nside, z, shift) result(ring_num_result)
        !=======================================================================
        ! ring = ring_num(nside, z [, shift=])
        !     returns the ring number in {1, 4*nside-1}
        !     from the z coordinate
        ! usually returns the ring closest to the z provided
        ! if shift < 0, returns the ring immediatly north (of smaller index) of z
        ! if shift > 0, returns the ring immediatly south (of smaller index) of z
        !
        !=======================================================================
        integer*4             :: ring_num_result
        real*8,     INTENT(IN) :: z
        integer*4, INTENT(IN) :: nside
        integer*4,      intent(in), optional :: shift

        integer*4 :: iring
        real*8 :: my_shift
        !=======================================================================


        my_shift = 0.0_dp
        if (present(shift)) my_shift = shift * 0.5_dp

        !     ----- equatorial regime ---------
        iring = NINT( nside*(2.0_dp-1.500_dp*z)   + my_shift )

        !     ----- north cap ------
        if (z > twothird) then
           iring = NINT( nside* SQRT(3.0_dp*(1.0_dp-z))  + my_shift )
           if (iring == 0) iring = 1
        endif

        !     ----- south cap -----
        if (z < -twothird   ) then
           ! beware that we do a -shift in the south cap
           iring = NINT( nside* SQRT(3.0_dp*(1.0_dp+z))   - my_shift )
           if (iring == 0) iring = 1
           iring = 4*nside - iring
        endif

        ring_num_result = iring

        return
    end function ring_num


    subroutine in_ring (nside, iz, phi0, dphi, listir, nir)
        !=======================================================================
        !     returns the list of pixels in RING scheme (listir)
        !     and their number (nir)
        !     with latitude in [phi0-dphi, phi0+dphi] on the ring ir
        !     (in {1,4*nside-1})
        !     the pixel id-numbers are in {0,12*nside^2-1}
        !     the indexing is RING
        !=======================================================================
        integer*4, intent(in)                 :: nside, iz
        integer*4, intent(out)                :: nir
        real*8,     intent(in)                :: phi0, dphi
        integer*4, intent(out), dimension(0:) :: listir

        !     logical(kind=lgt) :: conservative = .true.
        logical(kind=lgt) :: conservative = .false.
        logical(kind=lgt) :: take_all, to_top, do_ring

        integer*4 :: ip_low, ip_hi, i, in, inext, diff
        integer*4 :: npix, nr, nir1, nir2, ir, ipix1, ipix2, kshift, ncap
        real*8     :: phi_low, phi_hi, shift
        !=======================================================================

        take_all = .false.
        to_top   = .false.
        do_ring  = .true.
        npix = 12 * nside * nside
        ncap  = 2*nside*(nside-1) ! number of pixels in the north polar cap
        listir = -1
        nir = 0

        phi_low = MODULO(phi0 - dphi, twopi)
        phi_hi  = MODULO(phi0 + dphi, twopi)
        if (ABS(dphi-PI) < 1.0e-6_dp) take_all = .true.

        !     ------------ identifies ring number --------------
        if (iz >= nside .and. iz <= 3*nside) then ! equatorial region
           ir = iz - nside + 1  ! in {1, 2*nside + 1}
           ipix1 = ncap + 4*nside*(ir-1) !  lowest pixel number in the ring
           ipix2 = ipix1 + 4*nside - 1   ! highest pixel number in the ring
           kshift = MODULO(ir,2)
           nr = nside*4
        else
           if (iz < nside) then       !    north pole
              ir = iz
              ipix1 = 2*ir*(ir-1)        !  lowest pixel number in the ring
              ipix2 = ipix1 + 4*ir - 1   ! highest pixel number in the ring
           else                          !    south pole
              ir = 4*nside - iz
              ipix1 = npix - 2*ir*(ir+1) !  lowest pixel number in the ring
              ipix2 = ipix1 + 4*ir - 1   ! highest pixel number in the ring
           endif
           nr = ir*4
           kshift = 1
        endif

        !     ----------- constructs the pixel list --------------
        if (take_all) then
           nir    = ipix2 - ipix1 + 1
           listir(0:nir-1) = (/ (i, i=ipix1,ipix2) /)
           return
        endif

        shift = kshift * 0.5_dp
        if (conservative) then
           ! conservative : include every intersected pixels,
           ! even if pixel CENTER is not in the range [phi_low, phi_hi]
           ip_low = nint (nr * phi_low / TWOPI - shift)
           ip_hi  = nint (nr * phi_hi  / TWOPI - shift)
           ip_low = modulo (ip_low, nr) ! in {0,nr-1}
           ip_hi  = modulo (ip_hi , nr) ! in {0,nr-1}
        else
           ! strict : include only pixels whose CENTER is in [phi_low, phi_hi]
           ip_low = ceiling (nr * phi_low / TWOPI - shift)
           ip_hi  = floor   (nr * phi_hi  / TWOPI - shift)
        !        if ((ip_low - ip_hi == 1) .and. (dphi*nr < PI)) then ! EH, 2004-06-01
           diff = modulo(ip_low - ip_hi, nr) ! in {-nr+1, nr-1} or {0,nr-1} ???
           if (diff < 0) diff = diff + nr    ! in {0,nr-1}
           if ((diff == 1) .and. (dphi*nr < PI)) then
              ! the interval is so small (and away from pixel center)
              ! that no pixel is included in it
              nir = 0
              return
           endif
        !        ip_low = min(ip_low, nr-1) !  EH, 2004-05-28
        !        ip_hi  = max(ip_hi , 0   )
           if (ip_low >= nr) ip_low = ip_low - nr
           if (ip_hi  <  0 ) ip_hi  = ip_hi  + nr
        endif
        !
        if (ip_low > ip_hi) to_top = .true.
        ip_low = ip_low + ipix1
        ip_hi  = ip_hi  + ipix1

        if (to_top) then
           nir1 = ipix2 - ip_low + 1
           nir2 = ip_hi - ipix1  + 1
           nir  = nir1 + nir2
           listir(0:nir1-1)   = (/ (i, i=ip_low, ipix2) /)
           listir(nir1:nir-1) = (/ (i, i=ipix1, ip_hi) /)
        else
           nir = ip_hi - ip_low + 1
           listir(0:nir-1) = (/ (i, i=ip_low, ip_hi) /)
        endif

        return
    end subroutine in_ring


    subroutine radec_degrees_to_thetaphi_radians(ra, dec, theta, phi)
        !   ra gets converted to phi:
        !       (longitude measured eastward, in radians [0,2*pi]
        !   dec gets converted to theta:
        !       (co-latitude measured from North pole, in [0,Pi] radians)
        real*8, intent(in) :: ra, dec
        real*8, intent(out) :: theta, phi
        phi = ra*DEG2RAD
        !theta = dec*DEG2RAD + HALFPI
        theta = -dec*DEG2RAD + HALFPI
    end subroutine radec_degrees_to_thetaphi_radians

    subroutine eq2vec(ra, dec, vector)
        !=======================================================================
        !   renders the vector (x,y,z) corresponding to angles
        !
        !   ra gets converted to phi:
        !       (longitude measured eastward, in radians [0,2*pi]
        !   dec gets converted to theta:
        !       (co-latitude measured from North pole, in [0,Pi] radians)
        !
        !   North pole is (x,y,z)=(0,0,1)
        !=======================================================================
        real*8, intent(in) :: ra, dec
        real*8, intent(out), dimension(1:) :: vector

        real*8 theta, phi

        real*8 :: sintheta
        !=======================================================================
        
        call radec_degrees_to_thetaphi_radians(ra, dec, theta, phi)

        if (theta<0.0_dp .or. theta>pi)  then
           print*,"ANG2VEC: theta : ",theta," is out of range [0, Pi]"
           call fatal_error
        endif
        sintheta = SIN(theta)

        vector(1) = sintheta * COS(phi)
        vector(2) = sintheta * SIN(phi)
        vector(3) = COS(theta)

        return
    end subroutine eq2vec


    subroutine fatal_error(msg)
        character (len=*), intent(in), optional :: msg
        integer, save :: code = 1

        if (present(msg)) print *,trim(msg)
        print *,'program exits with exit code ', code

        call exit(code)
    end subroutine fatal_error



end module healpix



