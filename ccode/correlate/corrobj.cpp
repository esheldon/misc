#include "corrobj.h"

#define debug 0
#define debug2 0

corrobj::corrobj() {}
corrobj::corrobj(char *parfile)
{
    read(parfile);

    // Initialize the htm index
    htm = new htmInterface( (size_t) par.depth );
    spatialIndex = &htm->index();

    binobj = new binner(par);

    make_output();
    make_edge_output();
}

corrobj::~corrobj()
{
    delete kcorr;
    delete htm;
    delete binobj;
}


//////////////////////////////////////////////////////
// 
// Input/Output
//
//////////////////////////////////////////////////////

// Read each item in turn.
void corrobj::read(char *parfile)
{

    read_par(parfile);
    kcorr = new kcorr_table(par.kcorr_file);
    read_primary();

    if (par.corrtype == 1 || par.corrtype == 3)
        read_secondary();
    else
        read_random_secondary();

    read_rev();

}



// Read the par file
// Now requires version 0.9 of the config files
void corrobj::read_par(char *file)
{

    char name[100];

    FILE *fptr;
    if ( ! (fptr = fopen(file, "r") ) )
    {
        cout << "Could not open par file: " << file << endl;
        exit(1);
    }

    cout << endl <<
        "Reading parameters from file " << file << endl;

    fscanf(fptr, "%s %f", name, &par.version);
    if ( abs(par.version - 0.9) > 0.001)
    {
        cout<<"Config file version must be 0.9, got "<<par.version<<endl;
        fflush(stdout);
        exit(1);
    }
    fscanf(fptr, "%s %s", name, &par.sample);
    fscanf(fptr, "%s %s", name, &par.primary_file);
    fscanf(fptr, "%s %s", name, &par.secondary_file);
    fscanf(fptr, "%s %s", name, &par.rev_file);
    fscanf(fptr, "%s %s", name, &par.kcorr_file);
    fscanf(fptr, "%s %s", name, &par.output_file);

    fscanf(fptr, "%s %d", name, &par.corrtype);
    fscanf(fptr, "%s %d", name, &par.output_type);

    fscanf(fptr, "%s %f", name, &par.h);
    fscanf(fptr, "%s %f", name, &par.omega_m);

    fscanf(fptr, "%s %d", name, &par.nrad);
    fscanf(fptr, "%s %f", name, &par.rmin);
    fscanf(fptr, "%s %f", name, &par.rmax);

    par.logRmin = log10(par.rmin);
    par.logRmax = log10(par.rmax);  
    par.logBinsize = ( par.logRmax - par.logRmin )/par.nrad;


    fscanf(fptr, "%s %d", name, &par.nlum);
    fscanf(fptr, "%s %d", name, &par.lumband);
    fscanf(fptr, "%s %f", name, &par.loglmin);
    fscanf(fptr, "%s %f", name, &par.loglmax);

    par.lmin = pow(10.0, par.loglmin-10.0);
    par.lmax = pow(10.0, par.loglmax-10.0);


    fscanf(fptr, "%s %d", name, &par.nkgmr);
    fscanf(fptr, "%s %f", name, &par.kgmrmin);
    fscanf(fptr, "%s %f", name, &par.kgmrmax);

    fscanf(fptr, "%s %d", name, &par.comoving);

    fscanf(fptr, "%s %d", name, &par.depth);

    cout << " par version    = " << par.version << endl;
    cout << " sample         = " << par.sample << endl;
    cout << " primary_file   = " << par.primary_file << endl;
    cout << " secondary_file = " << par.secondary_file << endl;
    cout << " rev_file       = " << par.rev_file << endl;
    cout << " kcorr_file     = " << par.kcorr_file << endl;
    cout << " output_file    = " << par.output_file << endl;

    cout << endl;
    cout << " Correlation type = " << par.corrtype << endl;
    cout << endl;
    cout << " output_type    = "<<par.output_type<<endl;
    cout << " h = " << par.h << endl;
    cout << " omega_m = " << par.omega_m << endl;

    cout << " nrad = " << (int) par.nrad << endl;
    cout << " rmin = " << par.rmin << endl;
    cout << " rmax = " << par.rmax << endl;

    cout << " nlum = " << (int) par.nlum << endl;
    cout << " lumband = " <<  par.lumband << endl;
    cout << " loglmin = " << par.loglmin << endl;
    cout << " loglmax = " << par.loglmax << endl;
    cout << " logRmin = " << par.logRmin << endl;
    cout << " logRmax = " << par.logRmax << endl;
    cout << " logBinsize = " << par.logBinsize << endl;
    cout << " lmin = " << par.lmin << endl;
    cout << " lmax = " << par.lmax << endl;

    cout << " nkgmr = " << (int) par.nkgmr << endl;
    cout << " kgmrmin = " << par.kgmrmin << endl;
    cout << " kgmrmax = " << par.kgmrmax << endl;

    cout << " comoving = " << (int) par.comoving << endl;
    cout << " depth = " << (int) par.depth << endl;

    fclose(fptr);




}


