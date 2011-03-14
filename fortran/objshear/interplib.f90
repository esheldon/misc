module interplib
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

  real*4 function interpf4(x, y, u) result(val)
    real*4, dimension(:), intent(in) :: x
    real*4, dimension(:), intent(in) :: y
    real*4, intent(in) :: u

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

  end function interpf4



end module interplib

