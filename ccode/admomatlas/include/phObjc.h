#if !defined(PHOBJC_H)
#define PHOBJC_H
#include "dervish.h"
#include "phExtract.h"
#include "phSpanUtil.h"
#include "phFramestat.h"
#include "phPeaks.h"
#include "phConsts.h"

/*
 * An enum for types of object
 */
typedef enum {
   OBJ_UNK = 0, 
   OBJ_CR, 
   OBJ_DEFECT, 
   OBJ_GALAXY, 
   OBJ_GHOST, 
   OBJ_KNOWNOBJ,
   OBJ_STAR, 
   OBJ_TRAIL,
   OBJ_SKY,
   OBJ_NTYPE
} OBJ_TYPE;

/* here are the values the "flags" field can take, in bitwise fashion */
#define OBJECT1_CANONICAL_CENTER 0x1	/* used canonical, not local, centre
				   pragma typedef { */
#define OBJECT1_BRIGHT 0x2	/* detected by Bright Objects */
#define OBJECT1_EDGE 0x4	/* object is too close to edge of frame */
#define OBJECT1_BLENDED 0x8	/* object is/was blended */
#define OBJECT1_CHILD  0x10	/* object is a child */
#define OBJECT1_PEAKCENTER 0x20	/* given centre is position of peak pixel */
#define OBJECT1_NODEBLEND 0x40	/* no deblending attempted */
#define OBJECT1_NOPROFILE 0x80	/* too small to estimate a profile */
#define OBJECT1_NOPETRO 0x100	/* no Petrosian radius */
#define OBJECT1_MANYPETRO 0x200	/* more than one Petrosian radius */
#define OBJECT1_NOPETRO_BIG 0x400 /* no Petrosian radius as object is too big*/
#define OBJECT1_DEBLEND_TOO_MANY_PEAKS 0x800 /* too many peaks to deblend */
#define OBJECT1_CR 0x1000	/* contains a CR pixel */
#define OBJECT1_MANYR50 0x2000	/* more than one 50% radius */
#define OBJECT1_MANYR90 0x4000	/* more than one 90% radius */
#define OBJECT1_BAD_RADIAL 0x8000 /* some low S/N radial points */
#define OBJECT1_INCOMPLETE_PROFILE 0x10000 /* r_P includes off-frame pixels */
#define OBJECT1_INTERP 0x20000	/* object contains interpolated pixels */
#define OBJECT1_SATUR 0x40000	/* object contains saturated pixels */
#define OBJECT1_NOTCHECKED 0x80000 /* object contains NOTCHECKED pixels */
#define OBJECT1_SUBTRACTED 0x100000 /* object had wings subtracted */
#define OBJECT1_NOSTOKES 0x200000 /* object has no measured stokes params */
#define OBJECT1_BADSKY 0x400000  /* sky level is so bad that object is -ve */
#define OBJECT1_PETROFAINT 0x800000	/* >= 1 Petrosian radius too faint */
#define OBJECT1_TOO_LARGE 0x1000000	/* object is too large */
#define OBJECT1_DEBLENDED_AS_PSF 0x2000000 /* deblender treated obj as PSF */
#define OBJECT1_DEBLEND_PRUNED 0x4000000 /* deblender pruned peak list */
#define OBJECT1_ELLIPFAINT 0x8000000	/* Centre's fainter than desired
					   elliptical isophote */
#define OBJECT1_BINNED1 0x10000000 /* object was found in 1x1 binned image */
#define OBJECT1_BINNED2 0x20000000 /* object was found in 2x2 binned image */
#define OBJECT1_BINNED4 0x40000000 /* object was found in 4x4 binned image */
#define OBJECT1_MOVED 0x80000000 /* Object may have moved; largest flag value
				     pragma } OBJECT1_FLAGS */

#define OBJECT1_DETECTED (OBJECT1_BINNED1|OBJECT1_BINNED2|OBJECT1_BINNED4)
/*
 * here are values for flags2; only (flags2 & OBJECT2_SAVE_MASK) are saved
 * at the end of the run
 */
