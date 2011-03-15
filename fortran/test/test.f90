! vim: set filetype=fortran et ts=2 sw=2 sts=2 :

module interpolate
contains

  real*8 function interpf8(x, y, u) result(val)
    real*8, dimension(:), intent(in) :: x
    real*8, dimension(:), intent(in) :: y
    real*8, intent(in) :: u

    integer*4 ilo,ihi
    integer*4 i

    ilo=size(x)-1
    ihi=size(x)
    do i=1,size(x)
      if ( x(i) >= u ) then 
        if (i /= 1) then
          ilo = i-1
          ihi = i
        else
          ilo = 1
          ihi = 2
        endif
        exit
      endif
    enddo

    val = ( u-x(ilo) )*( y(ihi) - y(ilo) )/( x(ihi) - x(ilo) ) + y(ilo)

    return

  end function interpf8

  real*8 function tmp(i, j) result(val)
    integer*8 i,j
    val = i*j**2 + 35 + exp(float(i))
  end function tmp

end module interpolate



program test
  use interpolate
  use omp_lib
  real*8 x(25), y(25)
  real*8 r

  integer*8 i,j,k,l
  integer*8 n
  real*8 xx,yy,zz

  !n=100000000000_8
  n=1000000000_8

  do i=1,25
    x(i) = i
    y(i) = x(i)**2
  enddo

  yy = 0
!!!!$OMP PARALLEL DO DEFAULT(SHARED) PRIVATE(i,r) REDUCTION (+:yy)
!$OMP PARALLEL DO REDUCTION(+:yy)
  do i=1,n
    !j = j**2 + j + 25
    !j = mod(i,25)
    
    xx = mod(i,25)
    yy = yy + interpf8(x,y,xx)
    !call random_number(r)
    !!rint *,r,i
    !yy = yy + exp(r) + r

    !do k=1,n
    !  zz = exp(-xx + k**2) + cos(yy + k) + sin(float(j*k + i))
    !enddo

  enddo
!$OMP END PARALLEL DO
!!!$OMP END PARALLEL DO

  print *,yy
end program test