// Read the primaries
void corrobj::read_primary()
{

    FILE *fptr;

    if (! (fptr = fopen(par.primary_file, "r")) )
    {
        cout << "Cannot open primary file " << par.primary_file << endl;
        exit(45);
    }

    // Read the number of rows
    par.nprimary = nrows(fptr);
    cout << endl 
        << "Reading " << par.nprimary << " from primary file " 
        << par.primary_file << endl;

    int nlines = 0;
    char c;
    while (nlines < PRIMARY_HEADER_LINES) 
    {
        c = getc(fptr);
        if (c == '\n') nlines++;
    }
  
    primary.resize(par.nprimary);
  
    float H0 = 100*par.h;
    for (int row=0; row< par.nprimary; row++)
    {
        fread( &primary[row].index, sizeof(int), 1, fptr);

        fread( &primary[row].ra, sizeof(double), 1, fptr);
        fread( &primary[row].dec, sizeof(double), 1, fptr);      
      
        fread( &primary[row].z, sizeof(float), 1, fptr);

        primary[row].DA = angDist(H0, par.omega_m, primary[row].z);

    }

    int n = par.nprimary-1;

    cout << "Testing " << endl;
    printf("     %d %f %lf %lf %f\n",
            primary[0].index, primary[0].ra, primary[0].dec, 
            primary[0].z, primary[0].DA);
    printf("     %d %f %lf %lf %f\n",
            primary[n].index, primary[n].ra, primary[n].dec, 
            primary[n].z, primary[n].DA);
  
}

void corrobj::read_secondary()
{

    FILE *fptr;

    if (! (fptr = fopen(par.secondary_file, "r")) )
    {
        cout << "Cannot open secondary file " << par.secondary_file << endl;
        exit(45);
    }

    /* Read the number of rows */
    par.nsecondary = nrows(fptr);
    cout << endl 
        << "Reading " << par.nsecondary << " from secondary file " 
        << par.secondary_file << endl;

    int nlines = 0;
    char c;
    while (nlines < SECONDARY_HEADER_LINES) 
    {
        c = getc(fptr);
        if (c == '\n') nlines++;
    }


    secondary.resize(par.nsecondary);

    for (int row=0; row< par.nsecondary; row++)
    {
        fread( &secondary[row].ra,  sizeof(double), 1, fptr);
        fread( &secondary[row].dec, sizeof(double), 1, fptr);      

        fread( &secondary[row].gflux, sizeof(float), 1, fptr);
        fread( &secondary[row].rflux, sizeof(float), 1, fptr);
        fread( &secondary[row].iflux, sizeof(float), 1, fptr);


        fread( &secondary[row].htm_index, sizeof(int), 1, fptr);
    }

    int n = par.nsecondary-1;

    cout << "Testing " << endl;
    printf("     %lf %lf %f %f %f %d\n",
            secondary[0].ra, secondary[0].dec, 
            secondary[0].gflux, secondary[0].rflux, secondary[0].iflux, 
            secondary[0].htm_index);
    printf("     %lf %lf %f %f %f %d\n",
            secondary[n].ra, secondary[n].dec, 
            secondary[n].gflux, secondary[n].rflux, secondary[n].iflux, 
            secondary[n].htm_index);

}

void corrobj::read_random_secondary()
{

    FILE *fptr;

    if (! (fptr = fopen(par.secondary_file, "r")) )
    {
        cout << "Cannot open random secondary file " << par.secondary_file << endl;
        exit(45);
    }

    /* Read the number of rows */
    par.nsecondary = nrows(fptr);
    cout << endl 
        << "Reading " << par.nsecondary << " from random secondary file " 
        << par.secondary_file << endl;

    int nlines = 0;
    char c;
    while (nlines < RANDOM_SECONDARY_HEADER_LINES) 
    {
        c = getc(fptr);
        if (c == '\n') nlines++;
    }


    random_secondary.resize(par.nsecondary);

    for (int row=0; row< par.nsecondary; row++)
    {

        fread( &random_secondary[row].ra,  sizeof(double), 1, fptr);
        fread( &random_secondary[row].dec, sizeof(double), 1, fptr);      
        fread( &random_secondary[row].htm_index, sizeof(int), 1, fptr);

    }

    int n = par.nsecondary-1;

    cout << "Testing " << endl;
    printf("     %lf %lf %d\n",
            random_secondary[0].ra, random_secondary[0].dec, 
            random_secondary[0].htm_index);
    printf("     %lf %lf %d\n",
            random_secondary[n].ra, random_secondary[n].dec, 
            random_secondary[n].htm_index);

}


