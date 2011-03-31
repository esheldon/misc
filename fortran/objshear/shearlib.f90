! vim:set ft=fortran:
module shearlib

    use srclib
    use lenslib
    use configlib
    use cosmolib
    use healpix, only : HALFPI, DEG2RAD, query_disc
    use fileutil
    use arrlib

    implicit none

    ! (shape noise)**2 in ellipticity
    real*8, private, parameter :: SN2  = 0.1024
    ! (shape noise/2)**2  to give noise on gamma
    real*8, private, parameter :: GSN2 = 0.0256

    real*8, private, parameter :: minz = 0.05


    ! our beginning maxpix.
    ! for desmocks 2.13 and zmin about 0.01 and rmax 2.8, we needed
    !   20000 for nside=1024
    !   100000 for 2048
    !   300000 for 4096
    !       1000000  at 5Mpc
    !   doing 5000000 at 10Mpc

    integer*8, private, parameter :: MAXPIX=5000000
    integer*8, private, parameter :: inclusive=1

    !integer*8, private :: maxpix_used = 0

    type lens_sum

        integer*8 zindex
        real*8 weight

        integer*8, dimension(:), allocatable :: npair
        real*8,    dimension(:), allocatable :: wsum
        real*8,    dimension(:), allocatable :: dsum
        real*8,    dimension(:), allocatable :: osum
        real*8,    dimension(:), allocatable :: rsum

    end type lens_sum


    type sheardata 

        type(config) pars

        type(source_cat) scat 
        type(lens), dimension(:), allocatable :: lenses


        ! healpix info
        integer*8 minid
        integer*8 maxid
        integer*8, allocatable, dimension(:) :: rev

        ! use some extra memory for speed.
        real*8, dimension(:), allocatable :: sinsdec
        real*8, dimension(:), allocatable :: sinsra
        real*8, dimension(:), allocatable :: cossdec
        real*8, dimension(:), allocatable :: cossra

        real*8, dimension(:), allocatable :: sinldec
        real*8, dimension(:), allocatable :: sinlra
        real*8, dimension(:), allocatable :: cosldec
        real*8, dimension(:), allocatable :: coslra

    end type sheardata 