#define OBJECT2_DEBLENDED_AS_MOVING 0x1	/* deblended as a moving object
					   pragma typedef { */
#define OBJECT2_NODEBLEND_MOVING 0x2	/* no deblend of moving object */
#define OBJECT2_TOO_FEW_DETECTIONS 0x4  /* too few detections to deblend */
#define OBJECT2_BAD_MOVING_FIT 0x8	/* fit to moving object was too poor */
#define OBJECT2_STATIONARY 0x10		/* velocity is consistent with zero */
#define OBJECT2_PEAKS_TOO_CLOSE 0x20	/* at least some peaks were too
					   close, and thus merged */
#define OBJECT2_MEDIAN_CENTRE 0x40	/* centre is of median-smoothed image*/
#define OBJECT2_LOCAL_EDGE 0x80		/* per-band centre's too near edge */

#define OBJECT2_AMOMENT_FAINT 0x200000
#define OBJECT2_AMOMENT_SHIFT 0x400000
#define OBJECT2_AMOMENT_MAXITER 0x800000

#define OBJECT2_MEASURED 0x10000000	/* object has been measured */
#define OBJECT2_GROWN_MERGED 0x20000000	/* growing led to a merger */
#define OBJECT2_HAS_CENTER 0x40000000	/* OBJC has a canonical centre */
#define OBJECT2_MEASURE_BRIGHT 0x80000000 /* object should be measured bright;
					     largest flag
					     pragma } OBJECT2_FLAGS */
#define OBJECT2_SAVE_MASK 0xff		/* mask of bits to save */

/*
 * This struct holds one object in one colour
 *
 * In the following comments, (NS) means Not Saved to disk at the end of a run
 */
typedef struct {
   const int id;		/* id number (set in constructor) */
   REGION *region;		/* REGION containing object */
   OBJMASK *mask;		/* MASK of which pixels are in this object */
   float rowc,rowcErr;		/* row position and error of center */
   float colc,colcErr;		/* column position and error of center */
   float sky, skyErr;		/* sky value and error near this object */
/*
 * PSF, aperture, and Petrosian fits/magnitudes
 */
   float psfCounts,psfCountsErr;/* Counts via PSF-fitting and error */
   float fiberCounts,fiberCountsErr;/* Counts within 3" aperture */
   float petroCounts, petroCountsErr;/* Counts defining a Petrosian magnitude*/
   float petroRad, petroRadErr;	/* Petrosian radius (pixels) */
   float petroR50, petroR50Err;	/* radius with 50% of petrosian light */
   float petroR90, petroR90Err;	/*  "  "    "  90% "    "    "   " "  */
/*
 * Shape of object
 */
   float Q, QErr, U, UErr;	/* <(col^2 - row^2)/radius^2> etc. */
/*
 * Properties of a certain isophote. The `Err' is the 1-sigma error,
 * the `Grad' how much the value changes if we go to an isophote
 * one magnitude fainter
 */
   float iso_rowc, iso_rowcErr, iso_rowcGrad;
   float iso_colc, iso_colcErr, iso_colcGrad;
   float iso_a, iso_aErr, iso_aGrad;
   float iso_b, iso_bErr, iso_bGrad;
   float iso_phi, iso_phiErr, iso_phiGrad;
/*
 * Model parameters for deV and exponential models
 */
   float r_deV, r_deVErr;
   float ab_deV, ab_deVErr;
   float phi_deV, phi_deVErr;
   float counts_deV, counts_deVErr;
   float r_exp, r_expErr;
   float ab_exp, ab_expErr;
   float phi_exp, phi_expErr;
   float counts_exp, counts_expErr;
/*
 * The counts for the galaxy model that fit best in the canonical band
 */
   float counts_model, counts_modelErr;
/*
 * Measures of image structure
 */
   float texture;
/*
 * Classification information
 */
   float star_L;		/* likelihood of being a star */
   float exp_L;			/* likelihood of being an exponention disk */
   float deV_L;			/* likelihood of being a deV profile */
   float fracPSF;		/* fraction of total light that's in a
				   stellar component */

   int flags;			/* flags for e.g. blends */
   OBJ_TYPE type;		/* classification: OBJ_STAR, etc. */
/*
 * Profile and extent of object
 */
   int nprof;			/* num of valid profile values in arrays */
   float profMean[NANN];	/* Radial profile, mean within annuli */
   float profMed[NANN];		/* Radial profile, median within annuli */
   float profErr[NANN];		/* Radial profile, stdev from mean in annuli */
/*
 * None of the quantities below this comment are to be written to disk
 */
   int flags2;			/* flags for internal photo info (NS) */
   float aratio;		/* axis ratio from central moments (NS) */
   float majaxis;		/* pos. angle of major axis (radians) (NS) */
   int comp;			/* flag for compression of REGION/MASK (NS)*/
   PEAKS *peaks;		/* all peaks in object (NS) */
   int npix;			/* number of pixels in object (NS) */
/*
 * These are the quantities from which the likelihoods are derived
 */
   int nu_star;				/* number of d.o.f. for psf fit (NS)*/
   float chisq_star;			/* chi^2 for psf fit (NS) */
   int nu_deV;				/* number of d.o.f. for deV fit (NS) */
   float chisq_deV;			/* chi^2 for deV fit (NS) */
   int nu_exp;				/* number of d.o.f. for disk fit (NS)*/
   float chisq_exp;			/* chi^2 for disk fit (NS) */
} OBJECT1;