int corrobj::nrows(FILE *fptr)
{

    short i;
    char c;
    char nrows_string[11];

    /* 9 chars for "NROWS  = " */
    for (i=0;i<9;i++)
        c = getc(fptr);

    /* now read nrows */
    for (i=0;i<11;i++)
    {
        nrows_string[i] = getc(fptr);
    }

    return( atoi(nrows_string) );

}


void corrobj::read_rev()
{

    FILE *fptr;

    if (! (fptr = fopen(par.rev_file, "r")) )
    {
        cout << "Cannot open reverse indices file " << par.rev_file << endl;
        exit(45);
    }
  

    fread( &nrev,  sizeof(int), 1, fptr);
    fread( &min_htmind, sizeof(int), 1, fptr);
    fread( &max_htmind, sizeof(int), 1, fptr);

    cout << endl 
        << "Reading " << nrev << " rev data from file " << par.rev_file << endl;

    rev.resize(nrev);

    fread( (char *)&rev[0], sizeof(int), nrev, fptr);

    cout << "Testing " << endl;
    cout << "     min_htmind: " << min_htmind << " max_htmind: " << max_htmind << endl;
    cout << "     " << rev[0] << " " << rev[nrev-1] << endl;

    fclose(fptr);

}



// Output struct
void corrobj::make_output()
{

    output.nrad = par.nrad;
    output.nlum = par.nlum;
    output.nkgmr = par.nkgmr;

    output.rsum.resize(par.nrad);

    output.kgflux.resize(par.nrad);
    output.krflux.resize(par.nrad);
    output.kiflux.resize(par.nrad);

    switch (par.output_type)
    {
        case 1:
            output.radcounts.resize(par.nrad);
            break;
        case 2:
            output.radcounts.resize(par.nrad);
            output.radlum.resize(par.nrad);
            break;
        case 3:
            output.counts.Allocate(par.nkgmr,  par.nlum, par.nrad);
            output.lum.Allocate(par.nkgmr,  par.nlum, par.nrad);
            break;
        default: break;
    }

    reset_output();

}

void corrobj::reset_output()
{

    output.index = -1;
    output.totpairs = 0;
    for (int i=0; i<output.nrad; i++)
    {
        output.rsum[i] = 0.0;

        output.kgflux[i] = 0.0;
        output.krflux[i] = 0.0;
        output.kiflux[i] = 0.0;
    }

    switch (par.output_type)
    {
        case 1:
            for (int i=0; i<par.nrad; i++)
                output.radcounts[i] = 0;
            break;
        case 2:
            for (int i=0; i<par.nrad; i++)
            {
                output.radcounts[i] = 0;
                output.radlum[i] = 0.0;
            }
            break;
        case 3:
            for (int ci=0; ci< output.nkgmr; ci++)
            {
                for (int li=0; li< output.nlum; li++)
                {
                    for (int ri=0; ri< output.nrad; ri++)  
                    {
                        output.counts[ci][li][ri] = 0;
                        output.lum[ci][li][ri] = 0.0;
                    }
                }
            }
            break;
        default: break;
    }


}



