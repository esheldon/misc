! vim:set ft=fortran:


program main
    use healpix

    integer*8 nside, n_pix, ipix, i, ira, idec, ringnum
    real*8 area
    real*8 ra(10), dec(13)
    real*8 ra1, dec1, z(4)
    real*8 vector(3)
    real*8 radius
    integer*8 nfound
    integer*8, dimension(:), allocatable :: listpix
    integer lun
    character(255) filename
    
    nside = 4096
    n_pix = npix(nside)
    area = pixarea(nside)
    
    print '("nside: ",i0)',nside
    print '("  npix: ",i0)',n_pix
    print '("  area: ",e)',area*RAD2DEG**2

    ra1=175.0_8
    dec1=27.2_8
    call eq2vec(ra1, dec1, vector)
    print '("convert ",F15.8, F15.8," to vector (",F15.8, F15.8, F15.8,")")',&
        ra1, dec1, vector(1), vector(2), vector(3)

    z=(/-0.75, -0.2, 0.2, 0.75/)
    do i=1,4
        ringnum = ring_num(nside, z(i))
        print '("ring num at z=",F15.8,": ",i0)',z(i),ringnum
    enddo

    ra = (/ 0.,   40.,   80.,  120.,  160.,  200.,  240.,  280.,  320.,  360./)
    dec = (/-90., -85., -65., -45., -25., -5., 0., 5., 25., 45., 65., 85., 90./)

    print '("testing eq2pix")'
    do ira=1,size(ra)
        do idec=1,size(dec)
            call eq2pix(nside, ra(ira), dec(idec), ipix)
            print '(F15.8," ",F15.8," ",i0)',ra(ira),dec(idec),ipix
        end do
    end do

    allocate(listpix(4*nside))
    radius = 40.0_8/3600.0_8*PI/180.0_8

    filename = "test/test-healpix.dat"
    lun=50
    open(unit=lun,file=filename)
    print '("intersect disc")'

    do ira=1,size(ra)
        do idec=1,size(dec)
            call query_disc(nside, ra(ira), dec(idec), radius, listpix, nfound, 1_8)
            print '("  ra: ",F10.6,"  dec: ",F10.6, " pixels:",i0)',ra(ira),dec(idec),nfound

            write (lun,'(F10.6," ",F10.6," ",i0," ",$)'),ra(ira),dec(idec),nfound
            do i=1,nfound
                write (lun,'(i0," ",$)'), listpix(i)
            enddo
            write (lun,'("")')
        end do
    end do

    print '("Closing file: ",a)',filename


end program
