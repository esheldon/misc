#include "kcorr.h"


kcorr_table::kcorr_table() {} // default constructor does nothing
kcorr_table::kcorr_table(char *par_file) { // constructor

    read(par_file);

    sunabsmag.resize(5);
    sunabsmag[0] = 7.73022;
    sunabsmag[1] = 6.05181;
    sunabsmag[2] = 4.96901;
    sunabsmag[3] = 4.66824;
    sunabsmag[4] = 4.54800;

    sunnmgy.resize(5);
    for (int i=0; i<5; i++)
        sunnmgy[i] = pow(10.0, -0.4*sunabsmag[i])*1.e9;

}

void kcorr_table::read(char *file)
{

    FILE *fptr;

    cout << endl << "Reading from kcorr file: " << file << endl;

    fptr = fopen(file, "r");

    // redshift intervals
    fread(&nz, sizeof(int), 1, fptr);
    z.resize(nz);  
    fread((char *)&z[0], sizeof(float), nz, fptr);
    zmin = z[0];
    zmax = z[nz-1];
    zstep = z[1]-z[0];

    // gmr intervals
    fread(&ngmr, sizeof(int), 1, fptr);
    gmr.resize(ngmr);
    fread((char *)&gmr[0], sizeof(float), ngmr, fptr);
    gmrmin = gmr[0];
    gmrmax = gmr[ngmr-1];
    gmrstep = gmr[1]-gmr[0];

    // rmi intervals
    fread(&nrmi, sizeof(int), 1, fptr);
    rmi.resize(nrmi);
    fread((char *)&rmi[0], sizeof(float), nrmi, fptr);
    rmimin = rmi[0];
    rmimax = rmi[nrmi-1];
    rmistep = rmi[1]-rmi[0];

    // bandpasses
    fread(&nband, sizeof(int), 1, fptr);
    bands.resize(nband);

    fread((char *)&bands[0], sizeof(float), nband, fptr);

    // read the kcorr matrix.
    kcorrTable.Allocate(nz, ngmr, nrmi, nband);

    for (int iz=0; iz<nz; iz++)
        for (int igmr=0; igmr<ngmr; igmr++)
            for (int irmi=0; irmi<nrmi; irmi++)
                for (int ib=0; ib<nband; ib++)
                    fread( &kcorrTable[iz][igmr][irmi][ib], sizeof(float), 1, fptr);


    cout << " nz    = " << nz << endl;
    cout << " zmin  = " << zmin << endl;
    cout << " zmax  = " << zmax << endl;
    cout << " zstep = " << zstep << endl;

    cout << " ngmr    = " << ngmr << endl;
    cout << " gmrmin  = " << gmrmin << endl;
    cout << " gmrmax  = " << gmrmax << endl;
    cout << " gmrstep = " << gmrstep << endl;

    cout << " nrmi    = " << nrmi << endl;
    cout << " rmimin  = " << rmimin << endl;
    cout << " rmimax  = " << rmimax << endl;
    cout << " rmistep = " << rmistep << endl;

    cout << " nband   = " << nband << endl;


}




kcorr_interp_struct kcorr_table::new_interp_struct()
{
    kcorr_interp_struct ks;

    ks.iz0=-1;
    ks.iz1=-1;
    ks.igmr0=-1;
    ks.igmr1=-1;
    ks.irmi0=-1;
    ks.irmi1=-1;

    ks.zc=-1;
    ks.gmrc=-1;
    ks.rmic=-1;
    ks.flags=1;

    return(ks);
}

    kcorr_interp_struct 
kcorr_table::kcorr_get_interp_info(float z, float gmr, float rmi)
{

    kcorr_interp_struct ks;

    // default is failulre
    ks.flags = 1;

    if (z < zmin || z > zmax) return(ks);
    if (gmr < gmrmin || gmr > gmrmax) return(ks);
    if (rmi < rmimin || rmi > rmimax) return(ks);

    float fz = (z - zmin)/zstep;
    float fgmr = (gmr - gmrmin)/gmrstep;
    float frmi = (rmi - rmimin)/rmistep;

    // round downward
    ks.iz0 = (int) fz;
    ks.igmr0 = (int) fgmr;
    ks.irmi0 = (int) frmi;


    ks.iz1 = ks.iz0+1;
    if (ks.iz1 > (nz-1)) return(ks);

    ks.igmr1 = ks.igmr0+1;
    if (ks.igmr1 > (ngmr-1)) return(ks);

    ks.irmi1 = ks.irmi0+1;
    if (ks.irmi1 > (nrmi-1)) return(ks);


    // values within the little sub-cube
    ks.zc = fz-ks.iz0;
    ks.gmrc = fgmr - ks.igmr0;
    ks.rmic = frmi - ks.irmi0;


    ks.flags = 0;
    return(ks);

}