// Writing to output file
void corrobj::write_header(FILE *fptr)
{

    int nrad = par.nrad;

    // Required header keywords
    fprintf(fptr, "NROWS  = %15d\n", par.nprimary);
    fprintf(fptr, "FORMAT = BINARY\n");
    fprintf(fptr, "BYTE_ORDER = IEEE_LITTLE\n");  
    fprintf(fptr, "IDLSTRUCT_VERSION = 0.9\n");

    // optional keywords

    fprintf(fptr, "parversion = '%s'  # version of par files\n", par.version);
    fprintf(fptr, "sample = '%s'      # Sample name\n", par.sample);
    fprintf(fptr, "corrtype = %d      # Correlation type\n", par.corrtype);

    fprintf(fptr, "num = %d           # Number of primaries\n", par.nprimary);

    fprintf(fptr, "h = %f             # Hubble parameter/100 km/s\n", par.h);
    fprintf(fptr, "omega_m = %f       # omega_m, flat assumed\n", par.omega_m);

    fprintf(fptr, "nrad = %d          # Number of bins\n", par.nrad);
    fprintf(fptr, "nlum = %d          # Number of bins\n", par.nlum);
    fprintf(fptr, "lumband = %d       # band for lum binning\n", par.lumband);
    fprintf(fptr, "nkgmr = %d          # Number of bins\n", par.nkgmr);

    fprintf(fptr, "rmin = %f          # mininum radius (kpc)\n", par.rmin);
    fprintf(fptr, "rmax = %f          # maximum radius (kpc)\n", par.rmax);

    fprintf(fptr, "comoving = %d      # comoving radii?\n", par.comoving);
    fprintf(fptr, "htm_depth = %d     # depth of HTM search tree\n", par.depth);


    // field descriptions

    fprintf(fptr, "index 0L\n");

    fprintf(fptr, "totpairs 0L\n");

    fprintf(fptr, "rsum fltarr(%d)\n", nrad);

    fprintf(fptr, "kgflux fltarr(%d)\n", nrad);
    fprintf(fptr, "krflux fltarr(%d)\n", nrad);
    fprintf(fptr, "kiflux fltarr(%d)\n", nrad);

    switch (par.output_type)
    {
        case 1:
            fprintf(fptr, "radcounts lonarr(%d)\n", par.nrad);
            break;
        case 2:
            fprintf(fptr, "radcounts lonarr(%d)\n", par.nrad);
            fprintf(fptr, "radlum fltarr(%d)\n", par.nrad);
            break;
        case 3:
            // Note the order is backward because IDL uses fortran ordering
            fprintf(fptr, "counts lonarr(%d,%d,%d)\n", 
                    par.nrad, par.nlum, par.nkgmr);
            fprintf(fptr, "lum fltarr(%d,%d,%d)\n", 
                    par.nrad, par.nlum, par.nkgmr);
            break;
        default: break;
    }


    fprintf(fptr, "END\n");
    fprintf(fptr, "\n");

}


void corrobj::write_output(FILE *fptr)
{

    int nrad = par.nrad;
    int nlum = par.nlum;
    int nkgmr = par.nkgmr;

    fwrite(&output.index, sizeof(int), 1, fptr);

    fwrite(&output.totpairs, sizeof(int), 1, fptr);

    fwrite((char *)&output.rsum[0], sizeof(float), nrad, fptr);

    fwrite((char *)&output.kgflux[0], sizeof(float), nrad, fptr);
    fwrite((char *)&output.krflux[0], sizeof(float), nrad, fptr);
    fwrite((char *)&output.kiflux[0], sizeof(float), nrad, fptr);

    switch (par.output_type)
    {
        case 2:
            fwrite((char *)&output.radcounts[0], sizeof(int), nrad, fptr);
            fwrite((char *)&output.radlum[0], sizeof(float), nrad, fptr);
            break;
        case 3:
            fwrite((char *)&output.counts[0][0][0], 
                    sizeof(int), nkgmr*nlum*nrad, fptr);
            fwrite((char *)&output.lum[0][0][0], 
                    sizeof(float), nkgmr*nlum*nrad, fptr);
            break;
        case 1:
            fwrite((char *)&output.radcounts[0], sizeof(int), nrad, fptr);
            break;
        default: break;
    }

}


// For when we just save the pairs
void corrobj::write_pairindex_header(FILE *fptr)
{

    int nrad = par.nrad;

    // Required header keywords
    fprintf(fptr, "NROWS  = %15d\n", par.nprimary);
    fprintf(fptr, "FORMAT = BINARY\n");
    fprintf(fptr, "BYTE_ORDER = IEEE_LITTLE\n");  
    fprintf(fptr, "IDLSTRUCT_VERSION = 0.9\n");

    // optional keywords

    fprintf(fptr, "sample = '%s'      # Sample name\n", par.sample);
    fprintf(fptr, "corrtype = %d      # Correlation type\n", par.corrtype);

    fprintf(fptr, "num = %d           # Number of primaries\n", par.nprimary);

    fprintf(fptr, "h = %f             # Hubble parameter/100 km/s\n", par.h);
    fprintf(fptr, "omega_m = %f       # omega_m, flat assumed\n", par.omega_m);

    fprintf(fptr, "rmin = %f          # mininum radius (kpc)\n", par.rmin);
    fprintf(fptr, "rmax = %f          # maximum radius (kpc)\n", par.rmax);

    fprintf(fptr, "comoving = %d      # comoving radii?\n", par.comoving);
    fprintf(fptr, "htm_depth = %d     # depth of HTM search tree\n", par.depth);


    // field descriptions

    fprintf(fptr, "index 0L\n");
    fprintf(fptr, "npairs 0L\n");

    /*
       fprintf(fptr, "r 0.0\n");

       fprintf(fptr, "kgflux 0.0\n");
       fprintf(fptr, "krflux 0.0\n");
       fprintf(fptr, "kiflux 0.0\n");
       */
    fprintf(fptr, "END\n");
    fprintf(fptr, "\n");
}












