#if !defined (_lens_constants_h)
#define _lens_constants_h

// Some constants

#define D2R   0.017453292519943295
#define R2D  57.295779513082323
#define TWOPI 6.28318530717958647693
#define THREE_M_PI_2 4.7123889803846897

#define SHAPENOISE 0.32
#define SHAPENOISE2 0.1024

#define FLAGS_MASKED       0x1   /* central point is masked */
#define FLAGS_QUAD1_MASKED 0x2   /* Quadrant masked */
#define FLAGS_QUAD2_MASKED 0x4
#define FLAGS_QUAD3_MASKED 0x8
#define FLAGS_QUAD4_MASKED 0x10

#define FLAGS_QUAD1_MASKED_MONTE 0x20  /* Quadrant masked, monte carlo */
#define FLAGS_QUAD2_MASKED_MONTE 0x40  /* note quad1 masked will be set */
#define FLAGS_QUAD3_MASKED_MONTE 0x80  /* if the monte is set */
#define FLAGS_QUAD4_MASKED_MONTE 0x100

// Minimum angle in radians (10 arcseconds)
//#define MINIMUM_ANGLE 4.8481368e-05
// 20 arcsec
//#define MINIMUM_ANGLE 9.6962736e-05
#define MINIMUM_ANGLE 0.0

#define SOURCE_HEADER_LINES 20
#define LENS_HEADER_LINES 16

#endif /* _lens_constants_h */