contains


    subroutine load_shear_data(config_file, shdata)

        character(len=*), intent(in) :: config_file
        type(sheardata), intent(inout) ::  shdata
        integer*8 i
        integer*8 nsource

        !call read_config(config_file, shdata%pars)
        call read_config(config_file, shdata%pars)
        call print_config(shdata % pars)

        call cosmo_init(shdata%pars%H0, shdata%pars%omega_m, shdata%pars%npts)

        call read_lens_cat(shdata%pars%lens_file, shdata%lenses)
        call add_lens_dc(shdata%lenses)
        call print_lens_firstlast(shdata%lenses)

        call read_source_cat(shdata%pars%source_file, shdata%scat)

        nsource = shdata%scat%nel

        if (shdata%scat%sigmacrit_style == 1) then
            call add_source_dc(shdata%scat)
        endif
        call add_source_hpixid(shdata%pars%nside, shdata%scat)
        call print_source_firstlast(shdata%scat)

        print '(a)',"Calculating src sin/cosc"
        allocate(shdata%sinsdec(nsource))
        allocate(shdata%cossdec(nsource))
        allocate(shdata%sinsra(nsource))
        allocate(shdata%cossra(nsource))

        do i=1,nsource
            shdata%sinsra(i)  = sin( shdata %scat%ra(i)*DEG2RAD )
            shdata%sinsdec(i) = sin( shdata %scat%dec(i)*DEG2RAD )
            shdata%cossra(i)  = cos( shdata %scat%ra(i)*DEG2RAD )
            shdata%cossdec(i) = cos( shdata %scat%dec(i)*DEG2RAD )
        end do

        allocate(shdata%sinldec(size(shdata%lenses)))
        allocate(shdata%sinlra(size(shdata%lenses)))
        allocate(shdata%cosldec(size(shdata%lenses)))
        allocate(shdata%coslra(size(shdata%lenses)))
        do i=1,size(shdata%lenses)
            shdata%sinlra(i)  = sin( shdata %lenses(i)%ra*DEG2RAD )
            shdata%sinldec(i) = sin( shdata %lenses(i)%dec*DEG2RAD )
            shdata%coslra(i)  = cos( shdata %lenses(i)%ra*DEG2RAD )
            shdata%cosldec(i) = cos( shdata %lenses(i)%dec*DEG2RAD )
        end do

        call get_hpix_rev(shdata)
        call print_hpix_info(shdata)

    end subroutine load_shear_data


    subroutine calc_shear(shdata, lensums)
        type(sheardata), intent(in) :: shdata
        type(lens_sum), dimension(:), allocatable, intent(inout) :: lensums
        integer*8 i, nprint
        integer*8, allocatable, dimension(:) :: listpix

        type(lens_sum) lensum_tot
        integer*8 :: nlens, nbin

        allocate(listpix(MAXPIX)); listpix=0

        nlens = size(shdata%lenses)
        nbin  = shdata%pars%nbin
        nprint = 10

        call init_lens_sums(lensums, nlens, nbin)

        ! make sure these are called first, for thread safety
        print '(a,i0,a)',"Processing ",nlens," lenses."

        do i=1,nlens
            print '(".",$)'

            lensums(i)%zindex = shdata%lenses(i)%zindex

            if (shdata % lenses(i) % z > minz) then

                call process_lens_omp(shdata, i, lensums(i),listpix)

            endif

        end do

        print *
        print '(a)','Done lens loop'
        deallocate(listpix)


        call add_lens_sums(lensums, lensum_tot)
        call print_shear_sums(lensum_tot)

    end subroutine calc_shear


    subroutine process_lens_omp(shdata, ilens,  lensum, listpix)
        use interplib

        type(sheardata), intent(in) ::  shdata
        integer*8, intent(in) :: ilens
        type(lens_sum), intent(inout) :: lensum

        integer*8, dimension(:), allocatable, intent(inout) :: listpix

        real*8 weight
        real*8, allocatable, dimension(:) :: wsum
        real*8, allocatable, dimension(:) :: dsum
        real*8, allocatable, dimension(:) :: osum
        real*8, allocatable, dimension(:) :: rsum
        integer*8, allocatable, dimension(:) :: npair

        integer*8 k, isrc, n_in_bin
        integer*8 j, npixfound, pix
        real*8 zl, dl, dlc
        real*8 search_angle, cos_search_angle, theta, scinv
        real*8 phi, r, cos2theta, sin2theta
        integer*8 nbin

        nbin=size(lensum%npair)
        weight=0
        allocate(wsum(nbin)); wsum=0
        allocate(dsum(nbin)); dsum=0
        allocate(osum(nbin)); osum=0
        allocate(rsum(nbin)); rsum=0
        allocate(npair(nbin)); npair=0

        zl = shdata%lenses(ilens)%z
        dlc = shdata%lenses(ilens)%dc
        dl = dlc/(1+zl)

        search_angle = shdata%pars%rmax/dl
        cos_search_angle = cos(search_angle)

        call query_disc(shdata%pars%nside,    &
                        shdata%lenses(ilens)%ra, &
                        shdata%lenses(ilens)%dec, &
                        search_angle, listpix, npixfound,   &
                        inclusive)