// Edge output struct
void corrobj::make_edge_output()
{

    edge_output.rsum.resize(par.nrad);
    edge_output.counts.resize(par.nrad);
    reset_edge_output();

}

void corrobj::reset_edge_output()
{

    edge_output.index = -1;
    edge_output.totpairs = 0;
    for (int i=0; i<par.nrad; i++)
    {
        edge_output.rsum[i] = 0.0;
        edge_output.counts[i] = 0;
    }
}


// Writing to output file
void corrobj::write_edge_header(FILE *fptr)
{

    int nrad = par.nrad;

    // Required header keywords
    fprintf(fptr, "NROWS  = %15d\n", par.nprimary);
    fprintf(fptr, "FORMAT = BINARY\n");
    fprintf(fptr, "BYTE_ORDER = IEEE_LITTLE\n");  
    fprintf(fptr, "IDLSTRUCT_VERSION = 0.9\n");

    // optional keywords
    fprintf(fptr, "sample = '%s'      # Sample name\n", par.sample);
    fprintf(fptr, "corrtype = %d      # Correlation type\n", par.corrtype);

    fprintf(fptr, "num = %d           # Number of lenses\n", par.nprimary);

    fprintf(fptr, "h = %f             # Hubble parameter/100 km/s\n", par.h);
    fprintf(fptr, "omega_m = %f       # omega_m, flat assumed\n", par.omega_m);

    fprintf(fptr, "nrad = %d          # Number of bins\n", par.nrad);

    fprintf(fptr, "rmin = %f          # mininum radius (kpc)\n", par.rmin);
    fprintf(fptr, "rmax = %f          # maximum radius (kpc)\n", par.rmax);

    fprintf(fptr, "comoving = %d      # comoving radii?\n", par.comoving);

    fprintf(fptr, "htm_depth = %d     # depth of HTM search tree\n", par.depth);


    // field descriptions

    fprintf(fptr, "index 0L\n");

    fprintf(fptr, "totpairs 0L\n");

    fprintf(fptr, "rsum fltarr(%d)\n", nrad);

    fprintf(fptr, "counts lonarr(%d)\n", nrad);

    fprintf(fptr, "END\n");
    fprintf(fptr, "\n");

}

void corrobj::write_edge_output(FILE *fptr)
{

    int nrad = par.nrad;

    fwrite(&edge_output.index, sizeof(int), 1, fptr);

    fwrite(&edge_output.totpairs, sizeof(int), 1, fptr);

    fwrite((char *)&edge_output.rsum[0], sizeof(float), nrad, fptr);

    fwrite((char *)&edge_output.counts[0], sizeof(int), nrad, fptr);

}








//////////////////////////////////////////////////////////////////////////
//
//
// Tools for correlation functions
//
//
//////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Intersect ra/dec and circle with the htm tree
////////////////////////////////////////////////////////////////

void corrobj::intersect(double ra, double dec, float DA, vector<uint64> &idlist)
{

    double d = cos( par.rmax/DA );
    ValVec<uint64> plist, flist;

    // We must intitialize each time because it remembers it's state
    // internally
    SpatialDomain domain;    // initialize empty domain

    domain.setRaDecD(ra,dec,d); //put in ra,dec,d E.S.S.

    domain.intersect(spatialIndex, plist, flist);	  // intersect with list

    //cout << "Number in plist+flist: " << plist.length() + flist.length() << endl;
    //cout << "   Separately: " << plist.length() << " " << flist.length() << endl;
    // Save the result in idlist. This is not a bottleneck
    idlist.resize( flist.length() + plist.length() );

    int idCount=0;
    // ----------- FULL NODES -------------
    for(int i = 0; i < flist.length(); i++)
    {  
        idlist[idCount] = flist(i);
        idCount++;
    }
    // ----------- Partial Nodes ----------
    for(int i = 0; i < plist.length(); i++)
    {  
        idlist[idCount] = plist(i);
        idCount++;
    }


}

////////////////////////////////////////////////////////////////
// Get ids of secondary objects in the triangle list and within 
// the rmin/rmax annulus
////////////////////////////////////////////////////////////////

