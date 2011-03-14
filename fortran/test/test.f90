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

end module interpolate

program test
  use interpolate
  real*8, dimension(:), allocatable :: x
  real*8, dimension(:), allocatable :: y
  integer*4 i,n
  real*8 xmin,xmax,xstep

  n=1000
  xmin=0.
  xmax=10.
  xstep=(xmax-xmin)/n

  allocate(x(n))
  allocate(y(n))
  do i=1,n
    x(i) = (i-1)*xstep
    y(i) = x(i)**2
  enddo

  print *, 0.9, interpf8(x,y,0.9_8)
  print *, 1.0, interpf8(x,y,1.0_8)
  print *, 1.5, interpf8(x,y,1.5_8)
  print *, 8.7, interpf8(x,y,8.7_8)
  print *, 10.0, interpf8(x,y,10.0_8)
  print *, 10.3, interpf8(x,y,10.3_8)
end program test