/*
 * Here's an atlas image
 */
typedef struct {
   int id;				/* id number for this objc */
   const int ncolor;			/* number of colours */
   int shallow_copy;			/* is atlas image a shallow copy? */
   OBJMASK *master_mask;                /* master mask for OBJECT1s */

   int drow[NCOLOR], dcol[NCOLOR];	/* offsets wrt reference colour */
   SPANMASK *regmask[NCOLOR];		/* status mask from regions */
   OBJMASK *mask[NCOLOR];		/* object finder's masks */
   U16 *pix[NCOLOR];			/* image values in images for pixels in
					   master_mask, offset by drow, dcol */
/*
 * this field is not saved to disk
 */
   int npix;				/* dimension of pix (in each band) */
} ATLAS_IMAGE;				/* pragma SCHEMA */

/*
 * Here is the internal photo representation of an object in all bands
 */
typedef struct objc {
   const int id;			/* id number for this objc */
   const int ncolor;			/* number of colours */
   OBJECT1 **color;			/* obj1 information from each band */
   OBJ_TYPE type;			/* overall classification */
   ATLAS_IMAGE *aimage;			/* atlas image for this object */
   struct test_info *test;		/* information for testers */
   float rowc, rowcErr;			/* row position and error of centre */
   float colc, colcErr;			/* column position and error */
   float rowv, rowvErr;			/* row velocity, in pixels/frame */
   float colv, colvErr;			/* col velocity, in pixels/frame */

   int catID;			        /* catalog id number */
   int flags;				/* c.f. OBJECT1->flags */
   int flags2;				/* c.f. OBJECT1->flags2 */
   PEAKS *peaks;			/* merged peak list */
   int nchild;				/* number of children */
   struct objc *parent;			/* parent if blended */
   struct objc *sibbs;			/* list of siblings */
   struct objc *children;		/* list of children */
} OBJC;					/* pragma SCHEMA */

/*
 * And here is an OBJC reorganised for output, including an Atlas Image
 */
typedef struct {
   int id;				/* id number for this objc */
   int parent;			        /* id of parent for deblends */
   const int ncolor;			/* number of colours */
   int nchild;				/* number of children */
   OBJ_TYPE objc_type;			/* overall classification */
   int catID;			        /* catalog id number */
   int objc_flags;			/* flags from OBJC */
   int objc_flags2;			/* flags2 from OBJC */
   float objc_rowc, objc_rowcErr;	/* row position and error of centre */
   float objc_colc, objc_colcErr;	/* column position and error */
   float rowv, rowvErr;			/* row velocity, in pixels/frame (NS)*/
   float colv, colvErr;			/* col velocity, in pixels/frame (NS)*/

   ATLAS_IMAGE *aimage;			/* the atlas image */
   struct test_info *test;		/* information for testers */
/*
 * Unpacked OBJECT1s
 */
   float rowc[NCOLOR], rowcErr[NCOLOR];
   float colc[NCOLOR], colcErr[NCOLOR];
   float sky[NCOLOR], skyErr[NCOLOR];
/*
 * PSF and aperture fits/magnitudes
 */
   float psfCounts[NCOLOR], psfCountsErr[NCOLOR];
   float fiberCounts[NCOLOR], fiberCountsErr[NCOLOR];
   float petroCounts[NCOLOR], petroCountsErr[NCOLOR];
   float petroRad[NCOLOR], petroRadErr[NCOLOR];
   float petroR50[NCOLOR], petroR50Err[NCOLOR];
   float petroR90[NCOLOR], petroR90Err[NCOLOR];
/*
 * Shape of object
 */
   float Q[NCOLOR], QErr[NCOLOR], U[NCOLOR], UErr[NCOLOR];
/*
 * Properties of a certain isophote.
 */
   float iso_rowc[NCOLOR], iso_rowcErr[NCOLOR], iso_rowcGrad[NCOLOR];
   float iso_colc[NCOLOR], iso_colcErr[NCOLOR], iso_colcGrad[NCOLOR];
   float iso_a[NCOLOR], iso_aErr[NCOLOR], iso_aGrad[NCOLOR];
   float iso_b[NCOLOR], iso_bErr[NCOLOR], iso_bGrad[NCOLOR];
   float iso_phi[NCOLOR], iso_phiErr[NCOLOR], iso_phiGrad[NCOLOR];
/*
 * Model parameters for deV and exponential models
 */
   float r_deV[NCOLOR], r_deVErr[NCOLOR];
   float ab_deV[NCOLOR], ab_deVErr[NCOLOR];
   float phi_deV[NCOLOR], phi_deVErr[NCOLOR];
   float counts_deV[NCOLOR], counts_deVErr[NCOLOR];
   float r_exp[NCOLOR], r_expErr[NCOLOR];
   float ab_exp[NCOLOR], ab_expErr[NCOLOR];
   float phi_exp[NCOLOR], phi_expErr[NCOLOR];
   float counts_exp[NCOLOR], counts_expErr[NCOLOR];

   float counts_model[NCOLOR], counts_modelErr[NCOLOR];
/*
 * Measures of image structure
 */
   float texture[NCOLOR];
/*
 * Classification information
 */
   float star_L[NCOLOR];
   float exp_L[NCOLOR];
   float deV_L[NCOLOR];
   float fracPSF[NCOLOR];

   int flags[NCOLOR];
   int flags2[NCOLOR];
   OBJ_TYPE type[NCOLOR];
/*
 * Profile and extent of object
 */
   int nprof[NCOLOR];
   float profMean[NCOLOR][NANN];
   float profErr[NCOLOR][NANN];
} OBJC_IO;				/* pragma SCHEMA */

