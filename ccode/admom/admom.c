#include <stdio.h>
#include <stlib.h>
#include "admom.h"
#include "image.h"


/* 
   the am struct should contain on entry:

   ->row,col should be the starting center guess
   ->irr,irc,icc should be the starting moment guess
   ->nsub tells about sub-pixel integration

   On output the parameters of the gaussian are updated and flags are set.

*/
void admom(struct image *image,
           double skysig,
           struct am *am)
{

}


