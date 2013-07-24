#ifndef _JACOBIAN_HEADER_GUARD
#define _JACOBIAN_HEADER_GUARD

struct jacobian {
    double row0;  // center of coord system in pixel space
    double col0;
    double dudrow;
    double dudcol;
    double dvdrow;
    double dvdcol;
};


// row col here are relative to the "center"
#define JACOB_PIX2U(jacob, row, col)             \
    (  (jacob)->dudrow*(row - (jacob)->row0)     \
     + (jacob)->dudcol*(col - (jacob)->col0) )

#define JACOB_PIX2V(jacob, row, col)             \
    (  (jacob)->dvdrow*(row - (jacob)->row0)     \
     + (jacob)->dvdcol*(col - (jacob)->col0) )

struct jacobian *jacobian_new(double row0,
                              double col0,
                              double dudrow,
                              double dudcol,
                              double dvdrow,
                              double dvdcol);

void jacobian_set(struct jacobian *self, 
                  double row0,
                  double col0,
                  double dudrow,
                  double dudcol,
                  double dvdrow,
                  double dvdcol);

void jacobian_set_identity(struct jacobian *self);

void jacobian_set_cen(struct jacobian *self, double row, double col);

void jacobian_print(const struct jacobian *self, FILE *stream);
#endif
