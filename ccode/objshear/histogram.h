#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include <stdint.h>
#include "Vector.h"

// special simplified binsize=1, integer histogrammer
// The simplifications from binsize=1 and integer are significant
void i64hist1(struct i64vector* vec,
              struct szvector* sort_index,
              struct i64vector* h,   // output.  Should not be allocated on entry
              struct i64vector* rev); // output



#endif