void corrobj::get_seclist(
        double ra, double dec, float DA, vector <uint64> &idlist, 
        vector<int> &seclist, vector<float> &radlist)
{

    double sra, sdec;

    if (seclist.size() > 0) {
        //printf("Truncating seclist\n");
        seclist.resize(0);
    }
    if (radlist.size() > 0) {
        //printf("Truncating radlist\n");
        radlist.resize(0);
    }
    // Now loop over leaf ids and get the sources
    for(int i=0; i<idlist.size();i++)
    {

        int leafId = idlist[i];

        // Convert leafid into bin number
        int leafBin = idlist[i] - min_htmind;


        // Check if there are sources in this leafid
        if ( leafId >= min_htmind  &&  leafId <= max_htmind)
        {

            // Look for sources in this triangle
            if (rev[leafBin] != rev[leafBin+1]) 
            {

                int nLeafBin = rev[leafBin+1] - rev[leafBin];

                // Loop over sources in this leaf
                for(int iphot=0;iphot<nLeafBin;iphot++)
                {

                    int photUse = rev[ rev[leafBin]+iphot ];

                    if (par.corrtype == 1 || par.corrtype == 3)
                    {
                        sra  = secondary[photUse].ra;
                        sdec = secondary[photUse].dec;
                    }
                    else
                    {
                        sra  = random_secondary[photUse].ra;
                        sdec = random_secondary[photUse].dec;
                    }


                    double R = gcirc(ra, dec, sra, sdec);

                    // Mpc
                    float Rmpc = R*DA;

                    // convert to comoving?
                    //if (par.comoving) 
                    //  Rmpc = Rmpc*(1+z);


                    // Within our circular radius as well as lower limit
                    // in angular radius?
                    if (Rmpc >= par.rmin && Rmpc <= par.rmax && R > MINIMUM_ANGLE)
                    {
                        seclist.push_back(photUse);
                        radlist.push_back(Rmpc);
                    } // radius within circular radius min/max?

                } // loop over secondaries found

            } // any secondaries found in this leaf?

        } // leaf id is in allowed range?

    } // loop over found leaf ids


}


///////////////////////////////////////////
// bin by color, luminosity, and radius
///////////////////////////////////////////

int
corrobj::write_colorlumrad_pairs(int index, 
				 float z, 
				 float DLum, 
				 vector<int>   &seclist, 
				 vector<float> &radlist)
{

    int nkeep=0;

    secondary_struct *sec;  
    for (int i=0; i<seclist.size(); i++)
    {

        sec = &secondary[seclist[i]];

        // Radii are already verified
        float Rmpc = radlist[i];

        // All fluxes greater than zero?
        if (sec->gflux > 0.0 && sec->rflux > 0.0 && sec->iflux > 0.0)
        {
            float gk,rk,ik;
            int flags;

            float gmr = -2.5*log10(sec->gflux/sec->rflux);
            float rmi = -2.5*log10(sec->rflux/sec->iflux);

            kcorr->kcorr_griflux(z, gmr, rmi, 
                    gk, rk, ik, flags);

            if (flags == 0)
            {

                float kgflux = sec->gflux/gk;
                float krflux = sec->rflux/rk;
                float kiflux = sec->iflux/ik;

                float kgmr = -2.5*log10(kgflux/krflux);
                float lum=0;
                switch (par.lumband) {
                    case 3: 
                        lum = kcorr->knmgy2lumsolar(kiflux, DLum, par.lumband);
                        break;
                    case 2:
                        lum = kcorr->knmgy2lumsolar(krflux, DLum, par.lumband);
                        break;
                    case 1:
                        lum = kcorr->knmgy2lumsolar(kgflux, DLum, par.lumband);
                        break;
                    default: break;
                }

                int cbin = binobj->kgmr_bin(kgmr);
                int lbin = binobj->logl_bin(lum);

                if (cbin != -1 && lbin != -1)
                {
                    fwrite((char *)&index,  sizeof(int),   1, mFptr);

                    fwrite((char *)&Rmpc,   sizeof(float), 1, mFptr);

                    fwrite((char *)&kgflux, sizeof(float), 1, mFptr);
                    fwrite((char *)&krflux, sizeof(float), 1, mFptr);
                    fwrite((char *)&kiflux, sizeof(float), 1, mFptr);

                    nkeep++;
                } // Within bounds

            } // good k-corrections

        } // good fluxes 

    } // seclist

    return(nkeep);

}