/*****************************************************************************/
/*
 * Constructors/Destructors
 */
OBJECT1 *phObject1New(void);
void phObject1Del(OBJECT1 *object1);

OBJC *phObjcNew(int ncolor);
OBJC *phObjcNewFromObjc(const OBJC *objc, /* OBJC to copy */
			int deep);	/* copy atlas images etc? NOTUSED */
void phObjcDel(OBJC *objc, int deep);
void phObjcChainDel(CHAIN *chain, int deep);

OBJC_IO *phObjcIoNew(int ncolor);
OBJC_IO *phObjcIoNewFromObjc(const OBJC *objc);
void phObjcIoDel(OBJC_IO *objc_io, int deep);

ATLAS_IMAGE *phAtlasImageNew(int ncolor);
ATLAS_IMAGE *phAtlasImageNewFromObjc(const OBJC *objc);
void phAtlasImageDel(ATLAS_IMAGE *aimage, int deep);
void phAtlasImageDelFromObjc(OBJC *objc, int deep);
ATLAS_IMAGE *phAtlasImageCopy(const ATLAS_IMAGE *old, int deep);
/*
 * remove the REGIONs from an OBJC
 */
void phRegDelFromObjc(OBJC *objc, int deep);
/*
 * an iterator for OBJCs
 */
OBJC *phObjcDescendentNext(const OBJC *objc);
/*
 * ATLAS_IMAGE utilities
 */
void phAtlasImageCut(OBJC *objc, int color, const FIELDPARAMS *fparams,
		     int val, float sigma, RANDOM *rand);
void phRegionSetFromAtlasImage(const ATLAS_IMAGE *ai,
			       int c, REGION *reg, int row0, int col0);
void phRegionSetValFromAtlasImage(const ATLAS_IMAGE *ai, int c, REGION *reg,
				  int val, float sigma, RANDOM *rand,
				  int row0, int col0);
void phAtlasImageSetFromRegion(ATLAS_IMAGE *ai, int c,
			       const REGION *data);
void phAtlasImageSetInObjmask(ATLAS_IMAGE *ai, int c,
			      const OBJMASK *omask, const U16 val);
void phAtlasImageSetIfNotInObjmask(ATLAS_IMAGE *ai, int c,
				   const OBJMASK *omask, const U16 val);
void phAtlasImagesPlusEquals(ATLAS_IMAGE *ai, const ATLAS_IMAGE *cai,
			     int bias, int c);
void phAtlasImagesMinusEquals(ATLAS_IMAGE *ai, const ATLAS_IMAGE *cai,
			      int bias, int c);

/*
 * Print utilities
 */
void phObject1PrintTerse(const OBJECT1 *obj, FILE *fp);
void phObject1PrintPretty(const OBJECT1 *obj, FILE *fp);
void phObject1PrintCellProf(const OBJECT1 *obj, FILE *fp);
void phObjcListPrint(CHAIN *objclist, char *fname);
void phObjcPrintPretty(OBJC *objc, char *fname);
/*
 * Comparison functions
 */
int phObject1Compare(const OBJECT1 *obj1, const OBJECT1 *obj2);
int phObjcCompare(const OBJC *objc1, const OBJC *objc2);
OBJC *phObjcClosest(CHAIN *chain, float xc, float yc, int color);

#endif

