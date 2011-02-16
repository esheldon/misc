/********************************************************************************

NAME:
admomatlas.c

PURPOSE:
read in an SDSS object list, read their atlas images, process those
images using adaptive moment code, and output the results.

CALLING SEQUENCE:
admomatlas infile outfile [atlas_directory]

 *********************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "dervish.h"
#include "phFits.h"
#include "phConsts.h"
#include "export.h"
#include "atlas.h"
#define ncolor 5
#define STRUCTNAME "STRUCT1"

/* Function Prototypes */
void usagemain(char *name);
void setzero(float *Mcc, float *Mrr, float *Mcr, float *M1err, 
		float *Mrho4,int *whyflag);
void writehead(FILE *ofp);

//char * atlasname(char *atldir, char *run, char *camcol, char *field);
void atlasname(char *atldir, char *run, char *camcol, char *field, char* name);

/* global variables */

const int runlength=6;
const int fieldlength=4;
/*const int ncols=9; */
const int ncols=17;

/************************************
 ** Main
 ************************************/

int
main(int argc, char *argv[]) {

	int minarg=3;                             /* Min # of args */
	char *inf, *outf,                         /* infile,outfile*/
		 *atldir, atlfile[255];                  /* atlas dir, atlas file*/
	FILE *ifp, *ofp;                          /* input/output file pointers*/
	int run, rerun,                           /* uniquely identify photo object*/
		camcol, field, 
		id,
		fieldold;
	char srun[10],srerun[10],
		 scamcol[10],sfield[10],                 /* string versions */
		 sid[10];
	float colcg, rowcg,                       /* positional info in each bandpass*/
		  colcr, rowcr, 
		  colci, rowci, 
		  petroRadg, petroRadr, petroRadi,    /* PHOTO size info */
		  skysigg, skysigr, skysigi;          /* sky variance */
	float colc[ncolor], rowc[ncolor], petroRad[ncolor], skysig[ncolor];
	float xcen,ycen;
	long nrow,ncol;                         /* number of rows/columns in atlas image*/
	int color,minclr=1,maxclr=3,row0,col0;  /* color of atlas image to get*/
	float bkgd;
	REGION *reg;                            /* region (atlas image) */
	float Mcc, Mrr, Mcr, M1err, Mrho4;
	float ixx[ncolor], iyy[ncolor], ixy[ncolor], rho4[ncolor],momerr[ncolor];
	int twhyflag, whyflag[ncolor],dummy,iobj;

	if(argc < minarg) {
		usagemain(argv[0]);
		return 0;
	}

	bkgd = (float)SOFT_BIAS;

	inf = argv[1];
	outf = argv[2];
	if(argc >= 4) 
		atldir = argv[3];
	else 
		atldir = "";


	/* Print out some info*/
	printf("Input file name: %s\n", inf);
	printf("Output file name: %s\n", outf);


	/* Open files */
	if ( (ifp = fopen(inf,"r")) == NULL) {
		printf("Cannot open file: %s for input\n", inf);
		return 0;
	}
	if ( (ofp = fopen(outf,"w")) == NULL) {
		printf("Cannot open file: %s for output\n", outf);
		return 0;
	}

	/* Write Yanny Header */
	/*printf("Writing Yanny header\n");
	  writehead(ofp);*/

	printf("Processing File\n");

	/**********************************************************/
	/* will only do g,r,i (color=1,2,3), so initialize arrays */
	/**********************************************************/

	ixx[0]=0.;ixx[4]=0.;
	ixy[0]=0.;ixy[4]=0.;
	iyy[0]=0.;iyy[4]=0.;
	rho4[0]=0.;rho4[4]=0.;
	momerr[0]=0.;momerr[4]=0.;
	whyflag[0]=0;whyflag[4]=0;

	fieldold = 0;
	run=0; rerun=0;camcol=0;field=0;id=0;

	/* loop over objects from input file */
	/* reading run,rerun,camcol,field,id as string for making file names,
	 * but will also convert to int */
	iobj=0;
	while( fscanf(ifp,"%s %s %s %s %s %f %f %f %f %f %f %f %f %f %f %f %f", 
				srun, srerun, scamcol, sfield, sid, 
				&colcg, &rowcg, &colcr, &rowcr, &colci, &rowci, 
				&petroRadg, &petroRadr, &petroRadi,
				&skysigg, &skysigr, &skysigi) == ncols )
	{
		colc[1]=colcg; colc[2]=colcr; colc[3]=colci;
		rowc[1]=rowcg; rowc[2]=rowcr; rowc[3]=rowci;
		petroRad[1]=petroRadg; petroRad[2]=petroRadr; petroRad[3]=petroRadi;
		skysig[1]=skysigg; skysig[2]=skysigr; skysig[3]=skysigi;
		run=atoi(srun); rerun=atoi(srerun); camcol=atoi(scamcol); 
		field=atoi(sfield); id=atoi(sid);

		/* get new atlas file name if needed*/
		if (field != fieldold) {
			//strcpy( atlfile, atlasname(atldir,srun,scamcol,sfield) );
			atlasname(atldir,srun,scamcol,sfield,atlfile);
			printf("\n%s\n", atlfile);fflush(stdout);fflush(stdout);
		}
		fieldold = field;

		for(color=minclr;color<=maxclr;++color) {
			reg = atls(atlfile, color, id);	

			if (reg != NULL) {
				xcen = colc[color]-(float)(reg->col0)-0.5;
				ycen = rowc[color]-(float)(reg->row0)-0.5;
				setzero(&Mcc,&Mrr,&Mcr,&M1err,&Mrho4,&twhyflag);
				calc_adaptive_moments(reg, bkgd, skysig[color],
						petroRad[color], xcen, ycen,
						&Mcc, &Mrr, &Mcr,
						&M1err, &Mrho4, &twhyflag);

				if (twhyflag != 0) setzero(&Mcc,&Mrr,&Mcr,&M1err,&Mrho4,&dummy);

				ixx[color]=Mcc; iyy[color]=Mrr; ixy[color]=Mcr; 
				rho4[color] = Mrho4; momerr[color]=M1err;
				whyflag[color]=twhyflag;

				shRegDel(reg);
			} else {

				ixx[color]=0.; iyy[color]=0.; ixy[color]=0.; 
				rho4[color]=0.; momerr[color]=0.;
				whyflag[color]=0;
			}
		} /*end loop over bandpasses*/

		/*print to file*/
		fprintf(ofp,"%d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %d %d\n",
				run,rerun,camcol,field,id,
				ixx[1],ixx[2],ixx[3],
				ixy[1],ixy[2],ixy[3],
				iyy[1],iyy[2],iyy[3],
				momerr[1],momerr[2],momerr[3],
				rho4[1],rho4[2],rho4[3],
				whyflag[1],whyflag[2],whyflag[3]);

		if (!(++iobj % 20)) {
			printf(".");
			fflush(stdout);
		}
	} /* end loop over objects*/
	printf("\nDone\n");
	fclose(ifp);
	fclose(ofp);

}    /* end main */