!$OMP PARALLEL DO DEFAULT(SHARED) &
!$OMP PRIVATE(j,pix,n_in_bin,k,isrc,phi,cos2theta,sin2theta,r,scinv) &
!$OMP REDUCTION(+:weight,wsum,dsum,osum,rsum,npair)
        do j=1,npixfound
            pix = listpix(j)
            if (pix >= shdata%minid .and. pix <= shdata%maxid) then
                pix = listpix(j) - shdata%minid + 1
                n_in_bin = shdata%rev(pix+1) - shdata%rev(pix)
                do k=1,n_in_bin
                    isrc = shdata%rev( shdata%rev(pix) + k -1 )

                    call get_pair_info(shdata%coslra(ilens),  &
                                       shdata%sinlra(ilens),  &
                                       shdata%cosldec(ilens), &
                                       shdata%sinldec(ilens), &
                                       shdata%sinsdec(isrc),  &
                                       shdata%cossdec(isrc),  &
                                       shdata%sinsra(isrc),   &
                                       shdata%cossra(isrc),   &
                                       cos_search_angle,      &
                                       phi, cos2theta, sin2theta)

                    if (phi > 0 ) then
                        r = phi*dl
                        if (shdata%scat%sigmacrit_style == 1) then
                            scinv = sigmacritinv(zl,dlc, shdata%scat%dc(isrc))
                        else
                            if ( (zl >= shdata%scat%zlmin) .and. (zl <= shdata%scat%zlmax) ) then

                                    scinv = interpf8(shdata%scat%zlinterp, &
                                                     shdata%scat%scinv(isrc,:), &
                                                     zl)
                            else
                                scinv=0
                            endif
                        endif
                        if (scinv > 0) then
                            call calc_shear_sums_omp(shdata%pars, &
                                shdata%scat%g1(isrc), &
                                shdata%scat%g2(isrc), &
                                shdata%scat%err(isrc), &
                                r,cos2theta,sin2theta,scinv, &
                                weight,wsum,dsum,osum,rsum,npair)
                        end if
                    end if

                end do ! sources in pixel
            end if ! pixel is in source pixel list
        end do
