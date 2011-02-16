! vim:set ft=fortran:

module bit_manipulation

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



    !initialise array x2pix, y2pix and pix2x, pix2y used in several routines
    integer*4, private, save, dimension(128) :: x2pix=0,y2pix=0
    integer*4, private, save, dimension(0:1023) :: pix2x=0, pix2y=0

contains

    integer*4 function npix(nside)
        integer*4, intent(in) :: nside
        npix = 12*nside*nside
    end function npix

    real*8 function area(nside)
        integer*4, intent(in) :: nside

        integer*4 np
        np = npix(nside)
        area = 2.0*TWOPI/np
    end function area

    subroutine eq2pix_nest(nside, ra, dec, ipix)
        !=======================================================================
        !     renders the pixel number ipix (NESTED scheme) for a pixel which contains
        !     a point on a sphere at coordinates theta and phi, given the map
        !     resolution parametr nside
        !
        !     the computation is made to the highest resolution available (nside=8192)
        !     and then degraded to that required (by integer division)
        !     this doesn't cost more, and it makes sure
        !     that the treatement of round-off will be consistent
        !     for every resolution
        !=======================================================================

        integer*4, intent(in) :: nside
        real*8, intent(in) :: ra, dec

        integer*4, intent(out) :: ipix

        real*8 :: theta, phi

        real*8 ::  z, za, tt, tp, tmp
        integer*4 :: jp, jm, ifp, ifm, face_num, &
            &     ix, iy, ix_low, ix_hi, iy_low, iy_hi, ipf, ntt


        call radec_degrees_to_thetaphi_radians(ra, dec, theta, phi)

        !-----------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error("nside out of range")
        if (theta<0.0_dp .or. theta>pi)  then
            print*,"eq2pix_nest: theta : ",theta," is out of range [0,Pi]"
            call fatal_error
        endif
        if (x2pix(128) <= 0) call mk_xy2pix()

        z  = COS(theta)
        za = ABS(z)
        tt = MODULO(phi, twopi) / halfpi  ! in [0,4[

        if (za <= twothird) then ! equatorial region

            !        (the index of edge lines increase when the longitude=phi goes up)
            jp = INT(ns_max*(0.5_dp + tt - z*0.75_dp)) !  ascending edge line index
            jm = INT(ns_max*(0.5_dp + tt + z*0.75_dp)) ! descending edge line index

            !        finds the face
            ifp = jp / ns_max  ! in {0,4}
            ifm = jm / ns_max
            if (ifp == ifm) then          ! faces 4 to 7
                face_num = MODULO(ifp,4) + 4
            else if (ifp < ifm) then     ! (half-)faces 0 to 3
                face_num = MODULO(ifp,4)
            else                            ! (half-)faces 8 to 11
                face_num = MODULO(ifm,4) + 8
            endif

            ix = MODULO(jm, ns_max)
            iy = ns_max - MODULO(jp, ns_max) - 1

        else ! polar region, za > 2/3

            ntt = INT(tt)
            if (ntt >= 4) ntt = 3
            tp = tt - ntt
            tmp = SQRT( 3.0_dp*(1.0_dp - za) )  ! in ]0,1]

            ! (the index of edge lines increase when distance from the 
            !  closest pole goes up)
            ! line going toward the pole as phi increases
            jp = INT( ns_max * tp          * tmp ) 
            ! that one goes away of the closest pole
            jm = INT( ns_max * (1.0_dp - tp) * tmp ) 

            jp = MIN(ns_max-1, jp) ! for points too close to the boundary
            jm = MIN(ns_max-1, jm)

            !        finds the face and pixel's (x,y)
            if (z >= 0) then
                face_num = ntt  ! in {0,3}
                ix = ns_max - jm - 1
                iy = ns_max - jp - 1
            else
                face_num = ntt + 8 ! in {8,11}
                ix =  jp
                iy =  jm
            endif

            !         print*,z,face_num,ix,iy
        endif

        ix_low = MODULO(ix,128)
        ix_hi  =     ix/128
        iy_low = MODULO(iy,128)
        iy_hi  =     iy/128

        ipf =  (x2pix(ix_hi +1)+y2pix(iy_hi +1)) * (128 * 128) &
            &     + (x2pix(ix_low+1)+y2pix(iy_low+1))

        ipf = ipf / ( ns_max/nside ) **2  ! in {0, nside**2 - 1}

        ipix = ipf + face_num* nside **2    ! in {0, 12*nside**2 - 1}

        return
    end subroutine eq2pix_nest


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


    subroutine query_disc ( nside, ra, dec, radius, listpix, nlist, nest, inclusive)
        !=======================================================================
        !
        !      query_disc (Nside, Vector0, Radius, Listpix, Nlist[, Nest, Inclusive])
        !      ----------
        !      routine for pixel query in the RING or NESTED scheme
        !      all pixels within an angular distance Radius of the center
        !
        !     Nside    = resolution parameter (a power of 2)
        !     ra,dec   = central point vector position (x,y,z in double precision)
        !     Radius   = angular radius in RADIAN (in double precision)
        !     Listpix  = list of pixel closer to the center (angular dist) than Radius
        !     Nlist    = number of pixels in the list
        !     nest  (OPT), :0 by default, the output list is in RING scheme
        !                  if set to 1, the output list is in NESTED scheme
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
        !      in_ring(nside, iz, phi0, dphi, listir, nir, nest=nest)
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
        integer*4, intent(out), dimension(0:) :: listpix
        integer*4, intent(out)                :: nlist
        integer*4, intent(in), optional       :: nest
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
        integer*4                                :: my_nest

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

        my_nest = 0
        if (present(nest)) then
           if (nest == 0 .or. nest == 1) then
              my_nest = nest
           else
              print*,code//"> NEST should be 0 or 1"
              call fatal_error("> program abort ")
           endif
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
           call in_ring(nside, iz, phi0, dphi, listir, nir, nest=my_nest)

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


    subroutine in_ring (nside, iz, phi0, dphi, listir, nir, nest)
        !=======================================================================
        !     returns the list of pixels in RING or NESTED scheme (listir)
        !     and their number (nir)
        !     with latitude in [phi0-dphi, phi0+dphi] on the ring ir
        !     (in {1,4*nside-1})
        !     the pixel id-numbers are in {0,12*nside^2-1}
        !     the indexing is RING, unless NEST is set to 1
        !=======================================================================
        integer*4, intent(in)                 :: nside, iz
        integer*4, intent(out)                :: nir
        real*8,     intent(in)                :: phi0, dphi
        integer*4, intent(out), dimension(0:) :: listir
        integer*4, intent(in), optional       :: nest

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
        if (present(nest)) then
           do_ring = (nest == 0)
        endif
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
           if (do_ring) then
              listir(0:nir-1) = (/ (i, i=ipix1,ipix2) /)
           else
              call ring2nest(nside, ipix1, in)
              listir(0) = in
              do i=1,nir-1
                 call next_in_line_nest(nside, in, inext)
                 in = inext
                 listir(i) = in
              enddo
           endif
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
           if (do_ring) then
              listir(0:nir1-1)   = (/ (i, i=ip_low, ipix2) /)
              listir(nir1:nir-1) = (/ (i, i=ipix1, ip_hi) /)
           else
              call ring2nest(nside, ip_low, in)
              listir(0) = in
              do i=1,nir-1
                 call next_in_line_nest(nside, in, inext)
                 in = inext
                 listir(i) = in
              enddo
           endif
        else
           nir = ip_hi - ip_low + 1
           if (do_ring) then
              listir(0:nir-1) = (/ (i, i=ip_low, ip_hi) /)
           else
              call ring2nest(nside, ip_low, in)
              listir(0) = in
              do i=1,nir-1
                 call next_in_line_nest(nside, in, inext)
                 in = inext
                 listir(i) = in
              enddo
           endif
        endif

        return
    end subroutine in_ring


    subroutine ring2nest(nside, ipring, ipnest)
        !=======================================================================
        !     performs conversion from RING to NESTED pixel number
        !=======================================================================
        integer*4, INTENT(IN) :: nside, ipring
        integer*4, INTENT(OUT) :: ipnest

        real*8 :: fihip, hip
        integer*4 :: npix, nl2, nl4, ncap, ip, iphi, ipt, ipring1, &
             &     kshift, face_num, nr, &
             &     irn, ire, irm, irs, irt, ifm , ifp, &
             &     ix, iy, ix_low, ix_hi, iy_low, iy_hi, ipf

        ! coordinate of the lowest corner of each face
        integer*4, dimension(1:12) :: jrll = (/ 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4 /) ! in unit of nside
        integer*4, dimension(1:12) :: jpll = (/ 1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7 /) ! in unit of nside/2
        !-----------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error("nside out of range")
        npix = 12*nside**2      ! total number of points
        if (ipring <0 .or. ipring>npix-1) call fatal_error("ipring out of range")
        if (x2pix(128) <= 0) call mk_xy2pix()

        nl2 = 2*nside
        nl4 = 4*nside
        ncap = nl2*(nside-1) ! points in each polar cap, =0 for nside =1
        ipring1 = ipring + 1

        !     finds the ring number, the position of the ring and the face number
        if (ipring1 <= ncap) then ! north polar cap

           hip   = ipring1/2.0_dp
           fihip = AINT ( hip ,kind=DP)
           irn   = INT( SQRT( hip - SQRT(fihip) ) ) + 1 ! counted from North pole
           iphi  = ipring1 - 2*irn*(irn - 1)

           kshift = 0
           nr = irn                  ! 1/4 of the number of points on the current ring
           face_num = (iphi-1) / irn ! in {0,3}

        elseif (ipring1 <= nl2*(5*nside+1)) then ! equatorial region

           ip    = ipring1 - ncap - 1
           irn   = INT( ip / nl4 ) + nside               ! counted from North pole
           iphi  = MODULO(ip,nl4) + 1

           kshift  = MODULO(irn+nside,2)  ! 1 if irn+nside is odd, 0 otherwise
           nr = nside
           ire =  irn - nside + 1 ! in {1, 2*nside +1}
           irm =  nl2 + 2 - ire
           ifm = (iphi - ire/2 + nside -1) / nside ! face boundary
           ifp = (iphi - irm/2 + nside -1) / nside
           if (ifp == ifm) then          ! faces 4 to 7
              face_num = MODULO(ifp,4) + 4
           else if (ifp + 1 == ifm) then ! (half-)faces 0 to 3
              face_num = ifp
           else if (ifp - 1 == ifm) then ! (half-)faces 8 to 11
              face_num = ifp + 7
           endif

        else ! south polar cap

           ip    = npix - ipring1 + 1
           hip   = ip/2.0_dp
           fihip = AINT ( hip ,kind=DP)
           irs   = INT( SQRT( hip - SQRT(fihip) ) ) + 1  ! counted from South pole
           iphi  = 4*irs + 1 - (ip - 2*irs*(irs-1))

           kshift = 0
           nr = irs
           irn   = nl4 - irs
           face_num = (iphi-1) / irs + 8 ! in {8,11}

        endif

        !     finds the (x,y) on the face
        irt =   irn  - jrll(face_num+1)*nside + 1       ! in {-nside+1,0}
        ipt = 2*iphi - jpll(face_num+1)*nr - kshift - 1 ! in {-nside+1,nside-1}
        if (ipt >= nl2) ipt = ipt - 8*nside ! for the face #4

        ix =  (ipt - irt ) / 2
        iy = -(ipt + irt ) / 2

        ix_low = MODULO(ix,128)
        ix_hi  = ix/128
        iy_low = MODULO(iy,128)
        iy_hi  = iy/128

        ipf =  (x2pix(ix_hi +1)+y2pix(iy_hi +1)) * (128 * 128) &
             &     + (x2pix(ix_low+1)+y2pix(iy_low+1))        ! in {0, nside**2 - 1}


        ipnest = ipf + face_num* nside **2    ! in {0, 12*nside**2 - 1}

        return
    end subroutine ring2nest


    subroutine next_in_line_nest(nside, ipix, inext)
        !====================================================================
        !   given nside and a NESTED pixel number ipix, returns in inext
        !  the pixel that lies on the East side (and the same latitude) as ipix
        !
        !   Hacked by EH from BDW's neighbours_nest, 2001-12-18
        !   Hacked for Nside=1 by EH, 2004-05-28
        !====================================================================
        use bit_manipulation
        integer*4, intent(in)::nside, ipix
        integer*4, intent(out):: inext

        integer*4 :: npix,ipf,ipo,ix,ixp,iy,iym,ixo,iyo
        integer*4 :: face_num,other_face
        integer*4 :: ia,ib,ibp,ibm,ib2,icase,nsidesq
        integer*4 :: local_magic1,local_magic2

        !--------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error("nside out of range")
        nsidesq=nside*nside
        npix = 12*nsidesq       ! total number of points
        if (ipix <0 .or. ipix>npix-1) call fatal_error("ipix out of range")

        ! quick and dirty hack for Nside=1
        if (nside == 1) then
           inext = ipix + 1
           if (ipix == 3)  inext = 0
           if (ipix == 7)  inext = 4
           if (ipix == 11) inext = 8
           return
        endif
        !     initiates array for (x,y)-> pixel number -> (x,y) mapping
        if (x2pix(128) <= 0) call mk_xy2pix()

        local_magic1=(nsidesq-1)/3
        local_magic2=2*local_magic1
        face_num=ipix/nsidesq

        ipf=modulo(ipix,nsidesq)   !Pixel number in face

        call pix2xy_nest(nside,ipf,ix,iy)
        ixp=ix+1
        iym=iy-1

        !     Exclude corners
        if(ipf==local_magic2)     then !WestCorner
           inext = ipix - 1
           return
        endif
        if(ipf==(nsidesq-1)) then !NorthCorner
           icase=6
           goto 100
        endif
        if(ipf==0)           then !SouthCorner
           icase=7
           goto 100
        endif
        if(ipf==local_magic1)     then !EastCorner
           icase=8
           goto 100
        endif

        !     Detect edges
        if(IAND(ipf,local_magic1)==local_magic1) then !NorthEast
           icase=1
           goto 100
        endif
        if(IAND(ipf,local_magic2)==0)      then !SouthEast
           icase=4
           goto 100
        endif

        !     Inside a face
        call xy2pix_nest(nside, ixp, iym, face_num, inext)
        return

        100 continue

        ia= face_num/4            !in {0,2}
        ib= modulo(face_num,4)       !in {0,3}
        ibp=modulo(ib+1,4)
        ibm=modulo(ib+4-1,4)
        ib2=modulo(ib+2,4)

        if(ia==0) then          !North Pole region
           select case(icase)
           case(1)              !NorthEast edge
              other_face=0+ibp
              ipo=modulo(swapLSBMSB(ipf),nsidesq)    !East-West flip
              inext = other_face*nsidesq+ipo         ! (6)
           case(4)              !SouthEast edge
              other_face=4+ibp
              ipo=modulo(invMSB(ipf),nsidesq) !SE-NW flip
              call pix2xy_nest(nside,ipo,ixo,iyo)
              call xy2pix_nest(nside, ixo+1, iyo, other_face, inext)
           case(6)              !North corner
              other_face=0+ibp
              inext=other_face*nsidesq+nsidesq-1
           case(7)              !South corner
              other_face=4+ibp
              inext=other_face*nsidesq+local_magic2+1
           case(8)              !East corner
              other_face=0+ibp
              inext=other_face*nsidesq+local_magic2
           end select ! north

        elseif(ia==1) then      !Equatorial region
           select case(icase)
           case(1)              !NorthEast edge
              other_face=0+ib
              ipo=modulo(invLSB(ipf),nsidesq)    !NE-SW flip
              call pix2xy_nest(nside,ipo,ixo,iyo)
              call xy2pix_nest(nside, ixo, iyo-1, other_face, inext)
           case(4)              !SouthEast edge
              other_face=8+ib
              ipo=modulo(invMSB(ipf),nsidesq) !SE-NW flip
              call pix2xy_nest(nside,ipo,ixo,iyo)
              call xy2pix_nest(nside, ixo+1, iyo, other_face, inext)
           case(6)              !North corner
              other_face=0+ib
              inext=other_face*nsidesq+local_magic2-2
           case(7)              !South corner
              other_face=8+ib
              inext=other_face*nsidesq+local_magic2+1
           case(8)              !East corner
              other_face=4+ibp
              inext=other_face*nsidesq+local_magic2
           end select ! equator
        else                    !South Pole region
           select case(icase)
           case(1)              !NorthEast edge
              other_face=4+ibp
              ipo=modulo(invLSB(ipf),nsidesq)    !NE-SW flip
              call pix2xy_nest(nside,ipo,ixo,iyo)
              call xy2pix_nest(nside, ixo, iyo-1, other_face, inext)
           case(4)              !SouthEast edge
              other_face=8+ibp
              ipo=modulo(swapLSBMSB(ipf),nsidesq) !E-W flip
              inext = other_face*nsidesq+ipo   ! (8)
           case(6)              !North corner
              other_face=4+ibp
              inext=other_face*nsidesq+local_magic2 -2
           case(7)              !South corner
              other_face=8+ibp
              inext=other_face*nsidesq
           case(8)              !East corner
              other_face=8+ibp
              inext=other_face*nsidesq+local_magic2
           end select ! south
        endif

        return
    end subroutine next_in_line_nest


    subroutine xy2pix_nest(nside, ix, iy, face_num, ipix)
        !=======================================================================
        !     gives the pixel number ipix (NESTED)
        !     corresponding to ix, iy and face_num
        !
        !     Benjamin D. Wandelt 13/10/97
        !     using code from HEALPIX toolkit by K.Gorski and E. Hivon
        !=======================================================================
        integer*4, intent(in) ::  nside, ix, iy, face_num
        integer*4, intent(out) :: ipix
        integer*4 ::  ix_low, ix_hi, iy_low, iy_hi, ipf

        !-----------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error("nside out of range")
        if (ix<0 .or. ix>(nside-1)) call fatal_error("ix out of range")
        if (iy<0 .or. iy>(nside-1)) call fatal_error("iy out of range")
        if (x2pix(128) <= 0) call mk_xy2pix()

        ix_low = MODULO(ix,128)
        ix_hi  =     ix/128
        iy_low = MODULO(iy,128)
        iy_hi  =     iy/128

        ipf =  (x2pix(ix_hi +1)+y2pix(iy_hi +1)) * (128 * 128) &
             &     + (x2pix(ix_low+1)+y2pix(iy_low+1))

        ipix = ipf + face_num* nside **2    ! in {0, 12*nside**2 - 1}
        return
    end subroutine xy2pix_nest

    subroutine pix2xy_nest(nside, ipf, ix, iy)
        !=======================================================================
        !     gives the x, y coords in a face from pixel number within the face (NESTED)
        !
        !     Benjamin D. Wandelt 13/10/97
        !
        !     using code from HEALPIX toolkit by K.Gorski and E. Hivon
        !=======================================================================
        integer*4, intent(in) :: nside, ipf
        integer*4, intent(out) :: ix, iy

        integer*4 ::  ip_low, ip_trunc, ip_med, ip_hi

        !-----------------------------------------------------------------------
        if (nside<1 .or. nside>ns_max) call fatal_error("nside out of range")
        if (ipf <0 .or. ipf>nside*nside-1) &
             &     call fatal_error("ipix out of range")
        if (pix2x(1023) <= 0) call mk_pix2xy()

        ip_low = MODULO(ipf,1024)       ! content of the last 10 bits
        ip_trunc =   ipf/1024        ! truncation of the last 10 bits
        ip_med = MODULO(ip_trunc,1024)  ! content of the next 10 bits
        ip_hi  =     ip_trunc/1024   ! content of the high weight 10 bits

        ix = 1024*pix2x(ip_hi) + 32*pix2x(ip_med) + pix2x(ip_low)
        iy = 1024*pix2y(ip_hi) + 32*pix2y(ip_med) + pix2y(ip_low)
        return
    end subroutine pix2xy_nest

    subroutine mk_pix2xy()
        !=======================================================================
        !     constructs the array giving x and y in the face from pixel number
        !     for the nested (quad-cube like) ordering of pixels
        !
        !     the bits corresponding to x and y are interleaved in the pixel number
        !     one breaks up the pixel number by even and odd bits
        !=======================================================================
        integer*4 ::  kpix, jpix, ix, iy, ip, id

        !cc cf block data      data      pix2x(1023) /0/
        !-----------------------------------------------------------------------
        !      print *, 'initiate pix2xy'
        do kpix=0,1023          ! pixel number
           jpix = kpix
           IX = 0
           IY = 0
           IP = 1               ! bit position (in x and y)
        !        do while (jpix/=0) ! go through all the bits
           do
              if (jpix == 0) exit ! go through all the bits
              ID = MODULO(jpix,2)  ! bit value (in kpix), goes in ix
              jpix = jpix/2
              IX = ID*IP+IX

              ID = MODULO(jpix,2)  ! bit value (in kpix), goes in iy
              jpix = jpix/2
              IY = ID*IP+IY

              IP = 2*IP         ! next bit (in x and y)
           enddo
           pix2x(kpix) = IX     ! in 0,31
           pix2y(kpix) = IY     ! in 0,31
        enddo

        return
    end subroutine mk_pix2xy


    subroutine mk_xy2pix()
        !=======================================================================
        !     sets the array giving the number of the pixel lying in (x,y)
        !     x and y are in {1,128}
        !     the pixel number is in {0,128**2-1}
        !
        !     if  i-1 = sum_p=0  b_p * 2^p
        !     then ix = sum_p=0  b_p * 4^p
        !          iy = 2*ix
        !     ix + iy in {0, 128**2 -1}
        !=======================================================================

        integer*4 :: k,ip,i,j,id

        do i = 1,128           !for converting x,y into
           j  = i-1            !pixel numbers
           k  = 0
           ip = 1

           do
              if (j==0) then
                 x2pix(i) = k
                 y2pix(i) = 2*k
                 exit
              else
                 id = MODULO(J,2)
                 j  = j/2
                 k  = ip*id+k
                 ip = ip*4
              endif
           enddo

        enddo

        RETURN
    END subroutine mk_xy2pix

    subroutine radec_degrees_to_thetaphi_radians(ra, dec, theta, phi)
        ! all 4 inputs in radians
        real*8, intent(in) :: ra, dec
        real*8, intent(out) :: theta, phi
        phi = ra*DEG2RAD
        theta = dec*DEG2RAD + HALFPI
    end subroutine radec_degrees_to_thetaphi_radians

    subroutine eq2vec(ra, dec, vector)
        !=======================================================================
        !     renders the vector (x,y,z) corresponding to angles
        !     theta (co-latitude measured from North pole, in [0,Pi] radians)
        !     and phi (longitude measured eastward, in radians)
        !     North pole is (x,y,z)=(0,0,1)
        !     added by EH, Feb 2000
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



