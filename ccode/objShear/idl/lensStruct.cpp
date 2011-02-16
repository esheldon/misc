#include "lensStruct.h"

//////////////////////////////////////////////////////////
//
// Methods for LENS class, inherits IDLStruct class
//
//////////////////////////////////////////////////////////

LENS::LENS() {} // default is do nothing
LENS::LENS(IDL_VPTR st) {
  assign(st);
  return;
}
int LENS::assign(IDL_VPTR st) {

  IDL_VPTR tmp_desc;

  // Here we should call the assign function from
  // the IDLStruct class.  Then use the tag_n_elts
  // to get numbin and the hash table to get the
  // offsets, desc, etc.
  
  IDLStruct::assign(st);
  
  // Number of bins 
  numbin = tag_n_elts("RSUM");

  // Get the offsets into the opaque data structure and save
  // them for efficient use later.  Save description, which
  // required to subscript arrays

  ra_offset = 
    getTagInfoFromIDLStruct(idlStruct, "RA", IDL_TYP_DOUBLE, 
			    ra_desc);
  dec_offset = 
    getTagInfoFromIDLStruct(idlStruct, "DEC", IDL_TYP_DOUBLE, 
			    dec_desc);
  clambda_offset = 
    getTagInfoFromIDLStruct(idlStruct, "CLAMBDA", IDL_TYP_DOUBLE, 
			    clambda_desc);
  ceta_offset = 
    getTagInfoFromIDLStruct(idlStruct, "CETA", IDL_TYP_DOUBLE, 
			    ceta_desc);

  z_offset = 
    getTagInfoFromIDLStruct(idlStruct, "Z", IDL_TYP_FLOAT, 
			    z_desc);

  zindex_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ZINDEX", IDL_TYP_LONG, 
			    zindex_desc);
  index_offset = 
    getTagInfoFromIDLStruct(idlStruct, "INDEX", IDL_TYP_LONG, 
			    index_desc);


  scritinv_offset = 
    getTagInfoFromIDLStruct(idlStruct, "SCRITINV", IDL_TYP_FLOAT, 
			    scritinv_desc);
  DL_offset = 
    getTagInfoFromIDLStruct(idlStruct, "DL", IDL_TYP_FLOAT, 
			    DL_desc);
  angMax_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ANGMAX", IDL_TYP_FLOAT, 
			    angMax_desc);

  pixelMaskFlags_offset = 
    getTagInfoFromIDLStruct(idlStruct, "PIXELMASKFLAGS", IDL_TYP_INT, 
			    pixelMaskFlags_desc);

  //totPairs_offset = 
  //  getTagInfoFromIDLStruct(idlStruct, "TOTPAIRS", IDL_TYP_FLOAT,
  //		    totPairs_desc);
  totPairs_offset = 
    getTagInfoFromIDLStruct(idlStruct, "TOTPAIRS", IDL_TYP_LONG,
			    totPairs_desc);

  ie_offset = 
    getTagInfoFromIDLStruct(idlStruct, "IE", IDL_TYP_FLOAT, 
			    ie_desc);
  weight_offset = 
    getTagInfoFromIDLStruct(idlStruct, "WEIGHT", IDL_TYP_FLOAT, 
			    weight_desc);

  sshsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "SSHSUM", IDL_TYP_FLOAT, 
			    sshsum_desc);
  wsum_ssh_offset = 
    getTagInfoFromIDLStruct(idlStruct, "WSUM_SSH", IDL_TYP_FLOAT, 
			    wsum_ssh_desc);

  angsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ANGSUM", IDL_TYP_FLOAT, 
			    angsum_desc);
  rsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "RSUM", IDL_TYP_FLOAT, 
			    rsum_desc);
  rmin_act_offset = 
    getTagInfoFromIDLStruct(idlStruct, "RMIN_ACT", IDL_TYP_FLOAT, 
			    rmin_act_desc);
  rmax_act_offset = 
    getTagInfoFromIDLStruct(idlStruct, "RMAX_ACT", IDL_TYP_FLOAT, 
			    rmax_act_desc);

  sigma_offset = 
    getTagInfoFromIDLStruct(idlStruct, "SIGMA", IDL_TYP_FLOAT, 
			    sigma_desc);
  sigmaerr_offset = 
    getTagInfoFromIDLStruct(idlStruct, "SIGMAERR", IDL_TYP_FLOAT, 
			    sigmaerr_desc);
  sigerrsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "SIGERRSUM", IDL_TYP_FLOAT, 
			    sigerrsum_desc);
  orthosig_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ORTHOSIG", IDL_TYP_FLOAT, 
			    orthosig_desc);
  orthosigerr_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ORTHOSIGERR", IDL_TYP_FLOAT, 
			    orthosigerr_desc);
  orthosigerrsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "ORTHOSIGERRSUM", IDL_TYP_FLOAT, 
			    orthosigerrsum_desc);


  wsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "WSUM", IDL_TYP_FLOAT, 
			    wsum_desc);
  owsum_offset = 
    getTagInfoFromIDLStruct(idlStruct, "OWSUM", IDL_TYP_FLOAT, 
			    owsum_desc);

  //npair_offset = 
  //  getTagInfoFromIDLStruct(idlStruct, "NPAIR", IDL_TYP_FLOAT, 
  //			    npair_desc);
  npair_offset = 
    getTagInfoFromIDLStruct(idlStruct, "NPAIR", IDL_TYP_LONG, 
			    npair_desc);

  return(1);

  ra_offset  = tagOffsetsHash["RA"];
  ra_desc    = tagDescHash["RA"];
  dec_offset = tagOffsetsHash["DEC"];
  dec_desc   = tagDescHash["DEC"];

  return(1);


}

