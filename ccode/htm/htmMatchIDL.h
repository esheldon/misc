#define SUCCESS 0
#define FAILURE 1
#define NO_MATCHES 2

#define DEFAULT_DEPTH 10
#define DEFAULT_MAXMATCH 100000000

int 
htmMatchGetData(IDL_VPTR ra1Vptr, IDL_VPTR dec1Vptr, 
		IDL_VPTR ra2Vptr, IDL_VPTR dec2Vptr, 
		IDL_VPTR htmrev2Vptr, 
		IDL_VPTR minLeafIdVptr, IDL_VPTR maxLeafIdVptr,
		IDL_VPTR angleVptr, 

		double **ra1, double **dec1,
		double **ra2, double **dec2, 
		IDL_LONG **htmrev2,
		IDL_LONG *minLeafId, IDL_LONG *maxLeafId,
		IDL_MEMINT *n1, IDL_MEMINT *n2, 
		double **angle, IDL_MEMINT *nangle,
		IDL_MEMINT *depth, IDL_MEMINT *maxmatch);


double
gcirc(double ra1, double dec1, 
      double ra2, double dec2);

void 
htmMatchErrOut(char *message);

static void
htmMatchSetStatus(int statusVal);

int htmMatchNParams(int argc);

typedef struct {
  int num;
  double  *d;
} vStruct;

typedef struct {
  IDL_KW_RESULT_FIRST_FIELD; /* Must be first entry in structure */

  IDL_VPTR depth;
  int depth_there;

  IDL_VPTR file;
  int file_there;

  IDL_VPTR maxmatch;
  int maxmatch_there;

  IDL_VPTR status;
  int status_there;

} KW_RESULT;

static IDL_KW_PAR kw_pars[] = {

  IDL_KW_FAST_SCAN, 

  /* These must be in alphabetical order */

  /* 
     IDL_KW_VIN: can be just an input, like var=3
     IDL_KW_OUT: can be output variable, var=var
     IDL_KW_ZERO: If not there, variable is set to 0

     If the VIN is set, then the _there variables will be
     0 when an undefined name variable is sent: no good for
     returning variables!
  */

  {"DEPTH", IDL_TYP_INT, 1, IDL_KW_VIN, 
   (int *) IDL_KW_OFFSETOF(depth_there), (char *) IDL_KW_OFFSETOF(depth) },

  {"FILE", IDL_TYP_STRING, 1, IDL_KW_VIN, 
   (int *) IDL_KW_OFFSETOF(file_there), (char *) IDL_KW_OFFSETOF(file) },

  {"MAXMATCH", IDL_TYP_MEMINT, 1, IDL_KW_VIN, 
   (int *) IDL_KW_OFFSETOF(maxmatch_there), (char *) IDL_KW_OFFSETOF(maxmatch) },

  {"STATUS", IDL_TYP_UNDEF, 1, IDL_KW_OUT | IDL_KW_ZERO, 
   (int *) IDL_KW_OFFSETOF(status_there), (char *) IDL_KW_OFFSETOF(status) },


  { NULL }
};