void usagemain(char *name) {

	printf("Usage: %s inputfile outputfile [atlas_dir]\n", name);

}

/*******************************
 * return the atlas file name
 *******************************/

void atlasname(char *atldir,char *run, char *camcol, char *field, char* name) {

	//char name[255],zeros1[10],zeros2[10];
	char zeros1[10],zeros2[10];
	int len,diff;
	strcpy(name,atldir);
	strcat(name,"fpAtlas-");

	strcpy(zeros1,"");
	strcpy(zeros2,"");

	len = strlen(run);
	diff = runlength-len;
	while (diff--) strcat(zeros1,"0");
	strcat(zeros1,run);
	strcat(name,zeros1);
	strcat(name,"-");

	strcat(name,camcol);
	strcat(name,"-");

	len = strlen(field);
	diff = fieldlength-len;
	while (diff--) strcat(zeros2,"0");
	strcat(zeros2,field);
	strcat(name,zeros2);
	strcat(name,".fit");

}


/*
   char *atlasname(char *atldir,char *run, char *camcol, char *field) {

   char name[255],zeros1[10],zeros2[10];
   int len,diff;
   strcpy(name,atldir);
   strcat(name,"fpAtlas-");

   strcpy(zeros1,"");
   strcpy(zeros2,"");

   len = strlen(run);
   diff = runlength-len;
   while (diff--) strcat(zeros1,"0");
   strcat(zeros1,run);
   strcat(name,zeros1);
   strcat(name,"-");

   strcat(name,camcol);
   strcat(name,"-");

   len = strlen(field);
   diff = fieldlength-len;
   while (diff--) strcat(zeros2,"0");
   strcat(zeros2,field);
   strcat(name,zeros2);
   strcat(name,".fit");

   return name;

   }
   */

void setzero(float *Mcc, float *Mrr, float *Mcr, float *M1err, 
		float *Mrho4,int *whyflag)
{
	*Mcc=0.;
	*Mrr=0.;
	*Mcr=0.;
	*M1err=0.;
	*Mrho4=0.;
	*whyflag=0;
}

void writehead(FILE *ofp) {

	fprintf(ofp,"typedef struct { \n");
	fprintf(ofp," int RUN; \n");
	fprintf(ofp," int RERUN; \n");
	fprintf(ofp," int CAMCOL; \n");
	fprintf(ofp," int FIELD; \n");
	fprintf(ofp," int ID; \n");
	fprintf(ofp," float IXX[5]; \n");
	fprintf(ofp," float IXY[5]; \n");
	fprintf(ofp," float IYY[5]; \n");
	fprintf(ofp," float MOMERR[5]; \n");
	fprintf(ofp," float RHO4[5]; \n");
	fprintf(ofp," int WHYFLAG[5]; \n");
	fprintf(ofp,"} %s; \n",STRUCTNAME);
	fprintf(ofp,"\n\n");
	fflush(ofp);

}