IDL_LONG LENS::nbin() {
  return(numbin);
}

// These are just efficient interfaces to the opaque idl 
// structure. One may still use the get() and geta() methods of the
// IDLStruct class to return other fields less efficiently

double LENS::ra(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       ra_offset);
}
double LENS::dec(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       dec_offset);
}
double LENS::clambda(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       clambda_offset);
}
double LENS::ceta(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       ceta_offset);
}

float LENS::z(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       z_offset);
}

IDL_LONG LENS::zindex(IDL_MEMINT index) {
  return *(IDL_LONG *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       zindex_offset);
}
IDL_LONG LENS::index(IDL_MEMINT index) {
  return *(IDL_LONG *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       index_offset);
}

float LENS::scritinv(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       scritinv_offset);
}
float LENS::DL(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       DL_offset);
}
float LENS::angMax(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       angMax_offset);
}

short LENS::pixelMaskFlags(IDL_MEMINT index) {
  return *(short *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       pixelMaskFlags_offset);
}

// These can be modified as well as extracted, so we actually
// return the pointer here for convenience

// scalars

/*
float *LENS::totPairs(IDL_MEMINT index) {
  return (float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       totPairs_offset);
}
*/
IDL_LONG *LENS::totPairs(IDL_MEMINT index) {
  return (IDL_LONG *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       totPairs_offset);
}

float *LENS::ie(IDL_MEMINT index) {
  return (float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       ie_offset);
}
float *LENS::weight(IDL_MEMINT index) {
  return (float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       weight_offset);
}
float *LENS::sshsum(IDL_MEMINT index) {
  return (float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       sshsum_offset);
}
float *LENS::wsum_ssh(IDL_MEMINT index) {
  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      wsum_ssh_offset);
}

// arrays of size numbin
float *LENS::angsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      angsum_offset + 
		      bin*angsum_desc->value.arr->elt_len);

}
float *LENS::rsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      rsum_offset + 
		      bin*rsum_desc->value.arr->elt_len);

}
float *LENS::rmin_act(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      rmin_act_offset + 
		      bin*rmin_act_desc->value.arr->elt_len);

}
float *LENS::rmax_act(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      rmax_act_offset + 
		      bin*rmax_act_desc->value.arr->elt_len);

}


float *LENS::sigma(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  	"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      sigma_offset + 
		      bin*sigma_desc->value.arr->elt_len);

}
float *LENS::sigmaerr(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      sigmaerr_offset + 
		      bin*sigmaerr_desc->value.arr->elt_len);

}
float *LENS::sigerrsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      sigerrsum_offset + 
		      bin*sigerrsum_desc->value.arr->elt_len);

}
float *LENS::orthosig(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
   IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  	"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      orthosig_offset + 
		      bin*orthosig_desc->value.arr->elt_len);

}
float *LENS::orthosigerr(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      orthosigerr_offset + 
		      bin*orthosigerr_desc->value.arr->elt_len);

}
float *LENS::orthosigerrsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  	"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      orthosigerrsum_offset + 
		      bin*orthosigerrsum_desc->value.arr->elt_len);

}