!$OMP END PARALLEL DO

        lensum%weight = weight
        lensum%wsum = wsum
        lensum%dsum = dsum
        lensum%osum = osum
        lensum%rsum = rsum
        lensum%npair = npair

        deallocate(wsum)
        deallocate(dsum)
        deallocate(osum)
        deallocate(rsum)
        deallocate(npair)
        
    end subroutine process_lens_omp

    subroutine process_lens(shdata, ilens, lensum)
        type(sheardata), intent(in) ::  shdata
        integer*8, intent(in) :: ilens
        type(lens_sum), intent(inout) :: lensum
        integer*8, allocatable, dimension(:) :: listpix

        integer*8 k, isrc, n_in_bin
        integer*8 j, pix, npixfound
        real*8 zl, dl, dlc
        real*8 search_angle, cos_search_angle, theta, scinv
        real*8 phi, r, cos2theta, sin2theta

        zl = shdata%lenses(ilens)%z
        dlc = shdata%lenses(ilens)%dc
        dl = dlc/(1+zl)

        search_angle = shdata%pars%rmax/dl
        cos_search_angle = cos(search_angle)

        !print '(a)',"Before query disc"
        call query_disc(shdata%pars%nside,    &
                        shdata%lenses(ilens)%ra, &
                        shdata%lenses(ilens)%dec, &
                        search_angle, listpix, npixfound,   &
                        inclusive)
        !print '(a)',"After query disc"

        !if (npixfound > maxpix_used) maxpix_used = npixfound
        do j=1,npixfound
        !do j=0,npixfound-1
            pix = listpix(j)
            if (pix >= shdata%minid .and. pix <= shdata%maxid) then
                ! add one to make 1-offset
                pix = listpix(j) - shdata%minid + 1
                n_in_bin = shdata%rev(pix+1) - shdata%rev(pix)
                do k=1,n_in_bin
                    isrc = shdata%rev( shdata%rev(pix) + k -1 )

                    call get_pair_info(shdata%coslra(ilens),  &
                                       shdata%sinlra(ilens),  &
                                       shdata%cosldec(ilens), &
                                       shdata%sinldec(ilens), &
                                       shdata%sinsdec(isrc),  &
                                       shdata%cossdec(isrc),  &
                                       shdata%sinsra(isrc),   &
                                       shdata%cossra(isrc),   &
                                       cos_search_angle,      &
                                       phi, cos2theta, sin2theta)

                    if (phi > 0 ) then
                        r = phi*dl
                        scinv = sigmacritinv(zl,dlc, shdata%scat%dc(isrc))
                        if (scinv > 0) then
                            call calc_shear_sums(shdata%pars, &
                                shdata%scat%g1(isrc), &
                                shdata%scat%g2(isrc), &
                                shdata%scat%err(isrc), &
                                r,cos2theta,sin2theta,scinv,lensum)
                        end if
                    end if

                end do ! sources in pixel
            end if ! pixel is in source pixel list
        end do
        
    end subroutine process_lens



    subroutine get_pair_info(coslra,sinlra,cosldec,sinldec, &
                             sinsdec,cossdec,sinsra,cossra, &
                             cos_search_angle,              &
                             phi, cos2theta, sin2theta)
        ! get the great circle distance between the points, and
        ! the cos(2*theta) rotation angle
        real*8, intent(in) :: coslra, sinlra, cosldec, sinldec 
        real*8, intent(in) :: sinsdec, cossdec, sinsra, cossra, cos_search_angle
        real*8, intent(out) :: phi, cos2theta, sin2theta

        real*8 cosradiff, sinradiff, cosphi
        real*8 theta, arg

        ! cos(A+B) = cos A cos B - sin A sin B
        ! sin(A+B) = sin A cos B + cos A sin B

        ! cos(A-B) = cos A cos B + sin A sin B
        ! sin(A-B) = sin A cos B - cos A sin B

        cosradiff = cossra*coslra + sinsra*sinlra

        cosphi = sinldec*sinsdec + cosldec*cossdec*cosradiff
        if (cosphi > cos_search_angle) then

            if (cosphi > 1.0) cosphi = 1.0
            if (cosphi < -1.0) cosphi = -1.0
            phi = acos(cosphi)

            sinradiff = sinsra*coslra - cossra*sinlra

            arg = sinldec*cosradiff - cosldec*sinsdec/cossdec
            theta = atan2(sinradiff, arg) - HALFPI

            cos2theta = cos(2*theta)
            sin2theta = sin(2*theta)
            ! we can replace with
            ! sin2theta = sign(sqrt(1.0-cos2theta**2), theta)
            ! will make code 30% faster

        else
            phi=-1
        end if
        
    end subroutine get_pair_info


    subroutine calc_shear_sums_omp(pars, g1, g2, err, &
                                   r, cos2theta, sin2theta, scinv, &
                                   weight, wsum, dsum, osum, rsum, npair)
                                   
        type(config), intent(in) :: pars
        real*8, intent(in) :: g1
        real*8, intent(in) :: g2
        real*8, intent(in) :: err
        real*8, intent(in) :: r, cos2theta, sin2theta
        real*8, intent(in) :: scinv

        real*8, intent(inout) :: weight
        real*8, dimension(:), intent(inout) :: wsum
        real*8, dimension(:), intent(inout) :: dsum
        real*8, dimension(:), intent(inout) :: osum
        real*8, dimension(:), intent(inout) :: rsum
        integer*8, dimension(:), intent(inout) :: npair

        real*8 scinv2, gamma1, gamma2, w
        real*8 logr
        integer*8 rbin

        logr = log10(r)
        rbin = int( (logr-pars%log_rmin)/pars%log_binsize ) + 1

        if (rbin >= 1 .and. rbin <= pars%nbin) then

            scinv2 = scinv*scinv

            gamma1 = -(g1*cos2theta + g2*sin2theta)
            gamma2 =  (g1*sin2theta - g2*cos2theta)
            w = scinv2/(GSN2 + err**2)

            weight = weight + w

            wsum(rbin)  = wsum(rbin) + w
            dsum(rbin)  = dsum(rbin) + w*gamma1/scinv
            osum(rbin)  = osum(rbin) + w*gamma2/scinv

            npair(rbin) = npair(rbin) + 1
            rsum(rbin)  = rsum(rbin) + r

        end if ! valid radial bin


    end subroutine calc_shear_sums_omp



    subroutine calc_shear_sums(pars, g1, g2, err, r, cos2theta, sin2theta, scinv, lensum)
        type(config), intent(in) :: pars
        real*8, intent(in) :: g1
        real*8, intent(in) :: g2
        real*8, intent(in) :: err
        real*8, intent(in) :: r, cos2theta, sin2theta
        real*8, intent(in) :: scinv
        type(lens_sum), intent(inout) :: lensum

        real*8 scinv2, gamma1, gamma2, weight
        real*8 logr
        integer*8 rbin

        logr = log10(r)
        rbin = int( (logr-pars%log_rmin)/pars%log_binsize ) + 1

        if (rbin >= 1 .and. rbin <= pars%nbin) then

            scinv2 = scinv*scinv

            gamma1 = -(g1*cos2theta + g2*sin2theta)
            gamma2 =  (g1*sin2theta - g2*cos2theta)
            weight = scinv2/(GSN2 + err**2)

            lensum%weight = lensum%weight + weight

            lensum%wsum(rbin)  = lensum%wsum(rbin) + weight
            lensum%dsum(rbin)  = lensum%dsum(rbin) + weight*gamma1/scinv
            lensum%osum(rbin)  = lensum%osum(rbin) + weight*gamma2/scinv

            lensum%npair(rbin) = lensum%npair(rbin) + 1
            lensum%rsum(rbin)  = lensum%rsum(rbin) + r

        end if ! valid radial bin


    end subroutine calc_shear_sums


    subroutine write_lens_sums(filename, lensums)

        character(len=*), intent(in) :: filename
        type(lens_sum), dimension(:), intent(in) :: lensums
        integer*8 i
        integer*8 nlens, nbin, lun

        lun = get_lun()

        nlens = size(lensums)
        nbin  = size(lensums(1)%npair)
        print '(a,a)',"Writing output file: ",trim(filename)
        open(unit=lun, file=filename, access='STREAM', status='replace')
        write (lun)nlens
        write (lun)nbin
 
        do i=1,size(lensums)
            call write_lens_sum(lun, lensums(i))
        end do

        close(lun)

    end subroutine write_lens_sums

    subroutine write_lens_sum(lun, lensum)
        integer*8, intent(in) :: lun
        type(lens_sum), intent(in) :: lensum


        write(lun)lensum%zindex
        write(lun)lensum%weight

        write(lun)lensum%npair
        write(lun)lensum%rsum
        write(lun)lensum%wsum
        write(lun)lensum%dsum
        write(lun)lensum%osum
    end subroutine write_lens_sum


    subroutine print_shear_sums(lensum)
        type(lens_sum), intent(in) :: lensum

        real*8, dimension(:), allocatable :: rbins
        real*8, dimension(:), allocatable :: dsig
        real*8, dimension(:), allocatable :: osig
        integer*8 i, nbin
        integer*8 :: npair = 0
        real*8 :: wsum=0, dsum=0, osum=0

        nbin = size(lensum%rsum)

        allocate(rbins(nbin)); rbins=0
        allocate(dsig(nbin)); dsig=0
        allocate(osig(nbin)); osig=0

        rbins = lensum % rsum/lensum % npair
        dsig  = lensum % dsum/lensum % wsum
        osig  = lensum % osum/lensum % wsum

        print '(8a14)',"npair","rsum","meanr","wsum","dsum","osum","dsig","osig"
        do i=1,nbin
            print '(i15,E14.7,6F14.7)',&
                    lensum%npair(i), &
                    lensum%rsum(i), rbins(i), &
                    lensum%wsum(i), &
                    lensum%dsum(i), &
                    lensum%osum(i), &
                    dsig(i),osig(i)
        end do
        print '("total pairs: ",i0)',sum(lensum%npair)
        print '("mean radius: ",F14.7)',sum(lensum%rsum)/sum(lensum%npair)
        print '("mean dsig:   ",F14.7)',sum(lensum%dsum)/sum(lensum%wsum)
        print '("mean osig:   ",F14.7)',sum(lensum%osum)/sum(lensum%wsum)

        deallocate(rbins)
        deallocate(dsig)
        deallocate(osig)
 
    end subroutine print_shear_sums

    subroutine init_lens_sums(lensums, nlens, nbin)
        type(lens_sum), dimension(:), allocatable, intent(inout) :: lensums
        integer*8, intent(in) :: nlens, nbin

        integer*8 i

        allocate(lensums(nlens))
        do i=1,nlens
            call init_lens_sum(lensums(i), nbin)
        end do
    end subroutine init_lens_sums

    subroutine init_lens_sum(lensum, nbin)

        type(lens_sum), intent(inout) :: lensum
        integer*8 nbin

        allocate(lensum%npair(nbin))
        allocate(lensum%wsum(nbin))
        allocate(lensum%dsum(nbin))
        allocate(lensum%osum(nbin))
        allocate(lensum%rsum(nbin))

        call reset_lens_sum(lensum)
    end subroutine init_lens_sum


    subroutine reset_lens_sum(lensum)

        type(lens_sum), intent(inout) :: lensum

        integer*8 i

        lensum % zindex = 0
        lensum % weight = 0
        do i=1,size(lensum%npair)
            lensum % npair(i) = 0
            lensum % wsum(i) = 0
            lensum % dsum(i) = 0
            lensum % osum(i) = 0
            lensum % rsum(i) = 0
        end do

    end subroutine reset_lens_sum

    subroutine add_lens_sums(lsrc, ldest)

        type(lens_sum), dimension(:), intent(in) :: lsrc
        type(lens_sum), intent(inout) :: ldest

        integer*8 i, j, nbin

        nbin = size(lsrc(1) % npair)
        call init_lens_sum(ldest, nbin)

        do j=1,size(lsrc)
            do i=1,size(ldest%npair)
                ldest %weight = ldest %weight + lsrc(j) %wsum(i)

                ldest %npair(i) = ldest %npair(i) + lsrc(j) %npair(i)
                ldest %wsum(i) = ldest %wsum(i) + lsrc(j) %wsum(i)
                ldest %dsum(i) = ldest %dsum(i) + lsrc(j) %dsum(i)
                ldest %osum(i) = ldest %osum(i) + lsrc(j) %osum(i)
                ldest %rsum(i) = ldest %rsum(i) + lsrc(j) %rsum(i)
            end do
        end do

    end subroutine add_lens_sums



    subroutine get_hpix_rev(shdata)
        use sortlib
        use histogram

        type(sheardata) shdata

        integer*8, allocatable, dimension(:) :: sort_ind
        integer*8, allocatable, dimension(:) :: h
        integer*8 :: binsize = 1
        integer*8 nsource
        nsource = shdata%scat%nel

        print '(a)',"Getting healpix sort index"
        call qsorti8(shdata%scat%hpixid, sort_ind)

        shdata%minid = shdata%scat%hpixid(sort_ind(1))
        shdata%maxid = shdata%scat%hpixid(sort_ind(nsource))

        print '(a)',"Getting healpix revind"
        call histi8(shdata%scat%hpixid, sort_ind, binsize, &
                    h, shdata%rev)

    end subroutine get_hpix_rev


    subroutine print_hpix_info(shdata)
        type(sheardata) shdata

        write(*,'(a)')"healpix info:"
        write(*,'("    nside:   ",i0)')shdata%pars%nside
        write(*,'("    minid:   ",i0)')shdata%minid
        write(*,'("    maxid:   ",i0)')shdata%maxid
        write(*,'("    rev(1):  ",i0)')shdata%rev(1)
        write(*,'("    rev(-1): ",i0)')shdata%rev(size(shdata%rev))

    end subroutine print_hpix_info



end module shearlib