float kcorr_table::kcorr_interp(kcorr_interp_struct &ks, int band)
{

    float kmag = 	
        kcorrTable[ks.iz0][ks.igmr0][ks.irmi0][band]*(1 - ks.zc)*(1 - ks.gmrc)*(1 - ks.rmic) +
        kcorrTable[ks.iz1][ks.igmr0][ks.irmi0][band]*ks.zc*(1 - ks.gmrc)*(1 - ks.rmic) +
        kcorrTable[ks.iz0][ks.igmr1][ks.irmi0][band]*(1 - ks.zc)*ks.gmrc*(1 - ks.rmic) +
        kcorrTable[ks.iz0][ks.igmr0][ks.irmi1][band]*(1 - ks.zc)*(1 - ks.gmrc)*ks.rmic +
        kcorrTable[ks.iz1][ks.igmr0][ks.irmi1][band]*ks.zc*(1 - ks.gmrc)*ks.rmic +
        kcorrTable[ks.iz0][ks.igmr1][ks.irmi1][band]*(1 - ks.zc)*ks.gmrc*ks.rmic +
        kcorrTable[ks.iz1][ks.igmr1][ks.irmi0][band]*ks.zc*ks.gmrc*(1 - ks.rmic) +
        kcorrTable[ks.iz1][ks.igmr1][ks.irmi1][band]*ks.zc*ks.gmrc*ks.rmic;

    return(kmag);


}

float kcorr_table::kcorr(float z, float gmr, float rmi, int band)
{

    kcorr_interp_struct ks = kcorr_get_interp_info(z, gmr, rmi);
    if (ks.flags != 0)
        return(BAD_KCORR);

    float kmag = kcorr_interp(ks, band);

    return(kmag);

}

float kcorr_table::kflux(float z, float gmr, float rmi, int band)
{

    float kmag = kcorr(z, gmr, rmi, band);

    if (kmag < MIN_KCORR)
        return(BAD_KCORR);

    // convert to nanomaggies
    float kflux = pow(10.0, -0.4*kmag);

    return(kflux);

}


void kcorr_table::kcorr_griflux(float z, float gmr, float rmi, 
        float &gk, float &rk, float &ik,
        int &flags)
{

    kcorr_interp_struct ks = kcorr_get_interp_info(z, gmr, rmi);
    flags = ks.flags;

    if (flags != 0)
    {
        gk = BAD_KCORR;
        rk = BAD_KCORR;
        ik = BAD_KCORR;
        return;
    }

    gk = kcorr_interp(ks, 1);
    rk = kcorr_interp(ks, 2);
    ik = kcorr_interp(ks, 3);


    if (gk < MIN_KCORR || rk < MIN_KCORR || ik < MIN_KCORR)
    {
        gk = BAD_KCORR;
        rk = BAD_KCORR;
        ik = BAD_KCORR;
        flags = 1;
        return;
    }


    gk = pow(10.0, -0.4*gk);
    rk = pow(10.0, -0.4*rk);
    ik = pow(10.0, -0.4*ik);

}



// Convert angular diameter distance in Mpc to distance modulus 
float kcorr_table::DA2DM(float DA, float z)
{

    // The 6 is for converting Mpc to pc
    float Dlum = DA*(1+z)*(1+z);
    float DM = 5*(log10(Dlum) + 6.0) - 5.0;
    return(DM);

}


// convert k-corrected nanomaggies to solar luminosities (units of 10^10)
float kcorr_table::knmgy2lumsolar(float knmgy, float DLum, int band)
{

    // No conversions needed because all cancels:
    // 1.e6/10 for each Mpc->pc->units of 10 pc
    // 1.e10 for units of 10^10
    // (1.e6/10.0)^2/1.e10 = 1.0
    float lumsolar = knmgy/sunnmgy[band]*DLum*DLum;
    return(lumsolar);
}

// convert k-corrected nanomaggies to log10 solar luminosities
float kcorr_table::knmgy2loglumsolar(float knmgy, float DLum, int band)
{
    float lumsolar = knmgy2lumsolar(knmgy, DLum, band);
    float loglum = 10.0 + log10(lumsolar);
    return(loglum);
}

// convert k-corrected nanomaggies to absolute magnitude 
float kcorr_table::knmgy2absmag(float knmgy, float DM)
{
    // Convert from nmgy to magnitudes 
    float absmag = 22.5 - 2.5*log10(knmgy);

    // Apply distance modulus 
    absmag -= DM;
    return(absmag);
}

// convert absmag to 10^10 lum solar 
float kcorr_table::absmag2lumsolar(float absmag, int band)
{
    float logLumSolar = -0.4*(absmag - sunabsmag[band]);

    // minus to for units of 10^10 
    float lumSolar = pow( 10.0, logLumSolar - 10.0 );

    return(lumSolar);
}