float *LENS::wsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      wsum_offset + 
		      bin*wsum_desc->value.arr->elt_len);


}
float *LENS::owsum(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      owsum_offset + 
		      bin*owsum_desc->value.arr->elt_len);

}

/*
float *LENS::npair(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (float *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      npair_offset + 
		      bin*npair_desc->value.arr->elt_len);

}
*/
IDL_LONG *LENS::npair(IDL_MEMINT index, IDL_MEMINT bin) {

  if (bin < 0 || bin >= numbin)
    IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
  		"bin subscript must be in [0,nbin-1]");

  return (IDL_LONG *) (idlStruct->value.s.arr->data + 
		      index*idlStruct->value.arr->elt_len + 
		      npair_offset + 
		      bin*npair_desc->value.arr->elt_len);

}

//////////////////////////////////////////////////////////
// Methods for SOURCE class, inherits IDLStruct class
//////////////////////////////////////////////////////////

SOURCE::SOURCE() {} // default constructor: do nothing
SOURCE::SOURCE(IDL_VPTR st) {

  // point at the idl structure
  idlStruct = st;
  n_elements = idlStruct->value.arr->n_elts;

  // Get the offsets into the opaque data structure and save
  // them for efficient use later.  Save description, which
  // required to subscript arrays

  leafId_offset = 
    getTagInfoFromIDLStruct(idlStruct, "LEAFID", IDL_TYP_LONG, 
			    leafId_desc);

  clambda_offset = 
    getTagInfoFromIDLStruct(idlStruct, "CLAMBDA", IDL_TYP_DOUBLE, 
			    clambda_desc);
  ceta_offset = 
    getTagInfoFromIDLStruct(idlStruct, "CETA", IDL_TYP_DOUBLE, 
			    ceta_desc);

  e1_offset = 
    getTagInfoFromIDLStruct(idlStruct, "E1_RECORR", IDL_TYP_FLOAT, 
			    e1_desc);
  e2_offset = 
    getTagInfoFromIDLStruct(idlStruct, "E2_RECORR", IDL_TYP_FLOAT, 
			    e2_desc);

  e1e1err_offset = 
    getTagInfoFromIDLStruct(idlStruct, "E1E1ERR", IDL_TYP_FLOAT, 
			    e1e1err_desc);
  e1e2err_offset = 
    getTagInfoFromIDLStruct(idlStruct, "E1E2ERR", IDL_TYP_FLOAT, 
			    e1e2err_desc);
  e2e2err_offset = 
    getTagInfoFromIDLStruct(idlStruct, "E2E2ERR", IDL_TYP_FLOAT, 
			    e2e2err_desc);

  photoz_z_offset = 
    getTagInfoFromIDLStruct(idlStruct, "PHOTOZ_Z", IDL_TYP_FLOAT, 
			    photoz_z_desc);
  photoz_zerr_offset = 
    getTagInfoFromIDLStruct(idlStruct, "PHOTOZ_ZERR", IDL_TYP_FLOAT, 
			    photoz_zerr_desc);


}

// These are just efficient interfaces to the opaque idl 
// structure. One may still use the get() method of the
// IDLStruct class to return other fields less efficiently


IDL_LONG SOURCE::leafId(IDL_MEMINT index) {
  return *(IDL_LONG *) (idlStruct->value.s.arr->data + 
		     index*idlStruct->value.arr->elt_len + 
		     leafId_offset);
}

double SOURCE::clambda(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       clambda_offset);
}
double SOURCE::ceta(IDL_MEMINT index) {
  return *(double *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       ceta_offset);
}

float SOURCE::e1(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       e1_offset);
}
float SOURCE::e2(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       e2_offset);
}
float SOURCE::e1e1err(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       e1e1err_offset);
}
float SOURCE::e1e2err(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       e1e2err_offset);
}
float SOURCE::e2e2err(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       e2e2err_offset);
}

float SOURCE::photoz_z(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       photoz_z_offset);
}
float SOURCE::photoz_zerr(IDL_MEMINT index) {
  return *(float *) (idlStruct->value.s.arr->data + 
		       index*idlStruct->value.arr->elt_len + 
		       photoz_zerr_offset);
}