/////////////////////////////////////////////////////////////////////////////
// Just get color, luminosity, and radius, demand they are within the bounds,
// and write good ones to a file
/////////////////////////////////////////////////////////////////////////////

void 
corrobj::bin_by_colorlumrad(float z, float DLum, 
			    vector<int> & seclist, vector<float> &radlist)
{

    secondary_struct *sec;  
    for (int i=0; i<seclist.size(); i++)
    {
        if (debug) {
            cout<<"top of bin_by_colorlum loop"<<endl;
            fflush(stdout);
        }
        sec = &secondary[seclist[i]];

        float Rmpc = radlist[i];

        float logRmpc = log10(Rmpc);
        if (debug2) {
            cout<<"logRmpc = "<<logRmpc<<endl;
            fflush(stdout);
        }
        int radBin = (int8) ( (logRmpc-par.logRmin)/par.logBinsize );


        // valid bin number?
        if (radBin >= 0 && radBin < par.nrad)
        {

            if (debug2) {
                cout<<"radbin = "<<radBin<<endl;
                fflush(stdout);
            }
            // All fluxes greater than zero?
            if (sec->gflux > 0.0 && sec->rflux > 0.0 && sec->iflux > 0.0)
            {
                float gk,rk,ik;
                int flags;

                float gmr = -2.5*log10(sec->gflux/sec->rflux);
                float rmi = -2.5*log10(sec->rflux/sec->iflux);

                kcorr->kcorr_griflux(z, gmr, rmi, 
                        gk, rk, ik, flags);

                if (debug2) {
                    cout<<"kcorr flags = "<<flags<<endl;
                    fflush(stdout);
                }
                if (flags == 0)
                {

                    if (debug2) {
                        cout<<gk<<" "<<rk<<" "<<ik<<endl;
                        fflush(stdout);
                    }
                    float kgflux = sec->gflux/gk;
                    float krflux = sec->rflux/rk;
                    float kiflux = sec->iflux/ik;

                    float kgmr = -2.5*log10(kgflux/krflux);

                    // lumband is the band in which we check the luminosity
                    // bounds and output the luminosity values
                    float lum=0;
                    switch (par.lumband) {
                        case 3: 
                            lum=kcorr->knmgy2lumsolar(kiflux,DLum,par.lumband);
                            break;
                        case 2:
                            lum=kcorr->knmgy2lumsolar(krflux,DLum,par.lumband);
                            break;
                        case 1:
                            lum=kcorr->knmgy2lumsolar(kgflux,DLum,par.lumband);
                            break;
                        default: 
                            lum=kcorr->knmgy2lumsolar(kiflux,DLum,3);
                            break;
                    }

                    int cbin = binobj->kgmr_bin(kgmr);
                    int lbin = binobj->logl_bin(lum);

                    if (debug2) {
                        cout<<"  kgmr = "<<kgmr<<endl;
                        cout<<"  cbin = "<<cbin<<endl;
                        cout<<"  lbin = "<<lbin<<endl;
                        fflush(stdout);
                    }

                    if (cbin != -1 && lbin != -1)
                    {

                        float imag = 22.5 - 2.5*log10(sec->iflux);
                        if (debug2) {
                            cout<<imag<<endl;
                            fflush(stdout);
                        }
                        if (imag > max_kept_imag) 
                            max_kept_imag = imag;

                        output.totpairs++;
                        output.rsum[radBin] += Rmpc;

                        output.kgflux[radBin] += kgflux;
                        output.krflux[radBin] += krflux;
                        output.kiflux[radBin] += kiflux;

                        switch (par.output_type)
                        {
                            case 2:
                                output.radcounts[radBin]++;
                                output.radlum[radBin]+= lum;
                                break;
                            case 3:
                                output.counts[cbin][lbin][radBin]++;
                                output.lum[cbin][lbin][radBin] += lum;
                                break;
                            case 1:
                                output.radcounts[radBin]++;
                                break;
                            default: break;
                        }

                    } // within mag limit

                } // good k-corrections

            } // good fluxes 

        }// Good radbin?

    } // seclist

}



///////////////////////////////////////////
// bins just by radius
///////////////////////////////////////////

void 
corrobj::bin_by_rad(vector<int> & seclist, vector<float> &radlist)
{

    for (int i=0; i<seclist.size(); i++)
    {

        float Rmpc = radlist[i];

        float logRmpc = log10(Rmpc);
        int8 radBin = (int8) ( (logRmpc-par.logRmin)/par.logBinsize );


        // valid bin number?
        if (radBin >= 0 && radBin < par.nrad)
        {

            edge_output.totpairs++;
            edge_output.rsum[radBin] += Rmpc;
            edge_output.counts[radBin]++;

        }// Good radbin?

    } // seclist

}

