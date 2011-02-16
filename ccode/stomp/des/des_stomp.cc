#include "des_stomp.h"

DESFootprint::DESFootprint(AngularCoordinate& ang, double weight) {

	SetWeight(weight);

	double* ramin = new double[9];
	double* ramax = new double[9];
	double* decmin = new double[9];
	double* decmax = new double[9];

	ramin[0] = -9.61/60.0;
	ramax[0] = 9.61/60.0;
	decmin[0] = 56.76/60.0;
	decmax[0] = 66.27/60.0;

	ramin[1] = -29.60/60.0;
	ramax[1] = 29.60/60.0;
	decmin[1] = 47.25/60.0;
	decmax[1] = 56.76/60.0;

	ramin[2] = -39.59/60.0;
	ramax[2] = 39.59/60.0;
	decmin[2] = 37.74/60.0;
	decmax[2] = 47.25/60.0;

	ramin[3] = -49.59/60.0;
	ramax[3] = 49.59/60.0;
	decmin[3] = 28.23/60.0;
	decmax[3] = 37.74/60.0;

	ramin[4] = -59.58/60.0;
	ramax[4] = 59.58/60.0;
	decmin[4] = -28.23/60.0;
	decmax[4] = 28.23/60.0;

	ramin[5] = -49.59/60.0;
	ramax[5] = 49.59/60.0;
	decmin[5] = -37.74/60.0;
	decmax[5] = -28.23/60.0;

	ramin[6] = -39.59/60.0;
	ramax[6] = 39.59/60.0;
	decmin[6] = -47.25/60.0;
	decmax[6] = -37.74/60.0;

	ramin[7] = -29.60/60.0;
	ramax[7] = 29.60/60.0;
	decmin[7] = -56.76/60.0;
	decmax[7] = -47.25/60.0;

	ramin[8] = -9.61/60.0;
	ramax[8] = 9.61/60.0;
	decmin[8] = -66.27/60.0;
	decmax[8] = -56.76/60.0;

	poly_.reserve(9);

	for (unsigned long i=0;i<9;++i) {
		AngularVector tmp_ang;

		double tmp_ra = ang.RA();
		double tmp_dec = ang.DEC();

		tmp_ra += ramin[i]*AngularCoordinate::RAMultiplier(decmax[i]);
		tmp_dec += decmax[i];
		tmp_ang.push_back(AngularCoordinate(tmp_ra,tmp_dec,
					AngularCoordinate::Equatorial));

		tmp_ra = ang.RA() + ramin[i]*AngularCoordinate::RAMultiplier(decmin[i]);
		tmp_dec = ang.DEC() + decmin[i];
		tmp_ang.push_back(AngularCoordinate(tmp_ra,tmp_dec,
					AngularCoordinate::Equatorial));

		tmp_ra = ang.RA() + ramax[i]*AngularCoordinate::RAMultiplier(decmin[i]);
		tmp_dec = ang.DEC() + decmin[i];
		tmp_ang.push_back(AngularCoordinate(tmp_ra,tmp_dec,
					AngularCoordinate::Equatorial));

		tmp_ra = ang.RA() + ramax[i]*AngularCoordinate::RAMultiplier(decmax[i]);
		tmp_dec = ang.DEC() + decmax[i];
		tmp_ang.push_back(AngularCoordinate(tmp_ra,tmp_dec,
					AngularCoordinate::Equatorial));

		poly_.push_back(PolygonBound(tmp_ang,weight));
	}

	delete[] ramin;
	delete[] ramax;
	delete[] decmin;
	delete[] decmax;

	FindArea();
	FindAngularBounds();
	SetMaxResolution();
}

DESFootprint::~DESFootprint() {
	Clear();
	poly_.clear();
}

bool DESFootprint::CheckPoint(AngularCoordinate& ang) {

	for (PolygonIterator iter=poly_.begin();iter!=poly_.end();++iter) {
		if (iter->CheckPoint(ang)) {
			return(true);
		}
	}

	return false;
}

bool DESFootprint::FindAngularBounds() {

	double lammin = 100.0, lammax = -100.0, etamin = 200.0, etamax = -200.0;

	for (PolygonIterator iter=poly_.begin();iter!=poly_.end();++iter) {
		iter->FindAngularBounds();

		if (iter->LambdaMin() < lammin) lammin = iter->LambdaMin();
		if (iter->LambdaMax() > lammax) lammax = iter->LambdaMax();
		if (iter->EtaMin() < etamin) etamin = iter->EtaMin();
		if (iter->EtaMax() > etamax) etamax = iter->EtaMax();
	}

	SetAngularBounds(lammin,lammax,etamin,etamax);

	return true;
}

bool DESFootprint::FindArea() {

	double area = 0.0;

	for (PolygonIterator iter=poly_.begin();iter!=poly_.end();++iter) {
		iter->FindArea();
		area += iter->Area();
	}

	SetArea(area);

	return true;
}