void corrobj::printstuff(int index, int bigStep)
{

    printf(".");
    if ( (index % bigStep) == 0)
    {
        time_t t1 = time(NULL);
        cout << endl << index << "/" << par.nprimary;
        if (par.corrtype == 1) cout << " max_kept_imag: " << max_kept_imag;
        cout << " time: " << (t1-t0) << " sec" << endl;
    }


}



////////////////////////////////////////////////////////////////////////////
// Actually measure the correlation
////////////////////////////////////////////////////////////////////////////


void corrobj::correlate()
{

    int step=100;
    int bigStep=5000;

    cout << endl <<
        "Each dot is " << step << " primaries" << endl;

    //FILE *mFptr = fopen(par.output_file, "w");
    mFptr = fopen(par.output_file, "w");


    if (par.corrtype == 3)
    {
        string index_file = par.output_file;
        index_file += ".index";
        mIndexFptr = fopen(index_file.c_str(), "w");

        // Only have a header for the index file
        cout << "Pairs written to file: " << par.output_file << endl;
        cout << "Writing header to index file: " << index_file << endl;
        //return;
        write_pairindex_header(mIndexFptr);
    }
    else if (par.corrtype == 1)
    {
        max_kept_imag = 0;
        cout << "Writing header for file " << par.output_file << endl;
        write_header(mFptr);
    }
    else
    {
        cout << "Writing header for file " << par.output_file << endl;
        write_edge_header(mFptr);
    }

    t0 = time(NULL);

    int64 npairs=0;
    vector<uint64> idlist;
    vector<int> seclist;
    vector<float> radlist;

    for (int index=0; index < par.nprimary; index++)
    {
        if (debug) {
            cout<<"index = "<<index<<endl;
            fflush(stdout);
        }
        double ra = primary[index].ra;
        double dec = primary[index].dec;
        float z = primary[index].z;
        float DA = primary[index].DA;
        float DLum = DA*(1+z)*(1+z);

        if (debug) {
            cout<<"before intersect"<<endl;
            fflush(stdout);
        }

        // Get triangle list
        intersect(ra, dec, DA, idlist);

        if (debug) {
            cout<<"before seclist"<<endl;
            fflush(stdout);
        }
        // Get list of objects within rmin,rmax
        get_seclist(ra, dec, DA, idlist, seclist, radlist);

        // WARNING: corrtype == 3 isn't actually implemented and I think
        // it is mixed up in definition
        if (par.corrtype == 3)
        {
            // Just writing the pair information
            int tnpairs = write_colorlumrad_pairs(index, z, DLum, seclist, radlist);

            // Now write to index if any pairs were found
            npairs += tnpairs;
            fwrite((char *)&primary[index].index, sizeof(int), 1, mIndexFptr);
            fwrite((char *)&tnpairs, sizeof(int), 1, mIndexFptr);
        }
        else if (par.corrtype == 1)
        {
            if (debug) {
                cout<<"before bin"<<endl;
                fflush(stdout);
            }

            // bin into grid of color, luminosity and radius, or perhaps
            // just radius depending on output_type
            bin_by_colorlumrad(z, DLum, seclist, radlist);
            if (debug) { 
                cout<<"Writing output"<<endl;
                fflush(stdout);
            }

            output.index = primary[index].index;
            write_output(mFptr);
            if (debug) {
                cout<<"Resetting output"<<endl;
                fflush(stdout);
            }
            reset_output();
        }
        else 
        {
            // bin just into a grid of radius
            // This is for edges, we don't care anything about the lum or
            // color of the secondaries because it is only for randoms 
            // secondaries anyway
            bin_by_rad(seclist, radlist);

            edge_output.index = primary[index].index;
            write_edge_output(mFptr);
            reset_edge_output();
        }

        // Print some stuff
        if ( (index % step) == 0 && (index != 0))
            printstuff(index, bigStep);

        if (debug) {
            cout<<"restarting loop"<<endl;
            fflush(stdout);
        }
    }


    if (par.corrtype == 3)
    {
        fclose(mIndexFptr);
        //rewind(mFptr);
        //fprintf(mFptr, "NROWS  = %15lld\n", npairs);
    }


    fclose(mFptr);

    time_t t1 = time(NULL);
    //if(!quiet)
    printf("\nTime: %ld sec\n", (t1-t0));

    if (par.corrtype == 1)
        cout << endl << "Max kept imag: " << max_kept_imag << endl;
    cout << endl << "Done" << endl;

}

