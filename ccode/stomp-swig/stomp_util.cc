#include "stomp_util.h"

namespace Stomp {

// Define our Stomp class constants.  See stomp_util.h for their meanings.
const double Stomp::Pi = 2.0*asin(1.0);
const double Stomp::DegToRad = Stomp::Pi/180.0;
const double Stomp::RadToDeg = 180.0/Stomp::Pi;
const double Stomp::StradToDeg = 180.0*180.0/(Stomp::Pi*Stomp::Pi);
const uint32_t Stomp::Nx0 = 36;
const uint32_t Stomp::Ny0 = 13;
const double Stomp::EtaOffSet = 91.25;
const double Stomp::SurveyCenterRA = 185.0;
const double Stomp::SurveyCenterDEC = 32.5;
const double Stomp::Node = Stomp::DegToRad*(Stomp::SurveyCenterRA-90.0);
const double Stomp::EtaPole = Stomp::DegToRad*Stomp::SurveyCenterDEC;
const uint8_t Stomp::HPixLevel = 2; // 4 in the old parlance.
const uint8_t Stomp::MaxPixelLevel = 15; // 2^15 = 32768
const uint16_t Stomp::HPixResolution = 1 << Stomp::HPixLevel;
const uint16_t Stomp::MaxPixelResolution = 1 << Stomp::MaxPixelLevel;  // 32768
const uint8_t Stomp::ResolutionLevels = Stomp::MaxPixelLevel -
						Stomp::HPixLevel + 1;
const double Stomp::HPixArea = 4.0*Stomp::Pi*Stomp::StradToDeg/
						(Stomp::HPixResolution*
						 Stomp::HPixResolution*
						 Stomp::Nx0*Stomp::Ny0);
const uint32_t Stomp::MaxPixnum = Stomp::Nx0*Stomp::Ny0*2048*2048;
const uint32_t Stomp::MaxSuperpixnum = Stomp::Nx0*Stomp::Ny0*
						Stomp::HPixResolution*
						Stomp::HPixResolution;

// Define our default WMAP5 flat, LCDM cosmology.
double Cosmology::omega_m_ = 0.2736;
double Cosmology::h_ = 0.705;
const double Cosmology::AA_ = 1.718;
const double Cosmology::BB_ = 0.315;
double Cosmology::a_ = Cosmology::AA_*Cosmology::omega_m_;
double Cosmology::b_ = Cosmology::BB_*sqrt(Cosmology::omega_m_);

AngularCoordinate::AngularCoordinate(double theta, double phi,
				     Sphere sphere) {
  switch (sphere) {
  case Survey:
    if (Stomp::DoubleGE(theta, 90.0)) theta = 90.0;
    if (Stomp::DoubleLE(theta, -90.0)) theta = -90.0;
    if (phi > 180.0) phi -= 360.0;
    if (phi < -180.0) phi += 360.0;

    theta *= Stomp::DegToRad;
    phi *= Stomp::DegToRad;

    us_x_ = -1.0*sin(theta);
    us_y_ = cos(theta)*cos(phi+Stomp::EtaPole);
    us_z_ = cos(theta)*sin(phi+Stomp::EtaPole);
    break;
  case Equatorial:
    if (Stomp::DoubleGE(phi, 90.0)) phi = 90.0;
    if (Stomp::DoubleLE(phi, -90.0)) phi = -90.0;
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0) theta += 360.0;

    theta *= Stomp::DegToRad;
    phi *= Stomp::DegToRad;

    us_x_ = cos(theta-Stomp::Node)*cos(phi);
    us_y_ = sin(theta-Stomp::Node)*cos(phi);
    us_z_ = sin(phi);
    break;
  case Galactic:
    if (Stomp::DoubleGE(phi, 90.0)) phi = 90.0;
    if (Stomp::DoubleLE(phi, -90.0)) phi = -90.0;
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0) theta += 360.0;

    double ra, dec;
    GalacticToEquatorial(theta, phi, ra, dec);

    ra *= Stomp::DegToRad;
    dec *= Stomp::DegToRad;

    us_x_ = cos(ra-Stomp::Node)*cos(dec);
    us_y_ = sin(ra-Stomp::Node)*cos(dec);
    us_z_ = sin(dec);
    break;
  }
}

AngularCoordinate::AngularCoordinate(double unit_sphere_x,
				     double unit_sphere_y,
				     double unit_sphere_z) {
  double r_norm = sqrt(unit_sphere_x*unit_sphere_x +
		       unit_sphere_y*unit_sphere_y +
		       unit_sphere_z*unit_sphere_z);

  us_x_ = unit_sphere_x/r_norm;
  us_y_ = unit_sphere_y/r_norm;
  us_z_ = unit_sphere_z/r_norm;
}

AngularCoordinate::~AngularCoordinate() {
  us_x_ = 0.0;
  us_y_ = 0.0;
  us_z_ = 0.0;
}

void AngularCoordinate::SetSurveyCoordinates(double lambda, double eta) {
  if (Stomp::DoubleGE(lambda, 90.0)) lambda = 90.0;
  if (Stomp::DoubleLE(lambda, -90.0)) lambda = -90.0;
  if (eta > 180.0) eta -= 360.0;
  if (eta < -180.0) eta += 360.0;

  eta *= Stomp::DegToRad;
  lambda *= Stomp::DegToRad;

  us_x_ = -1.0*sin(lambda);
  us_y_ = cos(lambda)*cos(eta+Stomp::EtaPole);
  us_z_ = cos(lambda)*sin(eta+Stomp::EtaPole);
}

void AngularCoordinate::SetEquatorialCoordinates(double ra, double dec) {
  if (Stomp::DoubleGE(dec, 90.0)) dec = 90.0;
  if (Stomp::DoubleLE(dec, -90.0)) dec = -90.0;
  if (ra > 360.0) ra -= 360.0;
  if (ra < 0.0) ra += 360.0;

  ra *= Stomp::DegToRad;
  dec *= Stomp::DegToRad;

  us_x_ = cos(ra-Stomp::Node)*cos(dec);
  us_y_ = sin(ra-Stomp::Node)*cos(dec);
  us_z_ = sin(dec);
}

void AngularCoordinate::SetGalacticCoordinates(double gal_lon, double gal_lat) {
  if (Stomp::DoubleGE(gal_lat, 90.0)) gal_lat = 90.0;
  if (Stomp::DoubleLE(gal_lat, -90.0)) gal_lat = -90.0;
  if (gal_lon > 360.0) gal_lon -= 360.0;
  if (gal_lon < 0.0) gal_lon += 360.0;

  double ra, dec;
  GalacticToEquatorial(gal_lon, gal_lat, ra, dec);

  ra *= Stomp::DegToRad;
  dec *= Stomp::DegToRad;

  us_x_ = cos(ra-Stomp::Node)*cos(dec);
  us_y_ = sin(ra-Stomp::Node)*cos(dec);
  us_z_ = sin(dec);
}

void AngularCoordinate::SetUnitSphereCoordinates(double unit_sphere_x,
						 double unit_sphere_y,
						 double unit_sphere_z) {
  double r_norm = sqrt(unit_sphere_x*unit_sphere_x +
		       unit_sphere_y*unit_sphere_y +
		       unit_sphere_z*unit_sphere_z);

  us_x_ = unit_sphere_x/r_norm;
  us_y_ = unit_sphere_y/r_norm;
  us_z_ = unit_sphere_z/r_norm;
}

void AngularCoordinate::GreatCircle(AngularCoordinate& ang,
				    AngularCoordinate& great_circle) {
  great_circle = CrossProduct(ang);
}

double AngularCoordinate::PositionAngle(AngularCoordinate& ang, Sphere sphere) {
  return Stomp::RadToDeg*atan2(SinPositionAngle(ang, sphere),
			       CosPositionAngle(ang, sphere));
}

double AngularCoordinate::PositionAngle(Pixel& pix, Sphere sphere) {
  return Stomp::RadToDeg*atan2(SinPositionAngle(pix, sphere),
			       CosPositionAngle(pix, sphere));
}

double AngularCoordinate::CosPositionAngle(AngularCoordinate& ang,
					   Sphere sphere) {
  double theta = 0.0, phi = 0.0;
  double ang_theta = 0.0, ang_phi = 0.0;
  switch (sphere) {
  case Survey:
    theta = Eta()*Stomp::DegToRad;
    phi = Lambda()*Stomp::DegToRad;

    ang_theta = ang.Eta()*Stomp::DegToRad;
    ang_phi = ang.Lambda()*Stomp::DegToRad;
    break;
  case Equatorial:
    theta = RA()*Stomp::DegToRad;
    phi = DEC()*Stomp::DegToRad;

    ang_theta = ang.RA()*Stomp::DegToRad;
    ang_phi = ang.DEC()*Stomp::DegToRad;
    break;
  case Galactic:
    theta = GalLon()*Stomp::DegToRad;
    phi = GalLat()*Stomp::DegToRad;

    ang_theta = ang.GalLon()*Stomp::DegToRad;
    ang_phi = ang.GalLat()*Stomp::DegToRad;
    break;
  }

  return cos(phi)*tan(ang_phi) - sin(phi)*cos(ang_theta - theta);
}

double AngularCoordinate::CosPositionAngle(Pixel& pix, Sphere sphere) {
  double theta = 0.0, phi = 0.0;
  double pix_theta = 0.0, pix_phi = 0.0;
  switch (sphere) {
  case Survey:
    theta = Eta()*Stomp::DegToRad;
    phi = Lambda()*Stomp::DegToRad;

    pix_theta = pix.Eta()*Stomp::DegToRad;
    pix_phi = pix.Lambda()*Stomp::DegToRad;
    break;
  case Equatorial:
    theta = RA()*Stomp::DegToRad;
    phi = DEC()*Stomp::DegToRad;

    pix_theta = pix.RA()*Stomp::DegToRad;
    pix_phi = pix.DEC()*Stomp::DegToRad;
    break;
  case Galactic:
    theta = GalLon()*Stomp::DegToRad;
    phi = GalLat()*Stomp::DegToRad;

    pix_theta = pix.GalLon()*Stomp::DegToRad;
    pix_phi = pix.GalLat()*Stomp::DegToRad;
    break;
  }

  return cos(phi)*tan(pix_phi) - sin(phi)*cos(pix_theta - theta);
}

double AngularCoordinate::SinPositionAngle(AngularCoordinate& ang,
					   Sphere sphere) {
  double theta = 0.0;
  double ang_theta = 0.0;
  switch (sphere) {
  case Survey:
    theta = Eta()*Stomp::DegToRad;
    ang_theta = ang.Eta()*Stomp::DegToRad;
    break;
  case Equatorial:
    theta = RA()*Stomp::DegToRad;
    ang_theta = ang.RA()*Stomp::DegToRad;
    break;
  case Galactic:
    theta = GalLon()*Stomp::DegToRad;
    ang_theta = ang.GalLon()*Stomp::DegToRad;
    break;
  }

  return sin(ang_theta - theta);
}

double AngularCoordinate::SinPositionAngle(Pixel& pix, Sphere sphere) {
  double theta = 0.0;
  double pix_theta = 0.0;
  switch (sphere) {
  case Survey:
    theta = Eta()*Stomp::DegToRad;
    pix_theta = pix.Eta()*Stomp::DegToRad;
    break;
  case Equatorial:
    theta = RA()*Stomp::DegToRad;
    pix_theta = pix.RA()*Stomp::DegToRad;
    break;
  case Galactic:
    theta = GalLon()*Stomp::DegToRad;
    pix_theta = pix.GalLon()*Stomp::DegToRad;
    break;
  }

  return sin(pix_theta - theta);
}

void AngularCoordinate::Rotate(AngularCoordinate& fixed_ang,
			       double rotation_angle) {
  double cos_theta = cos(rotation_angle*Stomp::DegToRad);
  double sin_theta = sin(rotation_angle*Stomp::DegToRad);

  double new_x = us_x_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereX()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereX()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereX()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereY()*us_z_ - fixed_ang.UnitSphereZ()*us_y_)*sin_theta;

  double new_y = us_y_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereY()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereY()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereY()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereZ()*us_x_ - fixed_ang.UnitSphereX()*us_z_)*sin_theta;

  double new_z = us_z_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereX()*us_y_ - fixed_ang.UnitSphereY()*us_x_)*sin_theta;

  SetUnitSphereCoordinates(new_x, new_y, new_z);
}

void AngularCoordinate::Rotate(AngularCoordinate& fixed_ang,
			       double rotation_angle,
			       AngularCoordinate& rotated_ang) {
  double cos_theta = cos(rotation_angle*Stomp::DegToRad);
  double sin_theta = sin(rotation_angle*Stomp::DegToRad);

  double new_x = us_x_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereX()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereX()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereX()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereY()*us_z_ - fixed_ang.UnitSphereZ()*us_y_)*sin_theta;

  double new_y = us_y_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereY()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereY()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereY()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereZ()*us_x_ - fixed_ang.UnitSphereX()*us_z_)*sin_theta;

  double new_z = us_z_*cos_theta + (1.0 - cos_theta)*
    (fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereX()*us_x_ +
     fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereY()*us_y_ +
     fixed_ang.UnitSphereZ()*fixed_ang.UnitSphereZ()*us_z_) +
    (fixed_ang.UnitSphereX()*us_y_ - fixed_ang.UnitSphereY()*us_x_)*sin_theta;

  rotated_ang.SetUnitSphereCoordinates(new_x, new_y, new_z);
}

void AngularCoordinate::GalacticToSurvey(double gal_lon, double gal_lat,
                                         double& lambda, double& eta) {
  double ra, dec;

  GalacticToEquatorial(gal_lon,gal_lat,ra,dec);
  EquatorialToSurvey(ra,dec,lambda,eta);
}

void AngularCoordinate::SurveyToGalactic(double lambda, double eta,
                                         double& gal_lon, double& gal_lat) {
  double ra, dec;

  SurveyToEquatorial(lambda, eta, ra, dec);
  EquatorialToGalactic(ra,dec, gal_lon, gal_lat);
  if (gal_lon < 0.0) gal_lon += 360.0;
  if (gal_lon > 360.0) gal_lon -= 360.0;
}

void AngularCoordinate::SurveyToEquatorial(double lambda, double eta,
                                           double& ra, double& dec) {
  lambda *= Stomp::DegToRad;
  eta *= Stomp::DegToRad;

  double x = -1.0*sin(lambda);
  double y = cos(lambda)*cos(eta+Stomp::EtaPole);
  double z = cos(lambda)*sin(eta+Stomp::EtaPole);

  ra = (atan2(y,x) + Stomp::Node)*Stomp::RadToDeg;
  if (ra < 0.0) ra += 360.0;
  if (ra > 360.0) ra -= 360.0;
  dec = asin(z)*Stomp::RadToDeg;
}

void AngularCoordinate::SurveyToXYZ(double lambda, double eta,
                                    double& x, double& y, double& z) {
  lambda *= Stomp::DegToRad;
  eta *= Stomp::DegToRad;

  x = -1.0*sin(lambda);
  y = cos(lambda)*cos(eta+Stomp::EtaPole);
  z = cos(lambda)*sin(eta+Stomp::EtaPole);
}

void AngularCoordinate::EquatorialToSurvey(double ra, double dec,
                                           double& lambda, double& eta) {
  ra *= Stomp::DegToRad;
  dec *= Stomp::DegToRad;

  double x = cos(ra-Stomp::Node)*cos(dec);
  double y = sin(ra-Stomp::Node)*cos(dec);
  double z = sin(dec);

  lambda = -1.0*asin(x)*Stomp::RadToDeg;
  eta = (atan2(z,y) - Stomp::EtaPole)*Stomp::RadToDeg;
  if (eta < -180.0) eta += 360.0;
  if (eta > 180.0) eta -= 360.0;
}

void AngularCoordinate::EquatorialToXYZ(double ra, double dec,
                                        double& x, double& y, double& z) {
  ra *= Stomp::DegToRad;
  dec *= Stomp::DegToRad;

  x = cos(ra-Stomp::Node)*cos(dec);
  y = sin(ra-Stomp::Node)*cos(dec);
  z = sin(dec);
}

void AngularCoordinate::EquatorialToGalactic(double ra, double dec,
                                             double& gal_lon, double& gal_lat) {
  double g_psi = 0.57477043300;
  double stheta = 0.88998808748;
  double ctheta = 0.45598377618;
  double g_phi = 4.9368292465;

  double a = ra*Stomp::DegToRad - g_phi;
  double b = dec*Stomp::DegToRad;

  double sb = sin(b);
  double cb = cos(b);
  double cbsa = cb*sin(a);

  b = -1.0*stheta*cbsa + ctheta*sb;
  if (b > 1.0) b = 1.0;

  double bo = asin(b)*Stomp::RadToDeg;

  a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

  double ao = (a+g_psi+4.0*Stomp::Pi)*Stomp::RadToDeg;

  while (ao > 360.0) ao -= 360.0;

  gal_lon = ao;
  gal_lat = bo;
}

void AngularCoordinate::GalacticToEquatorial(double gal_lon, double gal_lat,
                                             double& ra, double& dec) {
  double g_psi = 4.9368292465;
  double stheta = -0.88998808748;
  double ctheta = 0.45598377618;
  double g_phi = 0.57477043300;

  double a = gal_lon*Stomp::DegToRad - g_phi;
  double b = gal_lat*Stomp::DegToRad;

  double sb = sin(b);
  double cb = cos(b);
  double cbsa = cb*sin(a);

  b = -1.0*stheta*cbsa + ctheta*sb;
  if (b > 1.0) b = 1.0;

  double bo = asin(b)*Stomp::RadToDeg;

  a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

  double ao = (a+g_psi+4.0*Stomp::Pi)*Stomp::RadToDeg;
  while (ao > 360.0) ao -= 360.0;

  ra = ao;
  dec = bo;
}

void AngularCoordinate::GalacticToXYZ(double gal_lon, double gal_lat,
                                      double& x, double& y, double& z) {
  double ra, dec;
  GalacticToEquatorial(gal_lat, gal_lon, ra, dec);
  ra *= Stomp::DegToRad;
  dec *= Stomp::DegToRad;

  x = cos(ra-Stomp::Node)*cos(dec);
  y = sin(ra-Stomp::Node)*cos(dec);
  z = sin(dec);
}

WeightedAngularCoordinate::WeightedAngularCoordinate() {
  weight_ = 0.0;
}

WeightedAngularCoordinate::WeightedAngularCoordinate(double theta,
						     double phi,
						     double weight,
						     Sphere sphere) {
  switch (sphere) {
  case Survey:
    SetSurveyCoordinates(theta, phi);
    break;
  case Equatorial:
    SetEquatorialCoordinates(theta, phi);
    break;
  case Galactic:
    SetGalacticCoordinates(theta, phi);
    break;
  }

  weight_ = weight;
}

WeightedAngularCoordinate::WeightedAngularCoordinate(double unit_sphere_x,
						     double unit_sphere_y,
						     double unit_sphere_z,
						     double weight) {
  SetUnitSphereCoordinates(unit_sphere_x, unit_sphere_y, unit_sphere_z);

  weight_ = weight;
}

WeightedAngularCoordinate::~WeightedAngularCoordinate() {
  weight_ = 0.0;
}

AngularCorrelation::AngularCorrelation(double theta_min, double theta_max,
				       double bins_per_decade,
				       bool assign_resolutions) {
  double unit_double = floor(log10(theta_min))*bins_per_decade;
  double theta = pow(10.0, unit_double/bins_per_decade);

  while (theta < theta_max) {
    if (Stomp::DoubleGE(theta, theta_min) && (theta < theta_max)) {
      AngularBin thetabin;
      thetabin.SetThetaMin(theta);
      thetabin.SetThetaMax(pow(10.0,(unit_double+1.0)/bins_per_decade));
      thetabin.SetTheta(pow(10.0,0.5*(log10(thetabin.ThetaMin())+
				      log10(thetabin.ThetaMax()))));
      thetabin_.push_back(thetabin);
    }
    unit_double += 1.0;
    theta = pow(10.0,unit_double/bins_per_decade);
  }

  theta_min_ = thetabin_[0].ThetaMin();
  sin2theta_min_ = thetabin_[0].Sin2ThetaMin();
  theta_max_ = thetabin_[thetabin_.size()-1].ThetaMax();
  sin2theta_max_ = thetabin_[thetabin_.size()-1].Sin2ThetaMax();

  if (assign_resolutions) {
    AssignBinResolutions();
    theta_pixel_begin_ = thetabin_.begin();
    theta_pair_begin_ = thetabin_.begin();
    theta_pair_end_ = thetabin_.begin();
  } else {
    min_resolution_ = Stomp::HPixResolution;
    max_resolution_ = Stomp::HPixResolution;
    theta_pixel_begin_ = thetabin_.end();
    theta_pair_begin_ = thetabin_.begin();
    theta_pair_end_ = thetabin_.end();
  }
}

AngularCorrelation::AngularCorrelation(uint32_t n_bins,
				       double theta_min, double theta_max,
				       bool assign_resolutions) {
  double dtheta = (theta_max - theta_min)/n_bins;

  for (uint32_t i=0;i<n_bins;i++) {
    AngularBin thetabin;
    thetabin.SetThetaMin(theta_min + i*dtheta);
    thetabin.SetThetaMin(theta_min + (i+1)*dtheta);
    thetabin.SetTheta(0.5*(thetabin.ThetaMin()+thetabin.ThetaMax()));
    thetabin_.push_back(thetabin);
  }

  theta_min_ = thetabin_[0].ThetaMin();
  sin2theta_min_ = thetabin_[0].Sin2ThetaMin();
  theta_max_ = thetabin_[n_bins-1].ThetaMax();
  sin2theta_max_ = thetabin_[n_bins-1].Sin2ThetaMax();

  if (assign_resolutions) {
    AssignBinResolutions();
    theta_pixel_begin_ = thetabin_.begin();
    theta_pair_begin_ = thetabin_.begin();
    theta_pair_end_ = thetabin_.begin();
  } else {
    min_resolution_ = Stomp::HPixResolution;
    max_resolution_ = Stomp::HPixResolution;
    theta_pixel_begin_ = thetabin_.end();
    theta_pair_begin_ = thetabin_.begin();
    theta_pair_end_ = thetabin_.end();
  }
}

void AngularCorrelation::AssignBinResolutions(double lammin, double lammax,
					      uint16_t min_resolution) {
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;

  if (lammin < -70.0) {
    std::cout << "Resetting minimum lambda value to -70.0...\n";
    lammin = -70.0;
  }
  if (lammax > 70.0) {
    std::cout << "Resetting maximum lambda value to 70.0...\n";
    lammax = 70.0;
  }

  AngularCoordinate min_ang(lammin,0.0,AngularCoordinate::Survey);
  AngularCoordinate max_ang(lammax,0.0,AngularCoordinate::Survey);

  for (ThetaIterator iter=thetabin_.begin();iter!=thetabin_.end();++iter) {
    uint16_t pixel_resolution = Stomp::HPixResolution;

    uint32_t ny_req = 1000000000, ny_min, ny_max;
    uint32_t small_good = 0, eta_good = 0;
    Pixel tmp_pix, tmp2_pix;

    while (((small_good < ny_req) || (eta_good < ny_req)) &&
	   (pixel_resolution <= min_resolution/2)) {

      small_good = eta_good = 0;
      pixel_resolution <<= 1;

      tmp_pix.SetResolution(pixel_resolution);
      tmp2_pix.SetResolution(pixel_resolution);

      tmp_pix.SetPixnumFromAng(min_ang);
      ny_max = tmp_pix.PixelY();

      tmp_pix.SetPixnumFromAng(max_ang);
      ny_min = tmp_pix.PixelY();

      ny_req = ny_max - ny_min;

      tmp2_pix = tmp_pix;
      for (uint32_t y=ny_min+1,x=tmp2_pix.PixelX();y<=ny_max;y++) {
	tmp_pix.SetPixnumFromXY(x+1,y);
	double costheta =
	  tmp_pix.UnitSphereX()*tmp2_pix.UnitSphereX() +
	  tmp_pix.UnitSphereY()*tmp2_pix.UnitSphereY() +
	  tmp_pix.UnitSphereZ()*tmp2_pix.UnitSphereZ();
	if (1.0 - costheta*costheta < iter->Sin2ThetaMax()) eta_good++;

	tmp_pix.SetPixnumFromXY(x,y);
	costheta =
	  tmp_pix.UnitSphereX()*tmp2_pix.UnitSphereX() +
	  tmp_pix.UnitSphereY()*tmp2_pix.UnitSphereY() +
	  tmp_pix.UnitSphereZ()*tmp2_pix.UnitSphereZ();
	if (1.0 - costheta*costheta < iter->Sin2ThetaMax()) small_good++;

	tmp2_pix = tmp_pix;
      }
    }
    // std::cout << pixel_resolution << ": " << iter->ThetaMin() <<
    // " - " << iter->ThetaMax() << "\n";
    iter->SetResolution(pixel_resolution);

    if (pixel_resolution < min_resolution_) min_resolution_ = pixel_resolution;
    if (pixel_resolution > max_resolution_) max_resolution_ = pixel_resolution;
  }
}

void AngularCorrelation::SetMaxResolution(uint16_t resolution) {
  max_resolution_ = resolution;
  theta_pair_begin_ = thetabin_.begin();
  for (ThetaIterator iter=thetabin_.begin();iter!=thetabin_.end();++iter) {
    if (iter->Resolution() > max_resolution_) {
      iter->SetResolution(0);
      ++theta_pixel_begin_;
      ++theta_pair_end_;
    }
  }
}

void AngularCorrelation::SetMinResolution(uint16_t resolution) {
  min_resolution_ = resolution;
  for (ThetaIterator iter=theta_pixel_begin_;iter!=thetabin_.end();++iter) {
    if (iter->Resolution() < min_resolution_) {
      iter->SetResolution(min_resolution_);
    }
  }
}

void AngularCorrelation::FindAutoCorrelation(Map& stomp_map,
					     WAngularVector& galaxy,
					     uint8_t random_iterations) {
  FindPixelAutoCorrelation(stomp_map, galaxy);
  FindPairAutoCorrelation(stomp_map, galaxy, random_iterations);
}

void AngularCorrelation::FindCrossCorrelation(Map& stomp_map,
					      WAngularVector& galaxy_a,
					      WAngularVector& galaxy_b,
					      uint8_t random_iterations) {
  FindPixelCrossCorrelation(stomp_map, galaxy_a, galaxy_b);
  FindPairCrossCorrelation(stomp_map, galaxy_a, galaxy_b, random_iterations);
}

void AngularCorrelation::FindAutoCorrelationWithRegions(Map& stomp_map,
							WAngularVector& gal,
							uint8_t random_iter,
							uint16_t n_regions) {
  if (n_regions == 0) n_regions = static_cast<uint16_t>(2*thetabin_.size());
  std::cout << "Regionating with " << n_regions << " regions...\n";
  uint16_t n_true_regions = stomp_map.InitializeRegions(n_regions);
  if (n_true_regions != n_regions) {
    std::cout << "Splitting into " << n_true_regions << " rather than " <<
      n_regions << "...\n";
    n_regions = n_true_regions;
  }

  std::cout << "Regionated at " << stomp_map.RegionResolution() << "...\n";
  for (ThetaIterator iter=Begin();iter!=End();++iter)
    iter->InitializeRegions(n_regions);
  SetMinResolution(stomp_map.RegionResolution());

  std::cout << "Auto-correlating with pixels...\n";
  FindPixelAutoCorrelation(stomp_map, gal);
  std::cout << "Auto-correlating with pairs...\n";
  FindPairAutoCorrelation(stomp_map, gal, random_iter);
}

void AngularCorrelation::FindCrossCorrelationWithRegions(Map& stomp_map,
							 WAngularVector& gal_a,
							 WAngularVector& gal_b,
							 uint8_t random_iter,
							 uint16_t n_regions) {
  if (n_regions == 0) n_regions = static_cast<uint16_t>(2*thetabin_.size());
  uint16_t n_true_regions = stomp_map.InitializeRegions(n_regions);
  if (n_true_regions != n_regions) {
    std::cout << "Splitting into " << n_true_regions << " rather than " <<
      n_regions << "...\n";
    n_regions = n_true_regions;
  }

  std::cout << "Regionated at " << stomp_map.RegionResolution() << "...\n";
  for (ThetaIterator iter=Begin();iter!=End();++iter)
    iter->InitializeRegions(n_regions);
  SetMinResolution(stomp_map.RegionResolution());

  FindPixelCrossCorrelation(stomp_map, gal_a, gal_b);
  FindPairCrossCorrelation(stomp_map, gal_a, gal_b, random_iter);
}

void AngularCorrelation::FindPixelAutoCorrelation(Map& stomp_map,
						  WAngularVector& galaxy) {
  ScalarMap* scalar_map = new ScalarMap(stomp_map, max_resolution_,
					ScalarMap::DensityField);
  if (stomp_map.NRegion() > 0) {
    scalar_map->InitializeRegions(stomp_map);
  }

  uint32_t n_galaxy = 0;
  for (WAngularIterator iter=galaxy.begin();iter!=galaxy.end();++iter)
    if (scalar_map->AddToMap(*iter)) n_galaxy++;
  if (n_galaxy != galaxy.size())
    std::cout << "WARNING: Failed to place " << galaxy.size() - n_galaxy <<
      "/" << galaxy.size() << " objects into map.\n";

  FindPixelAutoCorrelation(*scalar_map);

  delete scalar_map;
}

void AngularCorrelation::FindPixelAutoCorrelation(ScalarMap& stomp_map) {
  for (ThetaIterator iter=Begin(stomp_map.Resolution());
       iter!=End(stomp_map.Resolution());++iter) {
    if (stomp_map.NRegion() > 0) {
      std::cout << "\tAuto-correlating with regions at " <<
	stomp_map.Resolution() << "...\n";
      stomp_map.AutoCorrelateWithRegions(iter);
    } else {
      stomp_map.AutoCorrelate(iter);
    }
  }

  for (uint16_t resolution=stomp_map.Resolution()/2;
       resolution>=min_resolution_;resolution/=2) {
    ScalarMap* sub_scalar_map =
      new ScalarMap(stomp_map,resolution);
    if (stomp_map.NRegion() > 0) sub_scalar_map->InitializeRegions(stomp_map);
    for (ThetaIterator iter=Begin(resolution);iter!=End(resolution);++iter) {
      if (stomp_map.NRegion() > 0) {
	std::cout << "\tAuto-correlating with regions at " <<
	  sub_scalar_map->Resolution() << "...\n";
	sub_scalar_map->AutoCorrelateWithRegions(iter);
      } else {
	sub_scalar_map->AutoCorrelate(iter);
      }
    }
    delete sub_scalar_map;
  }
}

void AngularCorrelation::FindPixelCrossCorrelation(Map& stomp_map,
						   WAngularVector& galaxy_a,
						   WAngularVector& galaxy_b) {
  ScalarMap* scalar_map_a = new ScalarMap(stomp_map, max_resolution_,
					  ScalarMap::DensityField);
  ScalarMap* scalar_map_b = new ScalarMap(stomp_map, max_resolution_,
					  ScalarMap::DensityField);
  if (stomp_map.NRegion() > 0) {
    scalar_map_a->InitializeRegions(stomp_map);
    scalar_map_b->InitializeRegions(stomp_map);
  }

  uint32_t n_galaxy = 0;
  for (WAngularIterator iter=galaxy_a.begin();iter!=galaxy_a.end();++iter)
    if (scalar_map_a->AddToMap(*iter)) n_galaxy++;
  if (n_galaxy != galaxy_a.size())
    std::cout << "WARNING: Failed to place " << galaxy_a.size() - n_galaxy <<
      "/" << galaxy_a.size() << " objects into map.\n";

  n_galaxy = 0;
  for (WAngularIterator iter=galaxy_b.begin();iter!=galaxy_b.end();++iter)
    if (scalar_map_b->AddToMap(*iter)) n_galaxy++;
  if (n_galaxy != galaxy_b.size())
    std::cout << "WARNING: Failed to place " << galaxy_b.size() - n_galaxy <<
      "/" << galaxy_b.size() << " objects into map.\n";

  FindPixelCrossCorrelation(*scalar_map_a, *scalar_map_b);

  delete scalar_map_a;
  delete scalar_map_b;
}

void AngularCorrelation::FindPixelCrossCorrelation(ScalarMap& map_a,
						   ScalarMap& map_b) {
  if (map_a.Resolution() != map_b.Resolution()) {
    std::cout << "Incompatible density map resolutions.  Exiting!\n";
    exit(1);
  }

  for (ThetaIterator iter=Begin(map_a.Resolution());
       iter!=End(map_a.Resolution());++iter) {
    if (map_a.NRegion() > 0) {
      map_a.CrossCorrelateWithRegions(map_b, iter);
    } else {
      map_a.CrossCorrelate(map_b, iter);
    }
  }

  for (uint16_t resolution=map_a.Resolution()/2;
       resolution>=min_resolution_;resolution/=2) {
    ScalarMap* sub_map_a = new ScalarMap(map_a, resolution);
    ScalarMap* sub_map_b = new ScalarMap(map_b, resolution);

    if (map_a.NRegion() > 0) {
      sub_map_a->InitializeRegions(map_a);
      sub_map_b->InitializeRegions(map_a);
    }

    for (ThetaIterator iter=Begin(resolution);iter!=End(resolution);++iter) {
      if (map_a.NRegion() > 0) {
	sub_map_a->CrossCorrelateWithRegions(*sub_map_b, iter);
      } else {
	sub_map_a->CrossCorrelate(*sub_map_b, iter);
      }
    }
    delete sub_map_a;
    delete sub_map_b;
  }
}

void AngularCorrelation::FindPairAutoCorrelation(Map& stomp_map,
						 WAngularVector& galaxy,
						 uint8_t random_iterations) {
  TreeMap* galaxy_tree = new TreeMap(min_resolution_, 200);

  for (WAngularIterator iter=galaxy.begin();iter!=galaxy.end();++iter) {
    if (!galaxy_tree->AddPoint(*iter)) {
      std::cout << "Failed to add point: " << iter->Lambda() << ", " <<
	iter->Eta() << "\n";
    }
  }

  if (stomp_map.NRegion() > 0) {
    if (!galaxy_tree->InitializeRegions(stomp_map)) {
      std::cout << "Failed to initialize regions on TreeMap  Exiting.\n";
      exit(2);
    }
  }

  // Galaxy-galaxy
  std::cout << "\tGalaxy-galaxy pairs...\n";
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    if (stomp_map.NRegion() > 0) {
      galaxy_tree->FindWeightedPairsWithRegions(galaxy, *iter);
    } else {
      galaxy_tree->FindWeightedPairs(galaxy, *iter);
    }
    iter->MoveWeightToGalGal();
  }

  // Done with the galaxy-based tree, so we can delete that memory.
  delete galaxy_tree;

  // Before we start on the random iterations, we'll zero out the data fields
  // for those counts.
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    iter->ResetGalRand();
    iter->ResetRandGal();
    iter->ResetRandRand();
  }

  for (uint8_t rand_iter=0;rand_iter<random_iterations;rand_iter++) {
    std::cout << "\tRandom iteration " << rand_iter << "...\n";

    // Generate set of random points based on the input galaxy file and map.
    WAngularVector random_galaxy;
    stomp_map.GenerateRandomPoints(random_galaxy, galaxy);

    // Create the TreeMap from those random points.
    TreeMap* random_tree = new TreeMap(min_resolution_, 200);

    for (WAngularIterator iter=random_galaxy.begin();
	 iter!=random_galaxy.end();++iter) {
      if (!random_tree->AddPoint(*iter)) {
	std::cout << "Failed to add point: " << iter->Lambda() << ", " <<
	  iter->Eta() << "\n";
      }
    }

    if (stomp_map.NRegion() > 0) {
      if (!random_tree->InitializeRegions(stomp_map)) {
	std::cout << "Failed to initialize regions on TreeMap  Exiting.\n";
	exit(2);
      }
    }

    // Galaxy-Random -- there's a symmetry here, so the results go in GalRand
    // and RandGal.
    for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
      if (stomp_map.NRegion() > 0) {
	random_tree->FindWeightedPairsWithRegions(galaxy, *iter);
      } else {
	random_tree->FindWeightedPairs(galaxy, *iter);
      }
      iter->MoveWeightToGalRand(true);
    }

    // Random-Random
    for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
      if (stomp_map.NRegion() > 0) {
	random_tree->FindWeightedPairsWithRegions(random_galaxy, *iter);
      } else {
	random_tree->FindWeightedPairs(random_galaxy, *iter);
      }
      iter->MoveWeightToRandRand();
    }

    delete random_tree;
  }

  // Finally, we rescale our random pair counts to normalize them to the
  // number of input objects.
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    iter->RescaleGalRand(1.0*random_iterations);
    iter->RescaleRandGal(1.0*random_iterations);
    iter->RescaleRandRand(1.0*random_iterations);
  }
}

void AngularCorrelation::FindPairCrossCorrelation(Map& stomp_map,
						  WAngularVector& galaxy_a,
						  WAngularVector& galaxy_b,
						  uint8_t random_iterations) {
  TreeMap* galaxy_tree_a = new TreeMap(min_resolution_, 200);

  for (WAngularIterator iter=galaxy_a.begin();iter!=galaxy_a.end();++iter) {
    if (!galaxy_tree_a->AddPoint(*iter)) {
      std::cout << "Failed to add point: " << iter->Lambda() << ", " <<
	iter->Eta() << "\n";
    }
  }

  if (stomp_map.NRegion() > 0) {
    if (!galaxy_tree_a->InitializeRegions(stomp_map)) {
      std::cout << "Failed to initialize regions on TreeMap  Exiting.\n";
      exit(2);
    }
  }

  // Galaxy-galaxy
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    if (stomp_map.NRegion() > 0) {
      galaxy_tree_a->FindWeightedPairsWithRegions(galaxy_b, *iter);
    } else {
      galaxy_tree_a->FindWeightedPairs(galaxy_b, *iter);
    }
    // If the number of random iterations is 0, then we're doing a
    // WeightedCrossCorrelation instead of a cross-correlation between 2
    // population densities.  In that case, we want the ratio between the
    // WeightedPairs and Pairs, so we keep the values in the Weight and
    // Counter fields.
    if (random_iterations > 0) iter->MoveWeightToGalGal();
  }

  // Before we start on the random iterations, we'll zero out the data fields
  // for those counts.
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    iter->ResetGalRand();
    iter->ResetRandGal();
    iter->ResetRandRand();
  }

  for (uint8_t rand_iter=0;rand_iter<random_iterations;rand_iter++) {
    WAngularVector random_galaxy_a;
    stomp_map.GenerateRandomPoints(random_galaxy_a, galaxy_a);

    WAngularVector random_galaxy_b;
    stomp_map.GenerateRandomPoints(random_galaxy_b, galaxy_b);

    // Galaxy-Random
    for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
      if (stomp_map.NRegion() > 0) {
	galaxy_tree_a->FindWeightedPairsWithRegions(random_galaxy_b, *iter);
      } else {
	galaxy_tree_a->FindWeightedPairs(random_galaxy_b, *iter);
      }
      iter->MoveWeightToGalRand();
    }

    TreeMap* random_tree_a = new TreeMap(min_resolution_, 200);

    for (WAngularIterator iter=random_galaxy_a.begin();
	 iter!=random_galaxy_a.end();++iter) {
      if (!random_tree_a->AddPoint(*iter)) {
	std::cout << "Failed to add point: " << iter->Lambda() << ", " <<
	  iter->Eta() << "\n";
      }
    }

    if (stomp_map.NRegion() > 0) {
      if (!random_tree_a->InitializeRegions(stomp_map)) {
	std::cout << "Failed to initialize regions on TreeMap  Exiting.\n";
	exit(2);
      }
    }

    // Random-Galaxy
    for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
      if (stomp_map.NRegion() > 0) {
	random_tree_a->FindWeightedPairsWithRegions(galaxy_b, *iter);
      } else {
	random_tree_a->FindWeightedPairs(galaxy_b, *iter);
      }
      iter->MoveWeightToRandGal();
    }

    // Random-Random
    for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
      if (stomp_map.NRegion() > 0) {
	random_tree_a->FindWeightedPairsWithRegions(random_galaxy_b, *iter);
      } else {
	random_tree_a->FindWeightedPairs(random_galaxy_b, *iter);
      }
      iter->MoveWeightToRandRand();
    }

    delete random_tree_a;
  }

  delete galaxy_tree_a;

  // Finally, we rescale our random pair counts to normalize them to the
  // number of input objects.
  for (ThetaIterator iter=Begin(0);iter!=End(0);++iter) {
    iter->RescaleGalRand(1.0*random_iterations);
    iter->RescaleRandGal(1.0*random_iterations);
    iter->RescaleRandRand(1.0*random_iterations);
  }
}

double AngularCorrelation::ThetaMin(uint16_t resolution) {
  double theta_min = -1.0;
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      theta_min = theta_pair_begin_->ThetaMin();
    } else {
      theta_min = theta_min_;
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_,thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      theta_min = iter.first->ThetaMin();
    }
  }

  return theta_min;
}

double AngularCorrelation::ThetaMax(uint16_t resolution) {
  double theta_max = -1.0;
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      theta_max = theta_pair_end_->ThetaMin();
    } else {
      theta_max = theta_max_;
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_,thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      --iter.second;
      theta_max = iter.second->ThetaMax();
    }
  }

  return theta_max;
}

double AngularCorrelation::Sin2ThetaMin(uint16_t resolution) {
  double sin2theta_min = -1.0;
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      sin2theta_min = theta_pair_begin_->Sin2ThetaMin();
    } else {
      sin2theta_min = sin2theta_min_;
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_,thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      sin2theta_min = iter.first->Sin2ThetaMin();
    }
  }

  return sin2theta_min;
}

double AngularCorrelation::Sin2ThetaMax(uint16_t resolution) {
  double sin2theta_max = -1.0;
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      sin2theta_max = theta_pair_end_->Sin2ThetaMin();
    } else {
      sin2theta_max = sin2theta_max_;
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_,thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      --iter.second;
      sin2theta_max = iter.second->Sin2ThetaMax();
    }
  }

  return sin2theta_max;
}

ThetaIterator AngularCorrelation::Begin(uint16_t resolution) {
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      return theta_pair_begin_;
    } else {
      return thetabin_.begin();
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_, thetabin_.end(),
				 theta, AngularBin::ReverseResolutionOrder);
    return iter.first;
  }
}

ThetaIterator AngularCorrelation::End(uint16_t resolution) {
  if ((resolution < Stomp::HPixResolution) ||
      (resolution > Stomp::MaxPixelResolution) ||
      (resolution % 2 != 0)) {
    if (resolution == 0) {
      return theta_pair_end_;
    } else {
      return thetabin_.end();
    }
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(theta_pixel_begin_,thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    return iter.second;
  }
}

ThetaIterator AngularCorrelation::Find(ThetaIterator begin,
				       ThetaIterator end,
				       double sin2theta) {
  ThetaIterator top = --end;
  ThetaIterator bottom = begin;
  ThetaIterator iter;

  if ((sin2theta < bottom->Sin2ThetaMin()) ||
      (sin2theta > top->Sin2ThetaMax())) {
    iter = ++end;
  } else {
    ++top;
    --bottom;
    while (top-bottom > 1) {
      iter = bottom + (top - bottom)/2;
      if (sin2theta < iter->Sin2ThetaMin()) {
        top = iter;
      } else {
        bottom = iter;
      }
    }
    iter = bottom;
  }

  return iter;
}

ThetaIterator AngularCorrelation::BinIterator(uint8_t bin_idx) {
  ThetaIterator iter = thetabin_.begin();

  for (uint8_t i=0;i<bin_idx;++i) {
    if (iter != thetabin_.end()) ++iter;
  }

  return iter;
}

Pixel::Pixel() {
  resolution_ = 0;
  y_ = 0;
  x_ = 0;
  weight_ = 0.0;
}

Pixel::Pixel(const uint16_t input_resolution,
	     const uint32_t input_pixnum,
	     const double input_weight) {
  if ((input_resolution < Stomp::HPixResolution) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution))
    std::cout << "Invalid resolution value.\n ";

  if (input_pixnum > Stomp::MaxPixnum)
    std::cout << "Invalid pixel index value.\n ";

  SetResolution(input_resolution);
  y_ = input_pixnum/(Stomp::Nx0*Resolution());
  x_ = input_pixnum - Stomp::Nx0*Resolution()*y_;
  weight_ = input_weight;
}

Pixel::Pixel(const uint16_t input_resolution,
	     const uint32_t input_hpixnum,
	     const uint32_t input_superpixnum,
	     const double input_weight) {
  if ((input_resolution < Stomp::HPixResolution) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution)) {
    std::cout << "Invalid resolution value: " << input_resolution << ".\n";
    exit(2);
  }

  if (input_hpixnum > Stomp::MaxPixnum)
    std::cout << "Invalid hpixel index value.\n ";

  if (input_superpixnum > Stomp::MaxSuperpixnum)
    std::cout << "Invalid superpixel index value.\n ";

  SetResolution(input_resolution);

  uint16_t hnx = Resolution()/Stomp::HPixResolution;

  uint32_t y0 = input_superpixnum/(Stomp::Nx0*Stomp::HPixResolution);
  uint32_t x0 = input_superpixnum - y0*Stomp::Nx0*Stomp::HPixResolution;

  y0 *= hnx;
  x0 *= hnx;

  uint32_t tmp_y = input_hpixnum/hnx;
  uint32_t tmp_x = input_hpixnum - hnx*tmp_y;

  x_ = tmp_x + x0;
  y_ = tmp_y + y0;
  weight_ = input_weight;
}

Pixel::Pixel(const uint32_t input_x,
	     const uint32_t input_y,
	     const uint16_t input_resolution,
	     const double input_weight) {
  if ((input_resolution < Stomp::HPixResolution) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution)) {
    std::cout << "Invalid resolution value: " << input_resolution << ".\n";
    exit(2);
  }

  if (input_x > Stomp::Nx0*input_resolution)
    std::cout << "Invalid x index value.\n";

  if (input_y > Stomp::Ny0*input_resolution)
    std::cout << "Invalid y index value.\n";

  SetResolution(input_resolution);
  x_ = input_x;
  y_ = input_y;
  weight_ = input_weight;
}

Pixel::Pixel(AngularCoordinate& ang, const uint16_t input_resolution,
	     const double input_weight) {

  if ((input_resolution < Stomp::HPixResolution) ||
      (input_resolution % 2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution)) {
    std::cout << "Invalid resolution value: " << input_resolution << ".\n";
    exit(2);
  }

  SetResolution(input_resolution);

  double eta = (ang.Eta() - Stomp::EtaOffSet)*Stomp::DegToRad;

  if (eta <= 0.0) eta += 2.0*Stomp::Pi;

  eta /= 2.0*Stomp::Pi;

  x_ = static_cast<uint32_t>(Stomp::Nx0*Resolution()*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::DegToRad;

  if (lambda >= Stomp::Pi) {
    y_ = Stomp::Ny0*Resolution() - 1;
  } else {
    y_ = static_cast<uint32_t>(Stomp::Ny0*Resolution()*
			       ((1.0 - cos(lambda))/2.0));
  }

  weight_ = input_weight;
}

Pixel::~Pixel() {
  x_ = y_ = 0;
  resolution_ = 0;
  weight_ = 0.0;
}

void Pixel::Ang2Pix(uint16_t input_resolution, AngularCoordinate& ang,
		    uint32_t& output_pixnum) {
  double lambda = ang.Lambda();
  double eta = ang.Eta();
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  eta -= Stomp::EtaOffSet;

  eta *= Stomp::DegToRad;

  if (eta <= 0.0) eta += 2.0*Stomp::Pi;

  eta /= 2.0*Stomp::Pi;
  uint32_t i = static_cast<uint32_t>(nx*eta);

  lambda = (90.0 - lambda)*Stomp::DegToRad;

  uint32_t j;
  if (lambda >= Stomp::Pi) {
    j = ny - 1;
  } else {
    j = static_cast<uint32_t>(ny*((1.0 - cos(lambda))/2.0));
  }

  output_pixnum = nx*j + i;
}

void Pixel::SetPixnumFromAng(AngularCoordinate& ang) {

  double eta = (ang.Eta() - Stomp::EtaOffSet)*Stomp::DegToRad;

  if (eta <= 0.0) eta += 2.0*Stomp::Pi;

  eta /= 2.0*Stomp::Pi;
  x_ = static_cast<uint32_t>(Stomp::Nx0*Resolution()*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::DegToRad;

  if (lambda >= Stomp::Pi) {
    y_ = Stomp::Ny0*Resolution() - 1;
  } else {
    y_ = static_cast<uint32_t>(Stomp::Ny0*Resolution()*
                                    ((1.0 - cos(lambda))/2.0));
  }
}

void Pixel::Pix2Ang(uint16_t input_resolution, uint32_t input_pixnum,
		    AngularCoordinate& ang) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint32_t y = input_pixnum/nx;
  uint32_t x = input_pixnum - nx*y;

  ang.SetSurveyCoordinates(90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.5)/ny),
			   Stomp::RadToDeg*(2.0*Stomp::Pi*(x+0.5))/nx +
			   Stomp::EtaOffSet);
}

void Pixel::Pix2HPix(uint16_t input_resolution,
		     uint32_t input_pixnum,
		     uint32_t& output_hpixnum,
		     uint32_t& output_superpixnum) {
  uint32_t nx = Stomp::Nx0*input_resolution;

  uint32_t y = input_pixnum/nx;
  uint32_t x = input_pixnum - nx*y;

  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t x0 = x/hnx;
  uint32_t y0 = y/hnx;

  x -= x0*hnx;
  y -= y0*hnx;

  output_hpixnum = hnx*y + x;
  output_superpixnum = Stomp::Nx0*Stomp::HPixResolution*y0 + x0;
}

void Pixel::SuperPix(uint16_t hi_resolution, uint32_t hi_pixnum,
		     uint16_t lo_resolution, uint32_t& lo_pixnum) {
  if (hi_resolution < lo_resolution) {
    std::cout << "Can't go from low resolution to higher resolution.\n ";
    exit(1);
  } else {
    uint32_t nx_hi = Stomp::Nx0*hi_resolution;
    uint32_t nx_lo = Stomp::Nx0*lo_resolution;

    uint16_t ratio = hi_resolution/lo_resolution;

    uint32_t j = hi_pixnum/nx_hi;
    uint32_t i = hi_pixnum - nx_hi*j;

    i /= ratio;
    j /= ratio;

    lo_pixnum = nx_lo*j + i;
  }
}

bool Pixel::SetToSuperPix(uint16_t lo_resolution) {
  if (Resolution() < lo_resolution) {
    std::cout << "Illegal resolution value: " << lo_resolution <<
        " < " << Resolution();
    return false;
  }

  x_ /= Resolution()/lo_resolution;
  y_ /= Resolution()/lo_resolution;

  resolution_ = Stomp::MostSignificantBit(lo_resolution);

  return true;
}

void Pixel::NextSubPix(uint16_t input_resolution, uint32_t input_pixnum,
		       uint32_t& sub_pixnum1,
		       uint32_t& sub_pixnum2,
		       uint32_t& sub_pixnum3,
		       uint32_t& sub_pixnum4) {
  uint32_t nx_hi = 2*Stomp::Nx0*input_resolution;
  uint32_t nx_lo = Stomp::Nx0*input_resolution;

  uint32_t j = input_pixnum/nx_lo;
  uint32_t i = input_pixnum - nx_lo*j;

  sub_pixnum1 = nx_hi*(2*j) + 2*i;
  sub_pixnum2 = nx_hi*(2*j) + 2*i + 1;
  sub_pixnum3 = nx_hi*(2*j + 1) + 2*i;
  sub_pixnum4 = nx_hi*(2*j + 1) + 2*i + 1;
}

void Pixel::SubPix(uint16_t lo_resolution, uint32_t lo_pixnum,
		   uint16_t hi_resolution, uint32_t& x_min,
		   uint32_t& x_max, uint32_t& y_min,
		   uint32_t& y_max) {
  uint32_t nx_hi = Stomp::Nx0*hi_resolution;
  if (lo_resolution == hi_resolution) {
    y_min = lo_pixnum/nx_hi;
    y_max = lo_pixnum/nx_hi;
    x_min = lo_pixnum - nx_hi*y_min;
    x_max = lo_pixnum - nx_hi*y_max;
  } else {
    uint32_t tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4;
    uint16_t tmp_res;

    tmp_pixnum = lo_pixnum;
    for (tmp_res=lo_resolution;tmp_res<hi_resolution;tmp_res*=2) {
      NextSubPix(tmp_res, tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4);
      tmp_pixnum = pixnum1;
    }

    y_min = tmp_pixnum/nx_hi;
    x_min = tmp_pixnum - nx_hi*y_min;

    tmp_pixnum = lo_pixnum;
    for (tmp_res=lo_resolution;tmp_res<hi_resolution;tmp_res*=2) {
      NextSubPix(tmp_res, tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4);
      tmp_pixnum = pixnum4;
    }

    y_max = tmp_pixnum/nx_hi;
    x_max = tmp_pixnum - nx_hi*y_max;
  }
}

void Pixel::SubPix(uint16_t hi_resolution, PixelVector& pix) {
  if (!pix.empty()) pix.clear();

  uint32_t x_min, x_max, y_min, y_max;
  SubPix(hi_resolution, x_min, x_max, y_min, y_max);

  for (uint32_t y=y_min;y<=y_max;y++) {
    for (uint32_t x=x_min;x<=x_max;x++) {
      Pixel tmp_pix(x, y, hi_resolution, weight_);
      pix.push_back(tmp_pix);
    }
  }
}

void Pixel::SubPix(uint16_t hi_resolution, uint32_t& x_min,
		   uint32_t& x_max, uint32_t& y_min,
		   uint32_t& y_max) {
  if (Resolution() == hi_resolution) {
    y_min = y_max = y_;
    x_min = x_max = x_;
  } else {
    uint16_t tmp_res;

    x_min = x_max = x_;
    y_min = y_max = y_;
    for (tmp_res=Resolution();tmp_res<hi_resolution;tmp_res*=2) {
      x_min *= 2;
      y_min *= 2;

      x_max = 2*x_max + 1;
      y_max = 2*y_max + 1;
    }
  }
}

void Pixel::PixelBound(uint16_t input_resolution, uint32_t input_pixnum,
		       double& lammin, double& lammax, double& etamin,
		       double& etamax) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint32_t y = input_pixnum/nx;
  uint32_t x = input_pixnum - nx*y;

  lammin = 90.0 - Stomp::RadToDeg*acos(1.0 - 2.0*(y+1)/ny);
  lammax = 90.0 - Stomp::RadToDeg*acos(1.0 - 2.0*y/ny);
  etamin = Stomp::RadToDeg*2.0*Stomp::Pi*(x+0.0)/nx + Stomp::EtaOffSet;
  if (etamin >= 180.0) etamin = etamin - 360.0;
  etamax = Stomp::RadToDeg*2.0*Stomp::Pi*(x+1.0)/nx + Stomp::EtaOffSet;
  if (etamax >= 180.0) etamax = etamax - 360.0;
}

void Pixel::CohortPix(uint16_t input_resolution, uint32_t input_pixnum,
		      uint32_t& co_pixnum1,
		      uint32_t& co_pixnum2,
		      uint32_t& co_pixnum3) {
  uint32_t tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4;

  SuperPix(input_resolution, input_pixnum, input_resolution/2, tmp_pixnum);

  NextSubPix(input_resolution/2, tmp_pixnum,
	     pixnum1, pixnum2, pixnum3, pixnum4);

  if (input_pixnum == pixnum1) {
    co_pixnum1 = pixnum2;
    co_pixnum2 = pixnum3;
    co_pixnum3 = pixnum4;
  }
  if (input_pixnum == pixnum2) {
    co_pixnum1 = pixnum1;
    co_pixnum2 = pixnum3;
    co_pixnum3 = pixnum4;
  }
  if (input_pixnum == pixnum3) {
    co_pixnum1 = pixnum1;
    co_pixnum2 = pixnum2;
    co_pixnum3 = pixnum4;
  }
  if (input_pixnum == pixnum4) {
    co_pixnum1 = pixnum1;
    co_pixnum2 = pixnum2;
    co_pixnum3 = pixnum3;
  }
}

void Pixel::CohortPix(Pixel& pix_a, Pixel& pix_b, Pixel& pix_c) {
  uint32_t super_x, super_y, x1, x2, x3, x4, y1, y2, y3, y4;

  pix_a.SetResolution(Resolution());
  pix_b.SetResolution(Resolution());
  pix_c.SetResolution(Resolution());

  super_x = x_/2;
  super_y = y_/2;

  x1 = 2*super_x;
  y1 = 2*super_y;

  x2 = 2*super_x + 1;
  y2 = 2*super_y;

  x3 = 2*super_x;
  y3 = 2*super_y + 1;

  x4 = 2*super_x + 1;
  y4 = 2*super_y + 1;

  if ((x_ == x1) && (y_ == y1)) {
    pix_a.SetPixnumFromXY(x2,y2);
    pix_b.SetPixnumFromXY(x3,y3);
    pix_c.SetPixnumFromXY(x4,y4);
  }
  if ((x_ == x2) && (y_ == y2)) {
    pix_a.SetPixnumFromXY(x1,y1);
    pix_b.SetPixnumFromXY(x3,y3);
    pix_c.SetPixnumFromXY(x4,y4);
  }
  if ((x_ == x3) && (y_ == y3)) {
    pix_b.SetPixnumFromXY(x1,y1);
    pix_a.SetPixnumFromXY(x2,y2);
    pix_c.SetPixnumFromXY(x4,y4);
  }
  if ((x_ == x4) && (y_ == y4)) {
    pix_c.SetPixnumFromXY(x1,y1);
    pix_a.SetPixnumFromXY(x2,y2);
    pix_b.SetPixnumFromXY(x3,y3);
  }
}

void Pixel::XYBounds(double theta, uint32_t& x_min,
		     uint32_t& x_max, uint32_t& y_min,
		     uint32_t& y_max, bool add_buffer) {
  double lammin = Lambda() - theta;
  if (lammin < -90.0) lammin = -90.0;
  double lammax = Lambda() + theta;
  if (lammax > 90.0) lammax = 90.0;

  double sphere_correction  = 1.0;
  if (fabs(lammin) > fabs(lammax)) {
    sphere_correction =
      1.0 + 0.000192312*lammin*lammin -
      1.82764e-08*lammin*lammin*lammin*lammin +
      1.28162e-11*lammin*lammin*lammin*lammin*lammin*lammin;
  } else {
    sphere_correction =
      1.0 + 0.000192312*lammax*lammax -
      1.82764e-08*lammax*lammax*lammax*lammax +
      1.28162e-11*lammax*lammax*lammax*lammax*lammax*lammax;
  }

  uint32_t nx = Stomp::Nx0*Resolution();
  uint32_t ny = Stomp::Ny0*Resolution();

  double etamin = Eta() - theta*sphere_correction;
  etamin -= Stomp::EtaOffSet;
  etamin *= Stomp::DegToRad;

  if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi;

  etamin /= 2.0*Stomp::Pi;
  x_min = static_cast<uint32_t>(nx*etamin);

  lammax = (90.0 - lammax)*Stomp::DegToRad;

  if (lammax >= Stomp::Pi) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<uint32_t>(ny*((1.0 - cos(lammax))/2.0));
  }

  double etamax = Eta() + theta*sphere_correction;
  etamax -= Stomp::EtaOffSet;
  etamax *= Stomp::DegToRad;

  if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi;

  etamax /= 2.0*Stomp::Pi;
  x_max = static_cast<uint32_t>(nx*etamax);

  lammin = (90.0 - lammin)*Stomp::DegToRad;

  if (lammin >= Stomp::Pi) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<uint32_t>(ny*((1.0 - cos(lammin))/2.0));
  }

  if (add_buffer) {
    if (x_min == 0) {
      x_min = nx - 1;
    } else {
      x_min--;
    }

    if (x_max == nx - 1) {
      x_max = 0;
    } else {
      x_max++;
    }

    if (y_max < ny - 1) y_max++;
    if (y_min > 0) y_min--;
  }
}

void Pixel::XYBounds(double theta, std::vector<uint32_t>& x_min,
		     std::vector<uint32_t>& x_max,
		     uint32_t& y_min, uint32_t& y_max,
		     bool add_buffer) {
  if (!x_min.empty()) x_min.clear();
  if (!x_max.empty()) x_max.clear();

  double lammin = Lambda() - theta;
  if (lammin < -90.0) lammin = -90.0;
  double lammax = Lambda() + theta;
  if (lammax > 90.0) lammax = 90.0;

  uint32_t ny = Stomp::Ny0*Resolution();

  lammax = (90.0 - lammax)*Stomp::DegToRad;

  if (lammax >= Stomp::Pi) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<uint32_t>(ny*((1.0 - cos(lammax))/2.0));
  }

  lammin = (90.0 - lammin)*Stomp::DegToRad;

  if (lammin >= Stomp::Pi) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<uint32_t>(ny*((1.0 - cos(lammin))/2.0));
  }

  if (add_buffer) {
    if (y_max < ny - 1) y_max++;
    if (y_min > 0) y_min--;
  }

  if (!x_min.empty()) x_min.clear();
  if (!x_max.empty()) x_max.clear();

  x_min.reserve(y_max-y_min+1);
  x_max.reserve(y_max-y_min+1);

  uint32_t nx = Stomp::Nx0*Resolution();

  for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
    double lam = 90.0 -
      Stomp::RadToDeg*acos(1.0 - 2.0*(y+0.5)/(Stomp::Ny0*Resolution()));

    double sphere_correction = 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));

    double etamin = Eta() - theta*sphere_correction;
    etamin -= Stomp::EtaOffSet;
    etamin *= Stomp::DegToRad;

    if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi;

    etamin /= 2.0*Stomp::Pi;
    x_min.push_back(static_cast<uint32_t>(nx*etamin));

    double etamax = Eta() + theta*sphere_correction;
    etamax -= Stomp::EtaOffSet;
    etamax *= Stomp::DegToRad;

    if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi;

    etamax /= 2.0*Stomp::Pi;
    x_max.push_back(static_cast<uint32_t>(nx*etamax));

    if (add_buffer) {
      if (x_min[n] == 0) {
	x_min[n] = nx - 1;
      } else {
	x_min[n] -= 1;
      }

      if (x_max[n] == nx - 1) {
	x_max[n] = 0;
      } else {
	x_max[n] += 1;
      }
    }
  }
}

void Pixel::XYBounds(AngularCoordinate& ang, double theta,
		     uint32_t& x_min, uint32_t& x_max,
		     uint32_t& y_min, uint32_t& y_max,
		     bool add_buffer) {
  double lammin = ang.Lambda() - theta;
  if (lammin < -90.0) lammin = -90.0;
  double lammax = ang.Lambda() + theta;
  if (lammax > 90.0) lammax = 90.0;

  double sphere_correction  = 1.0;
  if (fabs(lammin) > fabs(lammax)) {
    sphere_correction =
      1.0 + 0.000192312*lammin*lammin -
      1.82764e-08*lammin*lammin*lammin*lammin +
      1.28162e-11*lammin*lammin*lammin*lammin*lammin*lammin;
  } else {
    sphere_correction =
      1.0 + 0.000192312*lammax*lammax -
      1.82764e-08*lammax*lammax*lammax*lammax +
      1.28162e-11*lammax*lammax*lammax*lammax*lammax*lammax;
  }

  uint32_t nx = Stomp::Nx0*Resolution();
  uint32_t ny = Stomp::Ny0*Resolution();

  double etamin = ang.Eta() - theta*sphere_correction;
  etamin -= Stomp::EtaOffSet;
  etamin *= Stomp::DegToRad;

  if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi;

  etamin /= 2.0*Stomp::Pi;
  x_min = static_cast<uint32_t>(nx*etamin);

  lammax = (90.0 - lammax)*Stomp::DegToRad;

  if (lammax >= Stomp::Pi) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<uint32_t>(ny*((1.0 - cos(lammax))/2.0));
  }

  double etamax = ang.Eta() + theta*sphere_correction;
  etamax -= Stomp::EtaOffSet;
  etamax *= Stomp::DegToRad;

  if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi;

  etamax /= 2.0*Stomp::Pi;
  x_max = static_cast<uint32_t>(nx*etamax);

  lammin = (90.0 - lammin)*Stomp::DegToRad;

  if (lammin >= Stomp::Pi) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<uint32_t>(ny*((1.0 - cos(lammin))/2.0));
  }

  if (add_buffer) {
    if (x_min == 0) {
      x_min = nx - 1;
    } else {
      x_min--;
    }

    if (x_max == nx - 1) {
      x_max = 0;
    } else {
      x_max++;
    }

    if (y_max < ny - 1) y_max++;
    if (y_min > 0) y_min--;
  }
}

void Pixel::XYBounds(AngularCoordinate& ang, double theta,
		     std::vector<uint32_t>& x_min,
		     std::vector<uint32_t>& x_max,
		     uint32_t& y_min, uint32_t& y_max,
		     bool add_buffer) {
  if (!x_min.empty()) x_min.clear();
  if (!x_max.empty()) x_max.clear();

  double lammin = ang.Lambda() - theta;
  if (lammin < -90.0) lammin = -90.0;
  double lammax = ang.Lambda() + theta;
  if (lammax > 90.0) lammax = 90.0;

  uint32_t ny = Stomp::Ny0*Resolution();

  lammax = (90.0 - lammax)*Stomp::DegToRad;
  if (lammax >= Stomp::Pi) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<uint32_t>(ny*((1.0 - cos(lammax))/2.0));
  }

  lammin = (90.0 - lammin)*Stomp::DegToRad;
  if (lammin >= Stomp::Pi) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<uint32_t>(ny*((1.0 - cos(lammin))/2.0));
  }

  if (add_buffer) {
    if (y_max < ny - 1) y_max++;
    if (y_min > 0) y_min--;
  }

  if (y_max > ny -1) {
    std::cout << "Illegal theta value on y index: Lambda,Eta = " <<
      ang.Lambda() << "," << ang.Eta() << ", theta = " << theta <<
      "\ny = " << y_max << "/" << ny - 1 << "\n";
    exit(2);
  }

  if (y_min > ny -1) {
    std::cout << "Illegal theta value on y index: Lambda,Eta = " <<
      ang.Lambda() << "," << ang.Eta() << ", theta = " << theta <<
      "\ny = " << y_min << "/" << ny - 1 << "\n";
    exit(2);
  }

  if (!x_min.empty()) x_min.clear();
  if (!x_max.empty()) x_max.clear();

  x_min.reserve(y_max-y_min+1);
  x_max.reserve(y_max-y_min+1);

  uint32_t nx = Stomp::Nx0*Resolution();
  for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
    double lam = 90.0 -
      Stomp::RadToDeg*acos(1.0 - 2.0*(y+0.5)/(Stomp::Ny0*Resolution()));

    double sphere_correction = 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));

    double etamin = ang.Eta() - theta*sphere_correction;
    etamin -= Stomp::EtaOffSet;
    etamin *= Stomp::DegToRad;
    if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi;
    etamin /= 2.0*Stomp::Pi;
    x_min.push_back(static_cast<uint32_t>(nx*etamin));

    double etamax = ang.Eta() + theta*sphere_correction;
    etamax -= Stomp::EtaOffSet;
    etamax *= Stomp::DegToRad;
    if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi;
    etamax /= 2.0*Stomp::Pi;
    x_max.push_back(static_cast<uint32_t>(nx*etamax));

    if (add_buffer) {
      if (x_min[n] == 0) {
	x_min[n] = nx - 1;
      } else {
	x_min[n] -= 1;
      }

      if (x_max[n] == nx - 1) {
	x_max[n] = 0;
      } else {
	x_max[n] += 1;
      }
    }

    if (x_max[n] > nx -1) {
      std::cout << "Illegal theta value on x index: Lambda,Eta = " <<
	ang.Lambda() << "," << ang.Eta() << ", theta = " << theta <<
	"\nx = " << x_max[n] << "/" << nx - 1 << "\n";
      exit(2);
    }
    if (x_min[n] > nx -1) {
      std::cout << "Illegal theta value on x index: Lambda,Eta = " <<
	ang.Lambda() << "," << ang.Eta() << ", theta = " << theta <<
	"\nx = " << x_min[n] << "/" << nx - 1 << "\n";
      exit(2);
    }
  }
}

void Pixel::WithinRadius(double theta_max, PixelVector& pix,
			 bool check_full_pixel) {
  AngularBin theta(0.0, theta_max);
  WithinAnnulus(theta, pix, check_full_pixel);
}

void Pixel::WithinAnnulus(double theta_min, double theta_max,
			  PixelVector& pix, bool check_full_pixel) {
  AngularBin theta(theta_min, theta_max);
  WithinAnnulus(theta, pix, check_full_pixel);
}

void Pixel::WithinAnnulus(AngularBin& theta, PixelVector& pix,
			  bool check_full_pixel) {
  if (!pix.empty()) pix.clear();

  uint32_t y_min;
  uint32_t y_max;
  std::vector<uint32_t> x_min;
  std::vector<uint32_t> x_max;

  XYBounds(theta.ThetaMax(), x_min, x_max, y_min, y_max, true);

  uint32_t nx = Stomp::Nx0*Resolution();
  uint32_t nx_pix;

  for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
    if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
      nx_pix = nx - x_min[n] + x_max[n] + 1;
    } else {
      nx_pix = x_max[n] - x_min[n] + 1;
    }
    if (nx_pix > nx) nx_pix = nx;
    for (uint32_t m=0,x=x_min[n];m<nx_pix;m++,x++) {
      if (x == nx) x = 0;
      Pixel tmp_pix(x, y, Resolution(), 1.0);
      bool within_bounds =
	theta.WithinCosBounds(UnitSphereX()*tmp_pix.UnitSphereX() +
			      UnitSphereY()*tmp_pix.UnitSphereY() +
			      UnitSphereZ()*tmp_pix.UnitSphereZ());
      if (check_full_pixel && within_bounds) {
	if (theta.WithinCosBounds(UnitSphereX()*tmp_pix.UnitSphereX_UL() +
				  UnitSphereY()*tmp_pix.UnitSphereY_UL() +
				  UnitSphereZ()*tmp_pix.UnitSphereZ_UL()) &&
	    theta.WithinCosBounds(UnitSphereX()*tmp_pix.UnitSphereX_UR() +
				  UnitSphereY()*tmp_pix.UnitSphereY_UR() +
				  UnitSphereZ()*tmp_pix.UnitSphereZ_UR()) &&
	    theta.WithinCosBounds(UnitSphereX()*tmp_pix.UnitSphereX_LL() +
				  UnitSphereY()*tmp_pix.UnitSphereY_LL() +
				  UnitSphereZ()*tmp_pix.UnitSphereZ_LL()) &&
	    theta.WithinCosBounds(UnitSphereX()*tmp_pix.UnitSphereX_LR() +
				  UnitSphereY()*tmp_pix.UnitSphereY_LR() +
				  UnitSphereZ()*tmp_pix.UnitSphereZ_LR())) {
	  within_bounds = true;
	} else {
	  within_bounds = false;
	}
      }
      if (within_bounds) pix.push_back(tmp_pix);
    }
  }
}

void Pixel::BoundingRadius(double theta_max, PixelVector& pix) {
  AngularCoordinate ang;
  Ang(ang);

  BoundingRadius(ang, theta_max, pix);
}

void Pixel::BoundingRadius(AngularCoordinate& ang, double theta_max,
			   PixelVector& pix) {
  SetPixnumFromAng(ang);
  if (!pix.empty()) pix.clear();

  uint32_t y_min;
  uint32_t y_max;
  std::vector<uint32_t> x_min;
  std::vector<uint32_t> x_max;

  XYBounds(ang, theta_max, x_min, x_max, y_min, y_max, true);

  uint32_t nx = Stomp::Nx0*Resolution();
  uint32_t nx_pix;

  for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
    if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
      nx_pix = nx - x_min[n] + x_max[n] + 1;
    } else {
      nx_pix = x_max[n] - x_min[n] + 1;
    }
    if (nx_pix > nx) nx_pix = nx;
    for (uint32_t m=0,x=x_min[n];m<nx_pix;m++,x++) {
      if (x == nx) x = 0;
      Pixel tmp_pix(x, y, Resolution(), 1.0);
      pix.push_back(tmp_pix);
    }
  }
}

double Pixel::NearEdgeDistance(AngularCoordinate& ang) {
  // If the test position is within the lambda or eta ranges of the pixel,
  // then we need to do a proper calculation.  Otherwise, we can just return
  // the nearest corner distance.
  double min_edge_distance = -1.0;
  bool inside_bounds = false;
  if (Stomp::DoubleLE(ang.Lambda(), LambdaMax()) &&
      Stomp::DoubleGE(ang.Lambda(), LambdaMin())) {
    inside_bounds = true;

    double lam = ang.Lambda();
    double eta_scaling = 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));

    min_edge_distance = fabs(ang.Eta() - EtaMin());

    if (min_edge_distance > fabs(ang.Eta() - EtaMax()))
      min_edge_distance = fabs(ang.Eta() - EtaMax());
    min_edge_distance /= eta_scaling;
  }

  if (Stomp::DoubleLE(ang.Eta(), EtaMax()) &&
      Stomp::DoubleGE(ang.Eta(), EtaMin())) {
    double lambda_distance = fabs(ang.Lambda() - LambdaMax());
    if (lambda_distance > fabs(ang.Lambda() - LambdaMin()))
      lambda_distance = fabs(ang.Lambda() - LambdaMin());
    if (inside_bounds) {
      if (lambda_distance < min_edge_distance)
	min_edge_distance = lambda_distance;
    } else {
      min_edge_distance = lambda_distance;
    }
    inside_bounds = true;
  }

  if (!inside_bounds) {
    // If we're outside of those bounds, then the nearest part of the pixel
    // should be the near corner.
    min_edge_distance = NearCornerDistance(ang);
  } else {
    // The return value for this function is (sin(theta))^2 rather than just
    // the angle theta.  If we can get by with the small angle approximation,
    // then we use that for speed purposes.
    if (min_edge_distance < 3.0) {
      min_edge_distance = min_edge_distance*Stomp::DegToRad;
    } else {
      min_edge_distance = sin(min_edge_distance*Stomp::DegToRad);
    }
    min_edge_distance *= min_edge_distance;
  }

  return min_edge_distance;
}

double Pixel::FarEdgeDistance(AngularCoordinate& ang) {
  // If the test position is within the lambda or eta ranges of the pixel,
  // then we need to do a proper calculation.  Otherwise, we can just return
  // the farthest corner distance.
  double max_edge_distance = -1.0;
  bool inside_bounds = false;
  if (Stomp::DoubleLE(ang.Lambda(), LambdaMax()) &&
      Stomp::DoubleGE(ang.Lambda(), LambdaMin())) {
    inside_bounds = true;

    double lam = ang.Lambda();
    double eta_scaling = 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));

    max_edge_distance = fabs(ang.Eta() - EtaMin());

    if (max_edge_distance < fabs(ang.Eta() - EtaMax()))
      max_edge_distance = fabs(ang.Eta() - EtaMax());
    max_edge_distance /= eta_scaling;
  }

  if (Stomp::DoubleLE(ang.Eta(), EtaMax()) &&
      Stomp::DoubleGE(ang.Eta(), EtaMin())) {
    double lambda_distance = fabs(ang.Lambda() - LambdaMax());
    if (lambda_distance > fabs(ang.Lambda() - LambdaMin()))
      lambda_distance = fabs(ang.Lambda() - LambdaMin());
    if (inside_bounds) {
      if (lambda_distance > max_edge_distance)
	max_edge_distance = lambda_distance;
    } else {
      max_edge_distance = lambda_distance;
    }
    inside_bounds = true;
  }

  if (!inside_bounds) {
    // If we're outside of those bounds, then the farthest part of the pixel
    // should be the far corner.
    max_edge_distance = FarCornerDistance(ang);
  } else {
    // The return value for this function is (sin(theta))^2 rather than just
    // the angle theta.  If we can get by with the small angle approximation,
    // then we use that for speed purposes.
    if (max_edge_distance < 3.0) {
      max_edge_distance = max_edge_distance*Stomp::DegToRad;
    } else {
      max_edge_distance = sin(max_edge_distance*Stomp::DegToRad);
    }
    max_edge_distance *= max_edge_distance;
  }
  return max_edge_distance;
}

double Pixel::NearCornerDistance(AngularCoordinate& ang) {
  double costheta_final = ang.UnitSphereX()*UnitSphereX_UL() +
    ang.UnitSphereY()*UnitSphereY_UL() +
    ang.UnitSphereZ()*UnitSphereZ_UL();

  double costheta = ang.UnitSphereX()*UnitSphereX_UR() +
    ang.UnitSphereY()*UnitSphereY_UR() +
    ang.UnitSphereZ()*UnitSphereZ_UR();
  if (costheta > costheta_final) costheta_final = costheta;

  costheta = ang.UnitSphereX()*UnitSphereX_LR() +
    ang.UnitSphereY()*UnitSphereY_LR() +
    ang.UnitSphereZ()*UnitSphereZ_LR();
  if (costheta > costheta_final) costheta_final = costheta;

  costheta = ang.UnitSphereX()*UnitSphereX_LL() +
    ang.UnitSphereY()*UnitSphereY_LL() +
    ang.UnitSphereZ()*UnitSphereZ_LL();
  if (costheta > costheta_final) costheta_final = costheta;

  // The return value for this function is (sin(theta))^2 rather than just
  // the angle theta for computing speed purposes.
  return 1.0 - costheta_final*costheta_final;
}

double Pixel::FarCornerDistance(AngularCoordinate& ang) {
  double costheta_final = ang.UnitSphereX()*UnitSphereX_UL() +
    ang.UnitSphereY()*UnitSphereY_UL() +
    ang.UnitSphereZ()*UnitSphereZ_UL();

  double costheta = ang.UnitSphereX()*UnitSphereX_UR() +
    ang.UnitSphereY()*UnitSphereY_UR() +
    ang.UnitSphereZ()*UnitSphereZ_UR();
  if (costheta < costheta_final) costheta_final = costheta;

  costheta = ang.UnitSphereX()*UnitSphereX_LR() +
    ang.UnitSphereY()*UnitSphereY_LR() +
    ang.UnitSphereZ()*UnitSphereZ_LR();
  if (costheta < costheta_final) costheta_final = costheta;

  costheta = ang.UnitSphereX()*UnitSphereX_LL() +
    ang.UnitSphereY()*UnitSphereY_LL() +
    ang.UnitSphereZ()*UnitSphereZ_LL();
  if (costheta < costheta_final) costheta_final = costheta;

  // The return value for this function is (sin(theta))^2 rather than just
  // the angle theta for computing speed purposes.
  return 1.0 - costheta_final*costheta_final;
}

bool Pixel::IsWithinRadius(AngularCoordinate& ang, double theta_max,
			   bool check_full_pixel) {
  AngularBin theta(0.0, theta_max);

  return IsWithinAnnulus(ang, theta, check_full_pixel);
}

bool Pixel::IsWithinRadius(Pixel& pix, double theta_max,
			   bool check_full_pixel) {
  AngularBin theta(0.0, theta_max);
  AngularCoordinate ang;
  pix.Ang(ang);

  return IsWithinAnnulus(ang, theta, check_full_pixel);
}

bool Pixel::IsWithinAnnulus(AngularCoordinate& ang, double theta_min,
			    double theta_max, bool check_full_pixel) {
  AngularBin theta(theta_min, theta_max);

  return IsWithinAnnulus(ang, theta, check_full_pixel);
}

bool Pixel::IsWithinAnnulus(Pixel& pix, double theta_min,
			    double theta_max, bool check_full_pixel) {
  AngularBin theta(theta_min, theta_max);
  AngularCoordinate ang;
  pix.Ang(ang);

  return IsWithinAnnulus(ang, theta, check_full_pixel);
}

bool Pixel::IsWithinAnnulus(AngularCoordinate& ang, AngularBin& theta,
			    bool check_full_pixel) {
  bool within_bounds = theta.WithinCosBounds(ang.UnitSphereX()*UnitSphereX() +
					     ang.UnitSphereY()*UnitSphereY() +
					     ang.UnitSphereZ()*UnitSphereZ());
  if (within_bounds && check_full_pixel) {
    if (theta.WithinCosBounds(ang.UnitSphereX()*UnitSphereX_UL() +
			      ang.UnitSphereY()*UnitSphereY_UL() +
			      ang.UnitSphereZ()*UnitSphereZ_UL()) &&
	theta.WithinCosBounds(ang.UnitSphereX()*UnitSphereX_UR() +
			      ang.UnitSphereY()*UnitSphereY_UR() +
			      ang.UnitSphereZ()*UnitSphereZ_UR()) &&
	theta.WithinCosBounds(ang.UnitSphereX()*UnitSphereX_LL() +
			      ang.UnitSphereY()*UnitSphereY_LL() +
			      ang.UnitSphereZ()*UnitSphereZ_LL()) &&
	theta.WithinCosBounds(ang.UnitSphereX()*UnitSphereX_LR() +
			      ang.UnitSphereY()*UnitSphereY_LR() +
			      ang.UnitSphereZ()*UnitSphereZ_LR())) {
      within_bounds = true;
    } else {
      within_bounds = false;
    }
  }
  return within_bounds;
}

bool Pixel::IsWithinAnnulus(Pixel& pix, AngularBin& theta,
			    bool check_full_pixel) {
  AngularCoordinate ang;
  pix.Ang(ang);

  return IsWithinAnnulus(ang, theta, check_full_pixel);
}

int8_t Pixel::IntersectsAnnulus(AngularCoordinate& ang,
				double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);

  return IntersectsAnnulus(ang, theta);
}

int8_t Pixel::IntersectsAnnulus(Pixel& pix, double theta_min,
				double theta_max) {
  AngularBin theta(theta_min, theta_max);
  AngularCoordinate ang;
  pix.Ang(ang);

  return IntersectsAnnulus(ang, theta);
}

int8_t Pixel::IntersectsAnnulus(AngularCoordinate& ang, AngularBin& theta) {
  // By default, we assume that there is no intersection between the annulus
  // and the pixel.
  int8_t intersects_annulus = 0;

  if (IsWithinAnnulus(ang, theta, true)) {
    // Fully contained in the annulus.
    intersects_annulus = 1;
  } else {
    // We're not fully contained, so we need to do some checking to see
    // if there's a possibility that the annulus intersects some part of
    // our pixel.  We check the case following cases:
    //   * the annulus center and inner boundary is within the pixel
    //   * the annulus cuts through the middle of the pixel
    //   * the annulus straddles the near pixel edge
    //   * the annulus straddles the far pixel edge.
    //   * the annulus includes either the near or far corners
    //   * the annulus is between the near and far corners
    // If these all fail, then the annulus doesn't intersect the pixel.
      double near_corner_distance = NearCornerDistance(ang);
      double far_corner_distance = FarCornerDistance(ang);
      if ((Stomp::DoubleLE(near_corner_distance, theta.Sin2ThetaMax()) &&
	   Stomp::DoubleGE(far_corner_distance, theta.Sin2ThetaMin())) ||
	  theta.WithinSin2Bounds(far_corner_distance))
	intersects_annulus = -1;

    if (intersects_annulus == 0) {
      // Edge checking is more expensive, so we only do it if the edge
      // checking has failed.
      double near_edge_distance = NearEdgeDistance(ang);
      double far_edge_distance = FarEdgeDistance(ang);
      if ((Contains(ang) &&
	   Stomp::DoubleLE(theta.Sin2ThetaMin(), far_edge_distance)) ||
	  (Stomp::DoubleGE(far_edge_distance, theta.Sin2ThetaMax()) &&
	   Stomp::DoubleLE(near_edge_distance, theta.Sin2ThetaMin())) ||
	  theta.WithinSin2Bounds(far_edge_distance) ||
	  theta.WithinSin2Bounds(near_edge_distance))
	intersects_annulus = -1;
    }
  }

  return intersects_annulus;
}

int8_t Pixel::IntersectsAnnulus(Pixel& pix, AngularBin& theta) {
  AngularCoordinate ang;
  pix.Ang(ang);

  return IntersectsAnnulus(ang, theta);
}

void Pixel::GenerateRandomPoints(AngularVector& ang, uint32_t n_point) {
  if (!ang.empty()) ang.clear();
  ang.reserve(n_point);

  MTRand mtrand;

  mtrand.seed();

  double z_min = sin(LambdaMin()*Stomp::DegToRad);
  double z_max = sin(LambdaMax()*Stomp::DegToRad);
  double eta_min = EtaMin();
  double eta_max = EtaMax();
  double z = 0.0;
  double lambda = 0.0;
  double eta = 0.0;

  for (uint32_t m=0;m<n_point;m++) {
    z = z_min + mtrand.rand(z_max - z_min);
    lambda = asin(z)*Stomp::RadToDeg;
    eta = eta_min + mtrand.rand(eta_max - eta_min);

    AngularCoordinate tmp_ang;
    tmp_ang.SetSurveyCoordinates(lambda, eta);
    ang.push_back(tmp_ang);
  }
}

double Pixel::RA() {
  double ra, dec;
  AngularCoordinate::SurveyToEquatorial(Lambda(), Eta(), ra, dec);

  return ra;
}

double Pixel::DEC() {
  double ra, dec;
  AngularCoordinate::SurveyToEquatorial(Lambda(), Eta(), ra, dec);

  return dec;
}

double Pixel::GalLon() {
  double gal_lon, gal_lat;
  AngularCoordinate::SurveyToGalactic(Lambda(), Eta(), gal_lon, gal_lat);

  return gal_lon;
}

double Pixel::GalLat() {
  double gal_lon, gal_lat;
  AngularCoordinate::SurveyToGalactic(Lambda(), Eta(), gal_lon, gal_lat);

  return gal_lat;
}

double Pixel::DECMin() {
  double ra = 0.0, dec = 0.0;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMin(), ra, dec);
  double dec_min = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMax(), ra, dec);
  if (dec < dec_min) dec_min = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMin(), ra, dec);
  if (dec < dec_min) dec_min = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMax(), ra, dec);
  if (dec < dec_min) dec_min = dec;

  return dec_min;
}

double Pixel::DECMax() {
  double ra = 0.0, dec = 0.0;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMin(), ra, dec);
  double dec_max = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMax(), ra, dec);
  if (dec > dec_max) dec_max = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMin(), ra, dec);
  if (dec > dec_max) dec_max = dec;

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMax(), ra, dec);
  if (dec > dec_max) dec_max = dec;

  return dec_max;
}

double Pixel::RAMin() {
  bool crosses_meridian = false;
  double ra = 0.0, dec = 0.0;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMin(), ra, dec);
  double ra_min = ra;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_min < 60.0) || (ra < 60.0 && ra_min > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra < ra_min) ra_min = ra;
  } else {
    if (ra > 300.0) ra_min = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMin(), ra, dec);
  if ((ra > 300.0 && ra_min < 60.0) || (ra < 60.0 && ra_min > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra < ra_min) ra_min = ra;
  } else {
    if (ra_min < 60.0) ra_min = ra;
    if (ra > 300.0 && ra < ra_min) ra_min = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_min < 60.0) || (ra < 60.0 && ra_min > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra < ra_min) ra_min = ra;
  } else {
    if (ra_min < 60.0) ra_min = ra;
    if (ra > 300.0 && ra < ra_min) ra_min = ra;
  }

  return ra_min;
}

double Pixel::RAMax() {
  bool crosses_meridian = false;
  double ra = 0.0, dec = 0.0;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMin(), ra, dec);
  double ra_max = ra;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra < 60.0) ra_max = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMin(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra_max > 300.0) ra_max = ra;
    if (ra < 60.0 && ra > ra_max) ra_max = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra_max > 300.0) ra_max = ra;
    if (ra < 60.0 && ra > ra_max) ra_max = ra;
  }

  return ra_max;
}

double Pixel::RAMaxContinuous() {
  bool crosses_meridian = false;
  double ra = 0.0, dec = 0.0;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMin(), ra, dec);
  double ra_max = ra;

  AngularCoordinate::SurveyToEquatorial(LambdaMin(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra < 60.0) ra_max = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMin(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra_max > 300.0) ra_max = ra;
    if (ra < 60.0 && ra > ra_max) ra_max = ra;
  }

  AngularCoordinate::SurveyToEquatorial(LambdaMax(), EtaMax(), ra, dec);
  if ((ra > 300.0 && ra_max < 60.0) || (ra < 60.0 && ra_max > 300.0))
    crosses_meridian = true;
  if (!crosses_meridian) {
    if (ra > ra_max) ra_max = ra;
  } else {
    if (ra_max > 300.0) ra_max = ra;
    if (ra < 60.0 && ra > ra_max) ra_max = ra;
  }

  if (crosses_meridian && ra_max < 60.0) ra_max += 360.0;

  return ra_max;
}

double Pixel::GalLatMin() {
  double gal_lon = 0.0, gal_lat = 0.0;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMin(), gal_lon, gal_lat);
  double gal_lat_min = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMax(), gal_lon, gal_lat);
  if (gal_lat < gal_lat_min) gal_lat_min = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMin(), gal_lon, gal_lat);
  if (gal_lat < gal_lat_min) gal_lat_min = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMax(), gal_lon, gal_lat);
  if (gal_lat < gal_lat_min) gal_lat_min = gal_lat;

  return gal_lat_min;
}

double Pixel::GalLatMax() {
  double gal_lon = 0.0, gal_lat = 0.0;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMin(), gal_lon, gal_lat);
  double gal_lat_max = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMax(), gal_lon, gal_lat);
  if (gal_lat > gal_lat_max) gal_lat_max = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMin(), gal_lon, gal_lat);
  if (gal_lat > gal_lat_max) gal_lat_max = gal_lat;

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMax(), gal_lon, gal_lat);
  if (gal_lat > gal_lat_max) gal_lat_max = gal_lat;

  return gal_lat_max;
}

double Pixel::GalLonMin() {
  bool crosses_meridian = false;
  double gal_lon = 0.0, gal_lat = 0.0;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMin(), gal_lon, gal_lat);
  double gal_lon_min = gal_lon;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_min < 60.0) ||
      (gal_lon < 60.0 && gal_lon_min > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon < gal_lon_min) gal_lon_min = gal_lon;
  } else {
    if (gal_lon > 300.0) gal_lon_min = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMin(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_min < 60.0) ||
      (gal_lon < 60.0 && gal_lon_min > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon < gal_lon_min) gal_lon_min = gal_lon;
  } else {
    if (gal_lon_min < 60.0) gal_lon_min = gal_lon;
    if (gal_lon > 300.0 && gal_lon < gal_lon_min) gal_lon_min = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_min < 60.0) ||
      (gal_lon < 60.0 && gal_lon_min > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon < gal_lon_min) gal_lon_min = gal_lon;
  } else {
    if (gal_lon_min < 60.0) gal_lon_min = gal_lon;
    if (gal_lon > 300.0 && gal_lon < gal_lon_min) gal_lon_min = gal_lon;
  }

  return gal_lon_min;
}

double Pixel::GalLonMax() {
  bool crosses_meridian = false;
  double gal_lon = 0.0, gal_lat = 0.0;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMin(), gal_lon, gal_lat);
  double gal_lon_max = gal_lon;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon < 60.0) gal_lon_max = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMin(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon_max > 300.0) gal_lon_max = gal_lon;
    if (gal_lon < 60.0 && gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon_max > 300.0) gal_lon_max = gal_lon;
    if (gal_lon < 60.0 && gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  }

  return gal_lon_max;
}

double Pixel::GalLonMaxContinuous() {
  bool crosses_meridian = false;
  double gal_lon = 0.0, gal_lat = 0.0;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMin(), gal_lon, gal_lat);
  double gal_lon_max = gal_lon;

  AngularCoordinate::SurveyToGalactic(LambdaMin(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon < 60.0) gal_lon_max = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMin(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon_max > 300.0) gal_lon_max = gal_lon;
    if (gal_lon < 60.0 && gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  }

  AngularCoordinate::SurveyToGalactic(LambdaMax(), EtaMax(), gal_lon, gal_lat);
  if ((gal_lon > 300.0 && gal_lon_max < 60.0) ||
      (gal_lon < 60.0 && gal_lon_max > 300.0)) crosses_meridian = true;
  if (!crosses_meridian) {
    if (gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  } else {
    if (gal_lon_max > 300.0) gal_lon_max = gal_lon;
    if (gal_lon < 60.0 && gal_lon > gal_lon_max) gal_lon_max = gal_lon;
  }

  if (crosses_meridian) gal_lon_max += 360.0;

  return gal_lon_max;
}

bool Pixel::Contains(AngularCoordinate& ang) {
  double eta = (ang.Eta() - Stomp::EtaOffSet)*Stomp::DegToRad;
  if (eta <= 0.0) eta += 2.0*Stomp::Pi;
  eta /= 2.0*Stomp::Pi;

  if (x_ == static_cast<uint32_t>(Stomp::Nx0*Resolution()*eta)) {
    double lambda = (90.0 - ang.Lambda())*Stomp::DegToRad;
    if (lambda >= Stomp::Pi) {
      return (y_ == Stomp::Ny0*Resolution() - 1 ? true : false);
    } else {
      return (y_ == static_cast<uint32_t>(Stomp::Ny0*Resolution()*
					  ((1.0 - cos(lambda))/2.0)) ?
	      true : false);
    }
  } else {
    return false;
  }
}

bool Pixel::WithinBounds(double lon_min, double lon_max,
			 double lat_min, double lat_max,
			 AngularCoordinate::Sphere sphere) {
  double pix_lon_min = 0.0;
  double pix_lon_max = 0.0;
  double pix_lat_min = 0.0;
  double pix_lat_max = 0.0;

  switch (sphere) {
  case AngularCoordinate::Survey:
    pix_lon_min = EtaMin();
    pix_lon_max = EtaMax();
    pix_lat_min = LambdaMin();
    pix_lat_max = LambdaMax();
    break;
  case AngularCoordinate::Equatorial:
    pix_lon_min = RAMin();
    pix_lon_max = RAMax();
    pix_lat_min = DECMin();
    pix_lat_max = DECMax();
    break;
  case AngularCoordinate::Galactic:
    pix_lon_min = GalLonMin();
    pix_lon_max = GalLonMax();
    pix_lat_min = GalLatMin();
    pix_lat_max = GalLatMax();
    break;
  }

  return (Stomp::DoubleLE(pix_lon_max, lon_max) &&
	  Stomp::DoubleGE(pix_lon_min, lon_min) &&
	  Stomp::DoubleLE(pix_lat_max, lat_max) &&
	  Stomp::DoubleGE(pix_lat_min, lat_min) ? true : false);
}

bool Pixel::IntersectsBounds(double lon_min, double lon_max,
			     double lat_min, double lat_max,
			     AngularCoordinate::Sphere sphere) {
  double pix_lon_min = 0.0;
  double pix_lon_max = 0.0;
  double pix_lat_min = 0.0;
  double pix_lat_max = 0.0;

  switch (sphere) {
  case AngularCoordinate::Survey:
    pix_lon_min = EtaMin();
    pix_lon_max = EtaMax();
    pix_lat_min = LambdaMin();
    pix_lat_max = LambdaMax();
    break;
  case AngularCoordinate::Equatorial:
    pix_lon_min = RAMin();
    pix_lon_max = RAMax();
    pix_lat_min = DECMin();
    pix_lat_max = DECMax();
    break;
  case AngularCoordinate::Galactic:
    pix_lon_min = GalLonMin();
    pix_lon_max = GalLonMax();
    pix_lat_min = GalLatMin();
    pix_lat_max = GalLatMax();
    break;
  }

  // Either the pixel contains the bounds or the bounds contain the pixel.
  bool contained = ((Stomp::DoubleLE(pix_lon_max, lon_max) &&
		     Stomp::DoubleGE(pix_lon_min, lon_min) &&
		     Stomp::DoubleLE(pix_lat_max, lat_max) &&
		     Stomp::DoubleGE(pix_lat_min, lat_min)) ||
		    (Stomp::DoubleLE(lon_max, pix_lon_max) &&
		     Stomp::DoubleGE(lon_min, pix_lon_min) &&
		     Stomp::DoubleLE(lat_max, pix_lat_max) &&
		     Stomp::DoubleGE(lat_min, pix_lat_min)));

  // Check to see if any of the corners of the bound are within the pixel.
  bool corner = ((Stomp::DoubleLE(pix_lon_min, lon_max) &&
		  Stomp::DoubleGE(pix_lon_min, lon_min) &&
		  Stomp::DoubleLE(pix_lat_max, lat_max) &&
		  Stomp::DoubleGE(pix_lat_max, lat_min)) ||
		 (Stomp::DoubleLE(pix_lon_max, lon_max) &&
		  Stomp::DoubleGE(pix_lon_max, lon_min) &&
		  Stomp::DoubleLE(pix_lat_max, lat_max) &&
		  Stomp::DoubleGE(pix_lat_max, lat_min)) ||
		 (Stomp::DoubleLE(pix_lon_min, lon_max) &&
		  Stomp::DoubleGE(pix_lon_min, lon_min) &&
		  Stomp::DoubleLE(pix_lat_min, lat_max) &&
		  Stomp::DoubleGE(pix_lat_min, lat_min)) ||
		 (Stomp::DoubleLE(pix_lon_max, lon_max) &&
		  Stomp::DoubleGE(pix_lon_max, lon_min) &&
		  Stomp::DoubleLE(pix_lat_min, lat_max) &&
		  Stomp::DoubleGE(pix_lat_min, lat_min)));

  // Check the cases where the bounds cut through the pixel horizontally, either
  // passing all the way through or taking a chunk out of the left or right
  // side of the pixel.
  bool lat_middle = (((Stomp::DoubleGE(pix_lon_min, lon_min) &&
		       Stomp::DoubleLE(pix_lon_max, lon_max)) ||
		      (Stomp::DoubleLE(pix_lon_min, lon_min) &&
		       Stomp::DoubleGE(pix_lon_max, lon_min)) ||
		      (Stomp::DoubleLE(pix_lon_min, lon_max) &&
		       Stomp::DoubleGE(pix_lon_max, lon_max))) &&
		     Stomp::DoubleGE(pix_lat_max, lat_max) &&
		     Stomp::DoubleLE(pix_lat_min, lat_min));

  // Same as above, but for a vertical slice through the pixel.
  bool lon_middle = (((Stomp::DoubleGE(pix_lat_min, lat_min) &&
		       Stomp::DoubleLE(pix_lat_max, lat_max)) ||
		      (Stomp::DoubleLE(pix_lat_min, lat_min) &&
		       Stomp::DoubleGE(pix_lat_max, lat_min)) ||
		      (Stomp::DoubleLE(pix_lat_min, lat_max) &&
		       Stomp::DoubleGE(pix_lat_max, lat_max))) &&
		     Stomp::DoubleLE(pix_lon_min, lon_min) &&
		     Stomp::DoubleGE(pix_lon_max, lon_max));

  // If any of our cases hold, send back True.  False, otherwise.
  return (contained || corner || lon_middle || lat_middle ? true : false);
}

void Pixel::AreaIndex(uint16_t input_resolution,
		      double lammin, double lammax,
		      double etamin, double etamax,
		      uint32_t& x_min, uint32_t& x_max,
		      uint32_t& y_min, uint32_t& y_max) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  etamin -= Stomp::EtaOffSet;
  etamin *= Stomp::DegToRad;
  if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi;
  etamin /= 2.0*Stomp::Pi;
  x_min = static_cast<uint32_t>(nx*etamin);

  etamax -= Stomp::EtaOffSet;
  etamax *= Stomp::DegToRad;
  if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi;
  etamax /= 2.0*Stomp::Pi;
  x_max = static_cast<uint32_t>(nx*etamax);

  uint32_t ny = Stomp::Ny0*input_resolution;
  lammax = (90.0 - lammax)*Stomp::DegToRad;
  if (lammax >= Stomp::Pi) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<uint32_t>(ny*((1.0 - cos(lammax))/2.0));
  }

  lammin = (90.0 - lammin)*Stomp::DegToRad;
  if (lammin >= Stomp::Pi) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<uint32_t>(ny*((1.0 - cos(lammin))/2.0));
  }
}

uint8_t Pixel::Pix2EtaStep(uint16_t input_resolution, uint32_t input_pixnum,
		       double theta) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint32_t y = input_pixnum/nx;
  double lam = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.5)/ny);

  double deta = 2.5/(input_resolution/4);
  double eta_step = theta;
  eta_step *= 1.0 +
    lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));
  uint8_t etastep = 1;
  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

uint8_t Pixel::EtaStep(double theta) {
  double lam = Lambda();

  double deta = 2.5/(Resolution()/4);
  double eta_step = theta;
  eta_step *= 1.0 +
    lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));
  uint8_t etastep = 1;
  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

uint32_t Pixel::Stripe(uint16_t input_resolution) {
  if ((input_resolution%2 != 0) || (input_resolution < 4)) {
    std::cout << "Illegal resolution in Stripe() call!\nExiting...\n";
    exit(1);
  }

  double stripe_width = 360.0/(Stomp::Nx0*input_resolution);
  int32_t stripe = static_cast<int32_t>((Eta() + 32.5)/stripe_width) + 10;

  double etamin = stripe_width*(stripe - 10) - 32.5 -
      stripe_width/2.0 + 0.0000001;
  double etamax = stripe_width*(stripe - 10) - 32.5 +
      stripe_width/2.0 - 0.0000001;
  if (Eta() < etamin) stripe++;
  if (Eta() > etamax) stripe++;
  if (stripe < 0) stripe += Stomp::Nx0*input_resolution;

  return static_cast<uint32_t>(stripe);
}

bool Pixel::LocalOrder(Pixel pix_a, Pixel pix_b) {
  if (pix_a.Resolution() == pix_b.Resolution()) {
    if (pix_a.PixelY() == pix_b.PixelY()) {
      return (pix_a.PixelX() < pix_b.PixelX() ? true : false);
    } else {
      return (pix_a.PixelY() < pix_b.PixelY() ? true : false);
    }
  } else {
    return (pix_a.Resolution() < pix_b.Resolution() ? true : false);
  }
}

bool Pixel::SuperPixelBasedOrder(Pixel pix_a, Pixel pix_b) {
  if (pix_a.Superpixnum() == pix_b.Superpixnum()) {
    if (pix_a.Resolution() == pix_b.Resolution()) {
      return (pix_a.HPixnum() < pix_b.HPixnum() ? true : false);
    } else {
      return (pix_a.Resolution() < pix_b.Resolution() ? true : false);
    }
  } else {
    return (pix_a.Superpixnum() < pix_b.Superpixnum() ? true : false);
  }
}

bool Pixel::SuperPixelOrder(Pixel pix_a, Pixel pix_b) {
  return (pix_a.Superpixnum() < pix_b.Superpixnum() ? true : false);
}

void Pixel::ResolvePixel(PixelVector& pix, bool ignore_weight) {
  sort(pix.begin(),pix.end(),Pixel::SuperPixelBasedOrder);

  PixelVector tmp_pix;
  PixelVector final_pix;
  uint32_t superpixnum = pix[0].Superpixnum();

  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
    if (superpixnum == iter->Superpixnum()) {
      tmp_pix.push_back(*iter);
    } else {
      ResolveSuperPixel(tmp_pix,ignore_weight);

      for (uint32_t i=0;i<tmp_pix.size();i++)
        final_pix.push_back(tmp_pix[i]);

      tmp_pix.clear();
      superpixnum = iter->Superpixnum();
      tmp_pix.push_back(*iter);
    }
  }

  ResolveSuperPixel(tmp_pix,ignore_weight);

  for (uint32_t i=0;i<tmp_pix.size();i++)
    final_pix.push_back(tmp_pix[i]);

  tmp_pix.clear();

  pix.clear();
  pix.reserve(final_pix.size());

  for (uint32_t i=0;i<final_pix.size();i++) pix.push_back(final_pix[i]);

  final_pix.clear();
}

void Pixel::ResolveSuperPixel(PixelVector& pix, bool ignore_weight) {
  if (ignore_weight)
    for (uint32_t i=0;i<pix.size();i++) pix[i].SetWeight(1.0);

  // First, remove any duplicate pixels or pixels that are contained within
  // other pixels.
  PixelVector unique_pix;
  Pixel::FindUniquePixels(pix, unique_pix);

  uint32_t n_start = pix.size();
  pix.clear();
  pix.reserve(unique_pix.size());

  for (uint32_t i=0;i<unique_pix.size();i++) pix.push_back(unique_pix[i]);

  sort(pix.begin(), pix.end(), Pixel::SuperPixelBasedOrder);
  unique_pix.clear();
  uint32_t n_finish = unique_pix.size();

  // Now we iterate through the pixel list, looking for possible cases where
  // we might combine high resolution pixels into low resolution pixels.  In
  // order to do this, we need to find all of the cohort pixels as well as
  // verify that they all have the same Weight.
  while (n_start != n_finish) {
    n_start = pix.size();

    unique_pix.reserve(pix.size());
    PixelIterator search_begin = pix.begin();
    for (uint32_t i=0;i<pix.size();i++) {
      if ((pix[i].Resolution() > Stomp::HPixResolution) &&
	  pix[i].FirstCohort()) {
        bool found_cohort = false;
        Pixel pix_a;
        Pixel pix_b;
        Pixel pix_c;

	pix[i].CohortPix(pix_a,pix_b,pix_c);

	PixelPair iter = equal_range(search_begin,pix.end(),pix_a,
				     Pixel::SuperPixelBasedOrder);
	if ((iter.first != iter.second) &&
	    (Pixel::WeightMatch(pix[i],*iter.first))) {
	  found_cohort = true;
	} else {
	  found_cohort = false;
	}

	if (found_cohort) {
          iter = equal_range(search_begin,pix.end(),pix_b,
                             Pixel::SuperPixelBasedOrder);
          if ((iter.first != iter.second) &&
              (Pixel::WeightMatch(pix[i],*iter.first))) {
	    found_cohort = true;
	  } else {
	    found_cohort = false;
	  }
	}

	if (found_cohort) {
          iter = equal_range(search_begin,pix.end(),pix_c,
                             Pixel::SuperPixelBasedOrder);
          if ((iter.first != iter.second) &&
              (Pixel::WeightMatch(pix[i],*iter.first))) {
	    found_cohort = true;
	  } else {
	    found_cohort = false;
	  }
	}

	if (found_cohort) {
	  pix_a = pix[i];
	  pix_a.SetToSuperPix(pix_a.Resolution()/2);
	  unique_pix.push_back(pix_a);
	} else {
	  unique_pix.push_back(pix[i]);
	}
      } else {
	unique_pix.push_back(pix[i]);
      }
      ++search_begin;
    }

    if (unique_pix.size() != pix.size()) {
      std::cout <<
	"Something has gone wrong searching for superpixels. Exiting.\n";
      exit(1);
    }

    for (uint32_t i=0;i<unique_pix.size();i++) pix[i] = unique_pix[i];
    unique_pix.clear();

    Pixel::FindUniquePixels(pix, unique_pix);

    n_finish = unique_pix.size();

    pix.clear();
    pix.reserve(unique_pix.size());
    for (uint32_t i=0;i<unique_pix.size();i++)
      pix.push_back(unique_pix[i]);

    unique_pix.clear();
  }
}

void Pixel::FindUniquePixels(PixelVector& input_pix, PixelVector& unique_pix) {
  if (!unique_pix.empty()) unique_pix.clear();
  sort(input_pix.begin(), input_pix.end(), Pixel::SuperPixelBasedOrder);

  PixelIterator search_end = input_pix.begin();
  unique_pix.push_back(input_pix[0]);
  ++search_end;
  // First, we iterate through the pixels, checking to see if copies of a given
  // pixel exist (we take the first such instance) or if superpixels which
  // contain the given pixel exist (we always take larger pixels).  If neither
  // case is met, then we keep the pixel.
  for (uint32_t i=1;i<input_pix.size();i++) {
    bool keep_pixel = true;

    // Check against copies of the current pixel.
    if (Pixel::WeightedPixelMatch(input_pix[i], input_pix[i-1]))
      keep_pixel = false;

    if ((keep_pixel) && (input_pix[i].Resolution() > Stomp::HPixResolution)) {
      Pixel tmp_pix = input_pix[i];

      // Check for larger pixels which might contain the current pixel.
      while (tmp_pix.Resolution() > Stomp::HPixResolution) {
        tmp_pix.SetToSuperPix(tmp_pix.Resolution()/2);

        if (binary_search(input_pix.begin(),search_end,tmp_pix,
                          Pixel::SuperPixelBasedOrder))
          keep_pixel = false;
      }
    }

    if (keep_pixel) unique_pix.push_back(input_pix[i]);
    ++search_end;
  }
}

void Pixel::Ang2HPix(uint16_t input_resolution, AngularCoordinate& ang,
		     uint32_t& output_hpixnum,
		     uint32_t& output_superpixnum) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  double eta = (ang.Eta() - Stomp::EtaOffSet)*Stomp::DegToRad;

  if (eta <= 0.0) eta += 2.0*Stomp::Pi;

  eta /= 2.0*Stomp::Pi;
  uint32_t x = static_cast<uint32_t>(nx*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::DegToRad;

  uint32_t y;
  if (lambda >= Stomp::Pi) {
    y = ny - 1;
  } else {
    y = static_cast<uint32_t>(ny*((1.0 - cos(lambda))/2.0));
  }

  uint32_t x0 = x/hnx;
  uint32_t y0 = y/hnx;

  x -= x0*hnx;
  y -= y0*hnx;

  output_hpixnum = nx*y + x;
  output_superpixnum = Stomp::Nx0*Stomp::HPixResolution*y0 + x0;
}

void Pixel::HPix2Ang(uint16_t input_resolution, uint32_t input_hpixnum,
		     uint32_t input_superpixnum,
		     AngularCoordinate& ang) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t y0 = input_superpixnum/(Stomp::Nx0*Stomp::HPixResolution);
  uint32_t x0 = input_superpixnum - y0*Stomp::Nx0*Stomp::HPixResolution;

  y0 *= hnx;
  x0 *= hnx;

  uint32_t y = input_hpixnum/hnx;
  uint32_t x = input_hpixnum - hnx*y;

  ang.SetSurveyCoordinates(90.0-Stomp::RadToDeg*acos(1.0-2.0*(y+y0+0.5)/ny),
			   Stomp::RadToDeg*(2.0*Stomp::Pi*(x+x0+0.5))/nx +
			   Stomp::EtaOffSet);
}

void Pixel::XY2HPix(uint16_t input_resolution, uint32_t x,
		    uint32_t y, uint32_t& output_hpixnum,
		    uint32_t& output_superpixnum) {
  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t x0 = x/hnx;
  uint32_t y0 = y/hnx;

  x -= x0;
  y -= y0;

  output_hpixnum = hnx*y + x;
  output_superpixnum = Stomp::Nx0*Stomp::HPixResolution*y0 + x0;
}

void Pixel::HPix2XY(uint16_t input_resolution, uint32_t input_hpixnum,
		    uint32_t input_superpixnum,
		    uint32_t& x, uint32_t& y) {
  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t y0 = input_superpixnum/(Stomp::Nx0*Stomp::HPixResolution);
  uint32_t x0 = input_superpixnum - y0*Stomp::Nx0*Stomp::HPixResolution;

  uint32_t tmp_y = input_hpixnum/hnx;
  uint32_t tmp_x = input_hpixnum - hnx*tmp_y;

  x = tmp_x + x0*hnx;
  y = tmp_y + y0*hnx;
}

void Pixel::SuperHPix(uint16_t hi_resolution, uint32_t hi_hpixnum,
		      uint16_t lo_resolution, uint32_t& lo_hpixnum) {
  if (hi_resolution < lo_resolution) {
    std::cout << "Can't go from low resolution to higher resolution.\n ";
    exit(1);
  } else {
    uint32_t nx_hi = hi_resolution/Stomp::HPixResolution;
    uint32_t nx_lo = lo_resolution/Stomp::HPixResolution;

    uint16_t ratio = hi_resolution/lo_resolution;

    uint32_t y = hi_hpixnum/nx_hi;
    uint32_t x = hi_hpixnum - nx_hi*y;

    x /= ratio;
    y /= ratio;

    lo_hpixnum = nx_lo*y + x;
  }
}

void Pixel::NextSubHPix(uint16_t input_resolution, uint32_t input_hpixnum,
			uint32_t& sub_hpixnum1,
			uint32_t& sub_hpixnum2,
			uint32_t& sub_hpixnum3,
			uint32_t& sub_hpixnum4) {
  uint32_t nx_hi = 2*input_resolution/Stomp::HPixResolution;
  uint32_t nx_lo = input_resolution/Stomp::HPixResolution;

  uint32_t y = input_hpixnum/nx_lo;
  uint32_t x = input_hpixnum - nx_lo*y;

  sub_hpixnum1 = nx_hi*(2*y) + 2*x;
  sub_hpixnum2 = nx_hi*(2*y) + 2*x + 1;
  sub_hpixnum3 = nx_hi*(2*y + 1) + 2*x;
  sub_hpixnum4 = nx_hi*(2*y + 1) + 2*x + 1;
}

void Pixel::SubHPix(uint16_t lo_resolution, uint32_t lo_hpixnum,
		    uint32_t lo_superpixnum, uint16_t hi_resolution,
		    uint32_t& x_min, uint32_t& x_max,
		    uint32_t& y_min, uint32_t& y_max) {
  uint32_t tmp_x, tmp_y;

  if (lo_resolution == hi_resolution) {
    HPix2XY(lo_resolution,lo_hpixnum,lo_superpixnum,tmp_x,tmp_y);

    y_min = tmp_y;
    y_max = tmp_y;
    x_min = tmp_x;
    x_max = tmp_x;
  } else {
    uint32_t tmp_hpixnum, hpixnum1, hpixnum2, hpixnum3, hpixnum4;
    uint16_t tmp_res;

    tmp_hpixnum = lo_hpixnum;
    for (tmp_res=lo_resolution;tmp_res<hi_resolution;tmp_res*=2) {
      NextSubHPix(tmp_res, tmp_hpixnum, hpixnum1,
		  hpixnum2, hpixnum3, hpixnum4);
      tmp_hpixnum = hpixnum1;
    }

    HPix2XY(hi_resolution,tmp_hpixnum,lo_superpixnum,tmp_x,tmp_y);

    y_min = tmp_y;
    x_min = tmp_x;

    tmp_hpixnum = lo_hpixnum;
    for (tmp_res=lo_resolution;tmp_res<hi_resolution;tmp_res*=2) {
      NextSubHPix(tmp_res, tmp_hpixnum, hpixnum1,
		  hpixnum2, hpixnum3, hpixnum4);
      tmp_hpixnum = hpixnum4;
    }

    HPix2XY(hi_resolution,tmp_hpixnum,lo_superpixnum,tmp_x,tmp_y);

    y_max = tmp_y;
    x_max = tmp_x;
  }
}

void Pixel::HPixelBound(uint16_t input_resolution, uint32_t input_hpixnum,
			uint32_t input_superpixnum,
			double& lammin, double& lammax,
			double& etamin, double& etamax) {
  uint32_t nx = Stomp::Nx0*input_resolution;
  uint32_t ny = Stomp::Ny0*input_resolution;

  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t y0 = input_superpixnum/(Stomp::Nx0*Stomp::HPixResolution);
  uint32_t x0 = input_superpixnum - y0*Stomp::Nx0*Stomp::HPixResolution;

  y0 *= hnx;
  x0 *= hnx;

  uint32_t y = input_hpixnum/hnx;
  uint32_t x = input_hpixnum - hnx*y;

  lammin = 90.0 - Stomp::RadToDeg*acos(1.0 - 2.0*(y+y0+1)/ny);
  lammax = 90.0 - Stomp::RadToDeg*acos(1.0 - 2.0*(y+y0)/ny);
  etamin =
    Stomp::RadToDeg*2.0*Stomp::Pi*(x+x0+0.0)/nx + Stomp::EtaOffSet;
  if (etamin >= 180.0) etamin = etamin - 360.0;
  etamax =
    Stomp::RadToDeg*2.0*Stomp::Pi*(x+x0+1.0)/nx + Stomp::EtaOffSet;
  if (etamax >= 180.0) etamax = etamax - 360.0;
}

void Pixel::CohortHPix(uint16_t input_resolution, uint32_t input_hpixnum,
		       uint32_t& co_hpixnum1,
		       uint32_t& co_hpixnum2,
		       uint32_t& co_hpixnum3) {
  uint32_t tmp_hpixnum, hpixnum1, hpixnum2, hpixnum3, hpixnum4;

  SuperHPix(input_resolution, input_hpixnum, input_resolution/2, tmp_hpixnum);

  NextSubHPix(input_resolution/2, tmp_hpixnum,
	      hpixnum1, hpixnum2, hpixnum3, hpixnum4);

  if (input_hpixnum == hpixnum1) {
    co_hpixnum1 = hpixnum2;
    co_hpixnum2 = hpixnum3;
    co_hpixnum3 = hpixnum4;
  }
  if (input_hpixnum == hpixnum2) {
    co_hpixnum1 = hpixnum1;
    co_hpixnum2 = hpixnum3;
    co_hpixnum3 = hpixnum4;
  }
  if (input_hpixnum == hpixnum3) {
    co_hpixnum1 = hpixnum1;
    co_hpixnum2 = hpixnum2;
    co_hpixnum3 = hpixnum4;
  }
  if (input_hpixnum == hpixnum4) {
    co_hpixnum1 = hpixnum1;
    co_hpixnum2 = hpixnum2;
    co_hpixnum3 = hpixnum3;
  }
}

uint8_t Pixel::HPix2EtaStep(uint16_t input_resolution, uint32_t input_hpixnum,
			    uint32_t input_superpixnum, double theta) {

  uint32_t ny = Stomp::Ny0*input_resolution;
  uint16_t hnx = input_resolution/Stomp::HPixResolution;

  uint32_t y0 = input_superpixnum/(Stomp::Nx0*Stomp::HPixResolution);
  uint32_t x0 = input_superpixnum - y0*Stomp::Nx0*Stomp::HPixResolution;

  y0 *= hnx;
  x0 *= hnx;

  uint32_t y = input_hpixnum/hnx;
  double lam = 90.0-Stomp::RadToDeg*acos(1.0-2.0*(y+y0+0.5)/ny);
  double deta = 2.5/(input_resolution/4);

  double eta_step = theta;
  eta_step *=  1.0 +
    lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));
  uint8_t etastep = 1;

  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

ScalarPixel::ScalarPixel() {
  intensity_ = 0.0;
  n_point_ = 0;
}

ScalarPixel::ScalarPixel(const uint16_t input_resolution,
			 const uint32_t input_pixnum,
			 const double input_weight,
			 const double input_intensity,
			 const uint32_t n_point) {
  SetResolution(input_resolution);

  uint32_t tmp_y = input_pixnum/(Stomp::Nx0*Resolution());
  uint32_t tmp_x = input_pixnum - Stomp::Nx0*Resolution()*tmp_y;

  SetPixnumFromXY(tmp_x, tmp_y);
  SetWeight(input_weight);
  intensity_ = input_intensity;
  n_point_ = n_point;
}

ScalarPixel::ScalarPixel(const uint16_t input_resolution,
			 const uint32_t input_hpixnum,
			 const uint32_t input_superpixnum,
			 const double input_weight,
			 const double input_intensity,
			 const uint32_t n_point) {
  SetResolution(input_resolution);

  uint32_t tmp_x, tmp_y;
  Pixel::HPix2XY(input_resolution, input_hpixnum, input_superpixnum,
		 tmp_x, tmp_y);

  SetPixnumFromXY(tmp_x, tmp_y);
  SetWeight(input_weight);
  intensity_ = input_intensity;
  n_point_ = n_point;
}

ScalarPixel::ScalarPixel(const uint32_t input_x,
			 const uint32_t input_y,
			 const uint16_t input_resolution,
			 const double input_weight,
			 const double input_intensity,
			 const uint32_t n_point) {
  SetResolution(input_resolution);
  SetPixnumFromXY(input_x, input_y);
  SetWeight(input_weight);
  intensity_ = input_intensity;
  n_point_ = n_point;
}

ScalarPixel::ScalarPixel(AngularCoordinate& ang,
			 const uint16_t input_resolution,
			 const double input_weight,
			 const double input_intensity,
			 const uint32_t n_point) {

  SetResolution(input_resolution);
  SetPixnumFromAng(ang);
  SetWeight(input_weight);
  intensity_ = input_intensity;
  n_point_ = n_point;
}

ScalarPixel::~ScalarPixel() {
  intensity_ = 0.0;
  n_point_ = 0;
}

TreePixel::TreePixel() {
  SetWeight(0.0);
  maximum_points_ = 0;
  point_count_ = 0;
  InitializeCorners();
}

TreePixel::TreePixel(const uint16_t input_resolution,
		     const uint32_t input_pixnum,
		     const uint16_t maximum_points) {
  SetResolution(input_resolution);

  uint32_t tmp_y = input_pixnum/(Stomp::Nx0*Resolution());
  uint32_t tmp_x = input_pixnum - Stomp::Nx0*Resolution()*tmp_y;

  SetPixnumFromXY(tmp_x, tmp_y);
  SetWeight(0.0);
  maximum_points_ = maximum_points;
  point_count_ = 0;
  initialized_subpixels_ = false;
  InitializeCorners();
}

TreePixel::TreePixel(const uint16_t input_resolution,
		     const uint32_t input_hpixnum,
		     const uint32_t input_superpixnum,
		     const uint16_t maximum_points) {
  SetResolution(input_resolution);
  uint32_t tmp_x, tmp_y;
  Pixel::HPix2XY(input_resolution, input_hpixnum, input_superpixnum,
		 tmp_x, tmp_y);

  SetPixnumFromXY(tmp_x, tmp_y);
  SetWeight(0.0);
  maximum_points_ = maximum_points;
  point_count_ = 0;
  initialized_subpixels_ = false;
  InitializeCorners();
}

TreePixel:: TreePixel(const uint32_t input_x,
		      const uint32_t input_y,
		      const uint16_t input_resolution,
		      const uint16_t maximum_points) {
  SetResolution(input_resolution);
  SetPixnumFromXY(input_x, input_y);
  SetWeight(0.0);
  maximum_points_ = maximum_points;
  point_count_ = 0;
  initialized_subpixels_ = false;
  InitializeCorners();
}

TreePixel::TreePixel(AngularCoordinate& ang,
		     const uint16_t input_resolution,
		     const uint16_t maximum_points) {
  SetResolution(input_resolution);
  SetPixnumFromAng(ang);
  SetWeight(0.0);
  maximum_points_ = maximum_points;
  point_count_ = 0;
  initialized_subpixels_ = false;
  InitializeCorners();
}

TreePixel::~TreePixel() {
  ang_.clear();
  subpix_.clear();
  maximum_points_ = 0;
  point_count_ = 0;
  initialized_subpixels_ = false;
}

void TreePixel::InitializeCorners() {
  unit_sphere_x_ = -1.0*sin(Lambda()*Stomp::DegToRad);
  unit_sphere_y_ = cos(Lambda()*Stomp::DegToRad)*
    cos(Eta()*Stomp::DegToRad+Stomp::EtaPole);
  unit_sphere_z_ = cos(Lambda()*Stomp::DegToRad)*
    sin(Eta()*Stomp::DegToRad+Stomp::EtaPole);

  unit_sphere_x_ul_ = -1.0*sin(LambdaMax()*Stomp::DegToRad);
  unit_sphere_y_ul_ = cos(LambdaMax()*Stomp::DegToRad)*
    cos(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  unit_sphere_z_ul_ = cos(LambdaMax()*Stomp::DegToRad)*
    sin(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);

  unit_sphere_x_ur_ = -1.0*sin(LambdaMax()*Stomp::DegToRad);
  unit_sphere_y_ur_ = cos(LambdaMax()*Stomp::DegToRad)*
    cos(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  unit_sphere_z_ur_ = cos(LambdaMax()*Stomp::DegToRad)*
    sin(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);

  unit_sphere_x_ll_ = -1.0*sin(LambdaMin()*Stomp::DegToRad);
  unit_sphere_y_ll_ = cos(LambdaMin()*Stomp::DegToRad)*
    cos(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  unit_sphere_z_ll_ = cos(LambdaMin()*Stomp::DegToRad)*
    sin(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);

  unit_sphere_x_lr_ = -1.0*sin(LambdaMin()*Stomp::DegToRad);
  unit_sphere_y_lr_ = cos(LambdaMin()*Stomp::DegToRad)*
    cos(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  unit_sphere_z_lr_ = cos(LambdaMin()*Stomp::DegToRad)*
    sin(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
}

bool TreePixel::AddPoint(WeightedAngularCoordinate* ang) {
  bool added_to_pixel = false;
  if (Contains(*ang)) {
    if ((point_count_ < maximum_points_) ||
	(Resolution() == Stomp::MaxPixelResolution)) {
      if (point_count_ == 0) ang_.reserve(maximum_points_);
      ang_.push_back(ang);
      added_to_pixel = true;
    } else {
      if (!initialized_subpixels_) {
	if (!_InitializeSubPixels()) {
	  std::cout << "Failed to initialize sub-pixels.  Exiting.\n";
	  exit(2);
	}
      }
      for (uint32_t i=0;i<subpix_.size();++i) {
	if (subpix_[i]->Contains(*ang)) {
	  added_to_pixel = subpix_[i]->AddPoint(ang);
	  i = subpix_.size();
	}
      }
    }
  } else {
    added_to_pixel = false;
  }

  if (added_to_pixel) {
    AddToWeight(ang->Weight());
    if (ang->HasFields()) {
      for (FieldIterator iter=ang->FieldBegin();iter!=ang->FieldEnd();++iter) {
	if (field_total_.find(iter->first) != field_total_.end()) {
	  field_total_[iter->first] += iter->second;
	} else {
	  field_total_[iter->first] = iter->second;
	}
      }
    }
    point_count_++;
  }

  return added_to_pixel;
}

bool TreePixel::_InitializeSubPixels() {
  initialized_subpixels_ = false;
  // If we're already at the maximum resolution, then we shouldn't be trying
  // to generate sub-pixels.
  if (Resolution() < Stomp::MaxPixelResolution) {
    PixelVector tmp_pix;
    SubPix(Resolution()*2, tmp_pix);
    subpix_.reserve(4);

    // Provided we passed that test, we create a vector of sub-pixels.
    for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter) {
      TreePixel* tree_pix = new TreePixel(iter->PixelX(), iter->PixelY(),
					  iter->Resolution(), maximum_points_);
      subpix_.push_back(tree_pix);
    }
    initialized_subpixels_ = true;

    // Now we iterate over all of the AngularCoordinates in the current pixel.
    // Provided that we find a home for all of them, we return true.  If any
    // of them fail to fit into a sub-pixel, then we return false.
    bool transferred_point_to_subpixels = false;
    for (uint32_t i=0;i<ang_.size();++i) {
      transferred_point_to_subpixels = false;
      for (uint32_t j=0;j<subpix_.size();++j) {
	if (subpix_[j]->AddPoint(ang_[i])) {
	  j = subpix_.size();
	  transferred_point_to_subpixels = true;
	}
      }
      if (!transferred_point_to_subpixels) initialized_subpixels_ = false;
    }
    ang_.clear();
  }

  return initialized_subpixels_;
}

uint32_t TreePixel::DirectPairCount(AngularCoordinate& ang,
				    AngularBin& theta,
				    int16_t region) {
  uint32_t pair_count = 0;
  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(ang))) pair_count++;
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(ang))) pair_count++;
    }
  }

  theta.AddToCounter(pair_count, region);
  return pair_count;
}

uint32_t TreePixel::FindPairs(AngularCoordinate& ang, AngularBin& theta,
			      int16_t region) {
  uint32_t pair_count = 0;

  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    pair_count = DirectPairCount(ang, theta, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(ang, theta);

    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      pair_count = point_count_;
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  pair_count += (*iter)->FindPairs(ang, theta, region);
	}
      } else {
	// Completely outside the annulus.
	pair_count = 0;
      }
    }
  }
  return pair_count;
}

uint32_t TreePixel::FindPairs(AngularCoordinate& ang,
			      double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindPairs(ang, theta);
}

uint32_t TreePixel::FindPairs(AngularCoordinate& ang, double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindPairs(ang, theta);
}

double TreePixel::DirectWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
				      int16_t region) {
  double total_weight = 0.0;
  uint32_t n_pairs = 0;

  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(ang))) {
	total_weight += (*iter)->Weight();
	n_pairs++;
      }
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(ang))) {
	total_weight += (*iter)->Weight();
	n_pairs++;
      }
    }
  }

  theta.AddToWeight(total_weight, region);
  theta.AddToCounter(n_pairs, region);

  return total_weight;
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
				    int16_t region) {
  double total_weight = 0.0;
  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    total_weight = DirectWeightedPairs(ang, theta, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(ang, theta);
    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      total_weight = Weight();
      theta.AddToWeight(Weight(), region);
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  total_weight += (*iter)->FindWeightedPairs(ang, theta, region);
	}
      } else {
	// Completely outside the annulus.
	total_weight = 0.0;
      }
    }
  }
  return total_weight;
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang,
				    double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(ang, theta);
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang, double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(ang, theta);
}

double TreePixel::DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
				      AngularBin& theta, int16_t region) {
  double total_weight = 0.0;
  uint32_t n_pairs = 0;

  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(w_ang))) {
	total_weight += (*iter)->Weight();
	n_pairs++;
      }
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(w_ang))) {
	total_weight += (*iter)->Weight();
	n_pairs++;
      }
    }
  }

  total_weight *= w_ang.Weight();

  theta.AddToWeight(total_weight, region);
  theta.AddToCounter(n_pairs, region);

  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    AngularBin& theta, int16_t region) {
  double total_weight = 0.0;
  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    total_weight = DirectWeightedPairs(w_ang, theta, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(w_ang, theta);
    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      total_weight = Weight()*w_ang.Weight();
      theta.AddToWeight(Weight(), region);
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  total_weight += (*iter)->FindWeightedPairs(w_ang, theta, region);
	}
      } else {
	// Completely outside the annulus.
	total_weight = 0.0;
      }
    }
  }
  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, theta);
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, theta);
}

void TreePixel::FindPairs(AngularVector& ang, AngularBin& theta,
			  int16_t region) {
  uint32_t n_pairs = 0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    n_pairs = FindPairs(*ang_iter, theta, region);
  }
}

void TreePixel::FindPairs(AngularVector& ang, AngularCorrelation& wtheta,
			  int16_t region) {
  uint32_t n_pairs = 0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      n_pairs = FindPairs(*ang_iter, *theta_iter, region);
    }
  }
}

void TreePixel::FindWeightedPairs(AngularVector& ang, AngularBin& theta,
				  int16_t region) {
  double total_weight = 0.0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, region);
  }
}

void TreePixel::FindWeightedPairs(AngularVector& ang,
				  AngularCorrelation& wtheta,
				  int16_t region) {
  double total_weight = 0.0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter, region);
    }
  }
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta,
				  int16_t region) {
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, region);
  }
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang,
				  AngularCorrelation& wtheta, int16_t region) {
  double total_weight = 0.0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter, region);
    }
  }
}

double TreePixel::DirectWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
				      const std::string& field_name,
				      int16_t region) {
  double total_weight = 0.0;
  uint32_t n_pairs = 0;

  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  }

  theta.AddToWeight(total_weight, region);
  theta.AddToCounter(n_pairs, region);

  return total_weight;
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
				    const std::string& field_name,
				    int16_t region) {
  double total_weight = 0.0;
  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    total_weight = DirectWeightedPairs(ang, theta, field_name, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(ang, theta);
    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      total_weight = FieldTotal(field_name);
      theta.AddToWeight(total_weight, region);
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  total_weight +=
	    (*iter)->FindWeightedPairs(ang, theta, field_name, region);
	}
      } else {
	// Completely outside the annulus.
	total_weight = 0.0;
      }
    }
  }
  return total_weight;
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang,
				    double theta_min, double theta_max,
				    const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(ang, theta, field_name);
}

double TreePixel::FindWeightedPairs(AngularCoordinate& ang, double theta_max,
				    const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(ang, theta, field_name);
}

void TreePixel::FindWeightedPairs(AngularVector& ang, AngularBin& theta,
				  const std::string& field_name,
				  int16_t region) {
  double total_weight = 0.0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, field_name, region);
  }
}

void TreePixel::FindWeightedPairs(AngularVector& ang,
				  AngularCorrelation& wtheta,
				  const std::string& field_name, int16_t region) {
  double total_weight = 0.0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      total_weight =
	FindWeightedPairs(*ang_iter, *theta_iter, field_name, region);
    }
  }
}

double TreePixel::DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
				      AngularBin& theta,
				      const std::string& field_name,
				      int16_t region) {
  double total_weight = 0.0;
  uint32_t n_pairs = 0;

  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(w_ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(w_ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  }

  total_weight *= w_ang.Weight();

  theta.AddToWeight(total_weight, region);
  theta.AddToCounter(n_pairs, region);

  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    AngularBin& theta,
				    const std::string& field_name,
				    int16_t region) {
  double total_weight = 0.0;
  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    total_weight = DirectWeightedPairs(w_ang, theta, field_name, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(w_ang, theta);
    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      total_weight = FieldTotal(field_name)*w_ang.Weight();
      theta.AddToWeight(total_weight, region);
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  total_weight +=
	    (*iter)->FindWeightedPairs(w_ang, theta, field_name, region);
	}
      } else {
	// Completely outside the annulus.
	total_weight = 0.0;
      }
    }
  }
  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    double theta_min, double theta_max,
				    const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, theta, field_name);
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    double theta_max,
				    const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, theta, field_name);
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang,
				  AngularBin& theta,
				  const std::string& field_name,
				  int16_t region) {
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, field_name, region);
  }
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang,
				  AngularCorrelation& wtheta,
				  const std::string& field_name,
				  int16_t region) {
  double total_weight = 0.0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight =
	FindWeightedPairs(*ang_iter, *theta_iter, field_name, region);
    }
  }
}

double TreePixel::DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
				      const std::string& ang_field_name,
				      AngularBin& theta,
				      const std::string& field_name,
				      int16_t region) {
  double total_weight = 0.0;
  uint32_t n_pairs = 0;

  if (theta.ThetaMax() < 90.0) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinCosBounds((*iter)->DotProduct(w_ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  } else {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      if (theta.WithinBounds((*iter)->AngularDistance(w_ang))) {
	total_weight += (*iter)->Field(field_name);
	n_pairs++;
      }
    }
  }

  total_weight *= w_ang.Field(ang_field_name);

  theta.AddToWeight(total_weight, region);
  theta.AddToCounter(n_pairs, region);

  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    const std::string& ang_field_name,
				    AngularBin& theta,
				    const std::string& field_name,
				    int16_t region) {
  double total_weight = 0.0;
  // If we have AngularCoordinates in this pixel, then this is just a
  // matter of iterating through them and finding how many satisfy the
  // angular bounds.
  if (!ang_.empty()) {
    total_weight =
      DirectWeightedPairs(w_ang, ang_field_name, theta, field_name, region);
  } else {
    // If the current pixel doesn't contain any points, then we need to see
    // if either the current pixel is either fully or partially contained in
    // the annulus.  For the former case, we can just send back the total
    // number of points in this pixel.  In the latter case, we pass things
    // along to the sub-pixels.  If neither of those things are true, then
    // we're done and we send back zero.
    int8_t intersects_annulus = _IntersectsAnnulus(w_ang, theta);
    if (intersects_annulus == 1) {
      // Fully contained in the annulus.
      total_weight = FieldTotal(field_name)*w_ang.Field(ang_field_name);
      theta.AddToWeight(total_weight, region);
      theta.AddToCounter(point_count_, region);
    } else {
      if (intersects_annulus == -1) {
      // Partial intersection with the annulus.
	for (TreePtrIterator iter=subpix_.begin();
	     iter!=subpix_.end();++iter) {
	  total_weight +=
	    (*iter)->FindWeightedPairs(w_ang, ang_field_name, theta,
				       field_name, region);
	}
      } else {
	// Completely outside the annulus.
	total_weight = 0.0;
      }
    }
  }
  return total_weight;
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    const std::string& ang_field_name,
				    double theta_min, double theta_max,
				    const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, ang_field_name, theta, field_name);
}

double TreePixel::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				    const std::string& ang_field_name,
				    double theta_max,
				    const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, ang_field_name, theta, field_name);
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang,
				  const std::string& ang_field_name,
				  AngularBin& theta,
				  const std::string& field_name,
				  int16_t region) {
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, ang_field_name, theta,
				     field_name, region);
  }
}

void TreePixel::FindWeightedPairs(WAngularVector& w_ang,
				  const std::string& ang_field_name,
				  AngularCorrelation& wtheta,
				  const std::string& field_name,
				  int16_t region) {
  double total_weight = 0.0;
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight =
	FindWeightedPairs(*ang_iter, ang_field_name, *theta_iter,
			  field_name, region);
    }
  }
}

uint16_t TreePixel::FindKNearestNeighbors(AngularCoordinate& ang,
					  uint8_t n_neighbors,
					  WAngularVector& neighbor_ang) {
  TreeNeighbor neighbors(ang, n_neighbors);

  _NeighborRecursion(ang, neighbors);

  neighbors.NearestNeighbors(neighbor_ang, false);

  return neighbors.NodesVisited();
}

uint16_t TreePixel::FindNearestNeighbor(AngularCoordinate& ang,
					WeightedAngularCoordinate& nbr_ang) {
  WAngularVector angVec;

  uint16_t nodes_visited = FindKNearestNeighbors(ang, 1, angVec);

  nbr_ang = angVec[0];

  return nodes_visited;
}

double TreePixel::KNearestNeighborDistance(AngularCoordinate& ang,
					   uint8_t n_neighbors,
					   uint16_t& nodes_visited) {

  TreeNeighbor neighbors(ang, n_neighbors);

  _NeighborRecursion(ang, neighbors);

  nodes_visited = neighbors.NodesVisited();

  return neighbors.MaxAngularDistance();
}

void TreePixel::_NeighborRecursion(AngularCoordinate& ang,
				   TreeNeighbor& neighbors) {

  neighbors.AddNode();

  if (!ang_.empty()) {
    // We have no sub-nodes in this tree, so we'll just iterate over the
    // points here and take the nearest N neighbors.
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter)
      neighbors.TestPoint(*iter);
  } else {
    // This node is the root node for our tree, so we first find the sub-node
    // that contains the point and start recursing there.
    //
    // While we iterate through the nodes, we'll also calculate the edge
    // distances for those nodes that don't contain the point and store them
    // in a priority queue.  This will let us do a follow-up check on nodes in
    // the most productive order.
    PixelQueue pix_queue;
    for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
      if ((*iter)->Contains(ang)) {
	(*iter)->_NeighborRecursion(ang, neighbors);
      } else {
	double min_edge_distance, max_edge_distance;
	(*iter)->_EdgeDistances(ang, min_edge_distance, max_edge_distance);
	DistancePixelPair dist_pair(min_edge_distance, (*iter));
	pix_queue.push(dist_pair);
      }
    }

    // That should give us back a TreeNeighbor object that contains a workable
    // set of neighbors and a search radius for possible matches.  Now we just
    // need to iterate over those sub-nodes that didn't contain the input point
    // to verify that there can't be any points in their sub-nodes which might
    // be closer to the input point.
    //
    // There's also the possibility that the input point is completely outside
    // our tree.  In that case (where the number of neighbors in the
    // TreeNeighbor object is less than the maximum), we want to check
    // all nodes.
    while (!pix_queue.empty()) {
      double pix_distance = pix_queue.top().first;
      TreePixel* pix_iter = pix_queue.top().second;
      if (pix_distance < neighbors.MaxDistance()) {
	pix_iter->_NeighborRecursion(ang, neighbors);
      }
      pix_queue.pop();
    }
  }
}

int8_t TreePixel::_IntersectsAnnulus(AngularCoordinate& ang,
				     AngularBin& theta) {
  // By default, we assume that there is no intersection between the disk
  // and the pixel.
  int8_t intersects_annulus = 0;

  double costheta =
    ang.UnitSphereX()*unit_sphere_x_ +
    ang.UnitSphereY()*unit_sphere_y_ +
    ang.UnitSphereZ()*unit_sphere_z_;
  double costheta_max = costheta;
  double costheta_min = costheta;

  double costheta_ul =
    ang.UnitSphereX()*unit_sphere_x_ul_ +
    ang.UnitSphereY()*unit_sphere_y_ul_ +
    ang.UnitSphereZ()*unit_sphere_z_ul_;
  if (costheta_ul > costheta_max) costheta_max = costheta_ul;
  if (costheta_ul < costheta_min) costheta_min = costheta_ul;

  double costheta_ll =
    ang.UnitSphereX()*unit_sphere_x_ll_ +
    ang.UnitSphereY()*unit_sphere_y_ll_ +
    ang.UnitSphereZ()*unit_sphere_z_ll_;
  if (costheta_ll > costheta_max) costheta_max = costheta_ll;
  if (costheta_ll < costheta_min) costheta_min = costheta_ll;

  double costheta_ur =
    ang.UnitSphereX()*unit_sphere_x_ur_ +
    ang.UnitSphereY()*unit_sphere_y_ur_ +
    ang.UnitSphereZ()*unit_sphere_z_ur_;
  if (costheta_ur > costheta_max) costheta_max = costheta_ur;
  if (costheta_ur < costheta_min) costheta_min = costheta_ur;

  double costheta_lr =
    ang.UnitSphereX()*unit_sphere_x_lr_ +
    ang.UnitSphereY()*unit_sphere_y_lr_ +
    ang.UnitSphereZ()*unit_sphere_z_lr_;
  if (costheta_lr > costheta_max) costheta_max = costheta_lr;
  if (costheta_lr < costheta_min) costheta_min = costheta_lr;

  double near_corner_distance = 1.0 - costheta_max*costheta_max;
  double far_corner_distance = 1.0 - costheta_min*costheta_min;
  double near_edge_distance;
  double far_edge_distance;

  if (!_EdgeDistances(ang, near_edge_distance, far_edge_distance)) {
    near_edge_distance = near_corner_distance;
    far_edge_distance = far_corner_distance;
  }

  bool contains_center = Contains(ang);

  // First we tackle the inner disk.
  int8_t intersects_inner_disk = 0;

  if (Stomp::DoubleGE(near_edge_distance, theta.Sin2ThetaMin())) {
    // If this is true, it means that the distance between the nearest edge
    // and the annulus center is greater than the inner annulus radius.  This
    // means that the inner disk is either completely inside or outside the
    // pixel.  Checking the center should tell us which is which.
    if (contains_center) {
      // disk is inside pixel.
      intersects_inner_disk = -1;
    } else {
      // disk is outside pixel.
      intersects_inner_disk = 0;
    }
  } else {
    // If the distance to the nearest edge is less than the inner annulus
    // radius, then there is some intersection between the two; either the
    // disk intersects the pixel edge or it completely contains the pixel.
    // Checking the corners will tell is which is which.
    if (Stomp::DoubleGE(costheta_ul, theta.CosThetaMax()) &&
	Stomp::DoubleGE(costheta_ur, theta.CosThetaMax()) &&
	Stomp::DoubleGE(costheta_ll, theta.CosThetaMax()) &&
	Stomp::DoubleGE(costheta_lr, theta.CosThetaMax())) {
      // pixel is inside the disk.
      intersects_inner_disk = 1;
    } else {
      // pixel intersects the disk.
      intersects_inner_disk = -1;
    }
  }

  // Now, the outer disk.
  int8_t intersects_outer_disk = 0;

  if (Stomp::DoubleGE(near_edge_distance, theta.Sin2ThetaMax())) {
    // If this is true, it means that the distance between the nearest edge
    // and the annulus center is greater than the outer annulus radius.  This
    // means that the outer disk is either completely inside or outside the
    // pixel.  Checking the center should tell us which is which.
    if (contains_center) {
      // disk is inside pixel.
      intersects_outer_disk = -1;
    } else {
      // disk is outside pixel.
      intersects_outer_disk = 0;
    }
  } else {
    // If the distance to the nearest edge is less than the outer annulus
    // radius, then there is some intersection between the two; either the
    // disk intersects the pixel edge or it completely contains the pixel.
    // Checking the corners will tell is which is which.
    if (Stomp::DoubleGE(costheta_ul, theta.CosThetaMin()) &&
	Stomp::DoubleGE(costheta_ur, theta.CosThetaMin()) &&
	Stomp::DoubleGE(costheta_ll, theta.CosThetaMin()) &&
	Stomp::DoubleGE(costheta_lr, theta.CosThetaMin())) {
      // pixel is inside the disk.
      intersects_outer_disk = 1;
    } else {
      // pixel intersects the disk.
      intersects_outer_disk = -1;
    }
  }

  // Now we deal with cases.
  if ((intersects_inner_disk == 1) && (intersects_outer_disk == 1)) {
    // This means that the pixel is contained by the inner and outer disks.
    // Hence, the pixel is in the hole of the annulus.
    intersects_annulus = 0;
  }

  if ((intersects_inner_disk == -1) && (intersects_outer_disk == 1)) {
    // This means that the inner disk shares some area with the pixel and the
    // outer disk contains it completely.
    intersects_annulus = -1;
  }

  if ((intersects_inner_disk == 0) && (intersects_outer_disk == 1)) {
    // The inner disk is outside the pixel, but the outer disk contains it.
    // Hence, the pixel is fully inside the annulus.
    intersects_annulus = 1;
  }


  if ((intersects_inner_disk == 1) && (intersects_outer_disk == -1)) {
    // This should be impossible.  Raise an error.
    std::cout << "Impossible annulus intersection: " <<
      intersects_inner_disk << ", " << intersects_outer_disk << ".  Bailing.\n";
    exit(2);
  }

  if ((intersects_inner_disk == -1) && (intersects_outer_disk == -1)) {
    // There's partial overlap with both the inner and outer disks.
    intersects_annulus = -1;
  }

  if ((intersects_inner_disk == 0) && (intersects_outer_disk == -1)) {
    // The inner disk is outside, but the outer intersects, so there's some
    // intersection between the pixel and annulus.
    intersects_annulus = -1;
  }


  if ((intersects_inner_disk == 1) && (intersects_outer_disk == 0)) {
    // This should be impossible.  Raise an error.
    std::cout << "Impossible annulus intersection: " <<
      intersects_inner_disk << ", " << intersects_outer_disk << ".  Bailing.\n";
    exit(2);
  }

  if ((intersects_inner_disk == -1) && (intersects_outer_disk == 0)) {
    // This should be impossible.  Raise an error.
    std::cout << "Impossible annulus intersection: " <<
      intersects_inner_disk << ", " << intersects_outer_disk << ".  Bailing.\n";
    exit(2);
  }

  if ((intersects_inner_disk == 0) && (intersects_outer_disk == 0)) {
    // The inner disk is outside the pixel, and so is the outer disk.
    // Hence, the pixel is fully outside the annulus.
    intersects_annulus = 0;
  }

  return intersects_annulus;
}

bool TreePixel::_EdgeDistances(AngularCoordinate& ang,
			       double& min_edge_distance,
			       double& max_edge_distance) {
  bool inside_bounds = false;
  double lambda_min = LambdaMin();
  double lambda_max = LambdaMax();
  double eta_min = EtaMin();
  double eta_max = EtaMax();
  double lam = ang.Lambda();
  double eta = ang.Eta();

  if (Stomp::DoubleLE(lam, lambda_max) && Stomp::DoubleGE(lam, lambda_min)) {
    inside_bounds = true;

    double eta_scaling = 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 - 1.28162e-11*lam*lam));

    min_edge_distance = fabs(eta - eta_min);
    max_edge_distance = fabs(eta - eta_min);

    if (min_edge_distance > fabs(eta - eta_max))
      min_edge_distance = fabs(eta - eta_max);
    if (max_edge_distance < fabs(eta - eta_max))
      max_edge_distance = fabs(eta - eta_max);

    min_edge_distance /= eta_scaling;
    max_edge_distance /= eta_scaling;
  }

  if (Stomp::DoubleLE(eta, eta_max) && Stomp::DoubleGE(eta, eta_min)) {
    if (inside_bounds) {
      if (min_edge_distance > fabs(lam - lambda_min))
	min_edge_distance = fabs(lam - lambda_min);
      if (min_edge_distance > fabs(lam - lambda_max))
	min_edge_distance = fabs(lam - lambda_max);

      if (max_edge_distance < fabs(lam - lambda_min))
	max_edge_distance = fabs(lam - lambda_min);
      if (max_edge_distance < fabs(lam - lambda_max))
	max_edge_distance = fabs(lam - lambda_max);
    } else {
      min_edge_distance = fabs(lam - lambda_min);
      max_edge_distance = fabs(lam - lambda_max);

      if (min_edge_distance > fabs(lam - lambda_max))
	min_edge_distance = fabs(lam - lambda_max);
      if (max_edge_distance < fabs(lam - lambda_max))
	max_edge_distance = fabs(lam - lambda_max);
    }
    inside_bounds = true;
  }

  if (inside_bounds) {
    // The return value for this function is (sin(theta))^2 rather than just
    // the angle theta.  If we can get by with the small angle approximation,
    // then we use that for speed purposes.
    if (min_edge_distance < 3.0) {
      min_edge_distance = min_edge_distance*Stomp::DegToRad;
    } else {
      min_edge_distance = sin(min_edge_distance*Stomp::DegToRad);
    }
    min_edge_distance *= min_edge_distance;

    if (max_edge_distance < 3.0) {
      max_edge_distance = max_edge_distance*Stomp::DegToRad;
    } else {
      max_edge_distance = sin(max_edge_distance*Stomp::DegToRad);
    }
    max_edge_distance *= max_edge_distance;
  }
  return inside_bounds;
}

uint32_t TreePixel::NPoints(Pixel& pix) {
  uint32_t total_points = 0;

  // First check to see if the input pixel contains the current pixel.
  if (pix.Contains(Resolution(), PixelX(), PixelY())) {
    // If so, then it also contains all of the points in the current pixel.
    total_points = NPoints();
  } else {
    // If not, then either the input pixel doesn't overlap the current one or
    // it's a sub-pixel of the current one.
    if (Contains(pix)) {
      // If we contain the input pixel, then we either iterate over the
      // sub-nodes to this pixel or iterate over the points contained in this
      // pixel.
      if (initialized_subpixels_) {
	for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
	  total_points += (*iter)->NPoints(pix);
	}
      } else {
	for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
	  if (pix.Contains(*(*iter))) total_points++;
	}
      }
    }
  }

  return total_points;
}

double TreePixel::PixelWeight(Pixel& pix) {
  double total_weight = 0.0;

  // First check to see if the input pixel contains the current pixel.
  if (pix.Contains(Resolution(), PixelX(), PixelY())) {
    // If so, then it also contains all of the points in the current pixel.
    total_weight = Weight();
  } else {
    // If not, then either the input pixel doesn't overlap the current one or
    // it's a sub-pixel of the current one.
    if (Contains(pix)) {
      // If we contain the input pixel, then we either iterate over the
      // sub-nodes to this pixel or iterate over the points contained in this
      // pixel.
      if (initialized_subpixels_) {
	for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
	  total_weight += (*iter)->PixelWeight(pix);
	}
      } else {
	for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
	  if (pix.Contains(*(*iter))) total_weight += (*iter)->Weight();
	}
      }
    }
  }

  return total_weight;
}

double TreePixel::Coverage() {
  double total_coverage = 0.0;

  // First check to see if the current pixel contains any sub-pixels.
  if (!initialized_subpixels_) {
    // If there are no sub-pixels, then we have no further information and
    // must assume that the whole pixel is covered with data, provided that
    // there's at least one point here.
    if (point_count_ > 0) total_coverage = 1.0;
  } else {
    // If we have sub-pixels, then we want to recursively probe the tree
    // structure to find out how many of the sub-pixels contain data.  Since
    // each sub-pixel contributes 1/4 the area of the current pixel, we
    // scale their results accordingly.
    for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
      total_coverage += 0.25*(*iter)->Coverage();
    }
  }

  return total_coverage;
}

double TreePixel::Coverage(Pixel& pix) {
  double total_coverage = 0.0;

  // First check to see if the input pixel contains the current pixel.
  if (pix.Contains(Resolution(), PixelX(), PixelY())) {
    // If so, then we return the coverage for this pixel, normalized by the
    // relative areas between the two pixels.
    total_coverage = Coverage()*Area()/pix.Area();
  } else {
    // If the input pixel doesn't contain the current pixel, then we need
    // to verify that the converse is true.  Otherwise, the Coverage() is 0.
    if (Contains(pix)) {
      // If there are no sub-pixels, then all we can say is that the input
      // pixel is completely covered by the current pixel.
      if (!initialized_subpixels_) {
	if (point_count_ > 0) total_coverage = 1.0;
      } else {
	// If we have sub-pixels, then we want to find the one that contains
	// the input pixel and recurse down the tree until we find either the
	// pixel itself or the last node that contains it.
	for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
	  if ((*iter)->Contains(pix))
	    total_coverage = (*iter)->Coverage(pix);
	}
      }
    }
  }

  return total_coverage;
}

void TreePixel::Points(WAngularVector& w_ang) {
  if (!w_ang.empty()) w_ang.clear();
  w_ang.reserve(point_count_);

  // If we haven't initialized any sub-nodes, then this is just a matter of
  // creating a copy of all of the points in the current pixel.
  if (!initialized_subpixels_) {
    for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
      WeightedAngularCoordinate tmp_ang = *(*iter);

      w_ang.push_back(tmp_ang);
    }
  } else {
    // If not, then we need to iterate through our sub-nodes and return an
    // aggregate list.
    for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
      WAngularVector tmp_ang;
      (*iter)->Points(tmp_ang);
      for (WAngularIterator ang_iter=tmp_ang.begin();
	   ang_iter!=tmp_ang.end();++ang_iter) w_ang.push_back(*ang_iter);
    }
  }
}

void TreePixel::Points(WAngularVector& w_ang, Pixel& pix) {
  if (!w_ang.empty()) w_ang.clear();
  w_ang.reserve(point_count_);

  // First, we need to check to verify that the input pixel is either contained
  // in the current pixel or contains it.
  if (Contains(pix) || pix.Contains(Resolution(), PixelX(), PixelY())) {
    // If we haven't initialized any sub-nodes, then this is just a matter of
    // creating a copy of all of the points in the current pixel that are
    // contained in the input pixel.
    if (!initialized_subpixels_) {
      for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
	if (pix.Contains(*(*iter))) {
	  WeightedAngularCoordinate tmp_ang((*iter)->Lambda(),
					    (*iter)->Eta(),
					    (*iter)->Weight(),
					    AngularCoordinate::Survey);
	  if ((*iter)->NFields() > 0) {
	    std::vector<std::string> field_names;
	    (*iter)->FieldNames(field_names);
	    for (uint16_t i=0;i<(*iter)->NFields();i++)
	      tmp_ang.SetField(field_names[i], (*iter)->Field(field_names[i]));
	  }
	  w_ang.push_back(tmp_ang);
	}
      }
    } else {
      // If not, then we need to iterate through our sub-nodes and return an
      // aggregate list.
      for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
	WAngularVector tmp_ang;
	(*iter)->Points(tmp_ang, pix);
	for (WAngularIterator ang_iter=tmp_ang.begin();
	     ang_iter!=tmp_ang.end();++ang_iter) w_ang.push_back(*ang_iter);
      }
    }
  }
}

uint16_t TreePixel::Nodes() {
  uint16_t n_nodes = 1;

  for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter)
    (*iter)->_AddSubNodes(n_nodes);

  return n_nodes;
}

void TreePixel::_AddSubNodes(uint16_t& n_nodes) {
  n_nodes++;

  for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter)
    (*iter)->_AddSubNodes(n_nodes);
}

double TreePixel::FieldTotal(const std::string& field_name, Pixel& pix) {
  double total_field = 0.0;

  // First check to see if the input pixel contains the current pixel.
  if (pix.Contains(Resolution(), PixelX(), PixelY())) {
    // If so, then it also contains all of the points in the current pixel.
    total_field = FieldTotal(field_name);
  } else {
    // If not, then either the input pixel doesn't overlap the current one or
    // it's a sub-pixel of the current one.
    if (Contains(pix)) {
      // If we contain the input pixel, then we either iterate over the
      // sub-nodes to this pixel or iterate over the points contained in this
      // pixel.
      if (initialized_subpixels_) {
	for (TreePtrIterator iter=subpix_.begin();iter!=subpix_.end();++iter) {
	  total_field += (*iter)->FieldTotal(field_name, pix);
	}
      } else {
	for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter) {
	  if (pix.Contains(*(*iter)))
	    total_field += (*iter)->Field(field_name);
	}
      }
    }
  }

  return total_field;
}

TreeNeighbor::TreeNeighbor(AngularCoordinate& reference_ang,
			   uint8_t n_neighbor) {
  reference_ang_ = reference_ang;
  n_neighbors_ = n_neighbor;
  max_distance_ = 100.0;
  n_nodes_visited_ = 0;
}

TreeNeighbor::~TreeNeighbor() {
  n_neighbors_ = 0;
  max_distance_ = 100.0;
  n_nodes_visited_ = 0;
}

void TreeNeighbor::NearestNeighbors(WAngularVector& w_ang,
				    bool save_neighbors) {
  if (!w_ang.empty()) w_ang.clear();

  std::vector<DistancePointPair> backup_copy;

  while (!ang_queue_.empty()) {
    DistancePointPair dist_pair = ang_queue_.top();
    ang_queue_.pop();

    WeightedAngularCoordinate tmp_ang(dist_pair.second->UnitSphereX(),
				      dist_pair.second->UnitSphereY(),
				      dist_pair.second->UnitSphereZ(),
				      dist_pair.second->Weight());
    tmp_ang.CopyFields(dist_pair.second);

    w_ang.push_back(tmp_ang);
    backup_copy.push_back(dist_pair);
  }

  if (save_neighbors) {
    for (uint8_t i=0;i<backup_copy.size();i++) {
      ang_queue_.push(backup_copy[i]);
    }
  }
}

bool TreeNeighbor::TestPoint(WeightedAngularCoordinate* test_ang) {
  bool kept_point = false;

  double costheta = reference_ang_.DotProduct(test_ang);

  double sin2theta = 1.0 - costheta*costheta;

  if (sin2theta < max_distance_ || Neighbors() < MaxNeighbors()) {
    // If the new point is closer than the most distant point in our queue or
    // we're still filling our heap, then we keep it.
    kept_point = true;

    // Throw away the current most distant point if we're at capacity.
    if (Neighbors() == MaxNeighbors()) ang_queue_.pop();

    // Create a new pair for the test point and add it to the queue.
    DistancePointPair dist_pair(sin2theta, test_ang);
    ang_queue_.push(dist_pair);

    // And reset our maximum distance using the new top of the heap.
    max_distance_ = ang_queue_.top().first;
  }

  return kept_point;
}

RegionMap::RegionMap() {
  ClearRegions();
};

RegionMap::~RegionMap() {
  ClearRegions();
};

uint16_t RegionMap::InitializeRegions(BaseMap* stomp_map, uint16_t n_region,
				      uint16_t region_resolution) {
  ClearRegions();

  // If we have the default value for the resolution, we need to attempt to
  // find a reasonable value for the resolution based on the area.  We want to
  // shoot for something along the lines of 50 pixels per region to give us a
  // fair chance of getting equal areas without using too many pixels.
  if (region_resolution == 0) {
    double target_area = stomp_map->Area()/(50*n_region);
    region_resolution = Stomp::HPixResolution;
    while ((Pixel::PixelArea(region_resolution) > target_area) &&
	   (region_resolution < 1024)) region_resolution <<= 1;
    // std::cout << "Automatically setting region resolution to " <<
    // region_resolution << " based on requested n_region\n";
  }

  if (region_resolution > 256) {
    std::cout <<
      "WARNING: Attempting to generate region map with resolution " <<
      "above 256!\n";
    std::cout << "This may end badly.\n";
    if (region_resolution > 2048) {
      std::cout <<
	"FAIL: Ok, the resolution is above 2048.  Can't do this.  Try again\n";
      exit(1);
    }
  }

  if (region_resolution > stomp_map->MaxResolution()) {
    std::cout << "WARNING: Re-setting region map resolution to " <<
      stomp_map->MaxResolution() << " to satisfy input map limits.\n";
    region_resolution = stomp_map->MaxResolution();
  }

  region_resolution_ = region_resolution;
  //std::cout << "Generating regionation map at " << region_resolution_ << "\n";

  PixelVector region_pix;
  stomp_map->Coverage(region_pix, region_resolution_);

  // std::cout << "Creating region map with " << region_pix.size() <<
  // " pixels...\n";

  if (static_cast<uint32_t>(n_region) > region_pix.size()) {
    std::cout << "WARNING: Exceeded maximum possible regions.  Setting to " <<
      region_pix.size() << " regions.\n";
    n_region = static_cast<uint32_t>(region_pix.size());
  }

  if (static_cast<uint32_t>(n_region) == region_pix.size()) {
    int16_t i=0;
    for (PixelIterator iter=region_pix.begin();iter!=region_pix.end();++iter) {
      region_map_[iter->Pixnum()] = i;
      i++;
    }
    std::cout <<
      "\tWARNING: Number of regions matches number of regionation pixels.\n";
    std::cout << "\tThis will be dead easy, " <<
      "but won't guarantee an equal area solution...\n";
  } else {
    // std::cout << "\tBreaking up " << stomp_map->Area() <<
    // " square degrees into " << n_region << " equal-area pieces.\n";

    std::vector<uint32_t> tmp_stripe;
    tmp_stripe.reserve(region_pix.size());
    for (PixelIterator iter=region_pix.begin();
	 iter!=region_pix.end();++iter) {
      tmp_stripe.push_back(iter->Stripe(region_resolution_));
    }

    sort(tmp_stripe.begin(), tmp_stripe.end());
    std::vector<uint32_t> stripe;
    stripe.push_back(tmp_stripe[0]);
    for (uint32_t i=1;i<tmp_stripe.size();i++)
      if (tmp_stripe[i] != tmp_stripe[i-1]) stripe.push_back(tmp_stripe[i]);

    tmp_stripe.clear();

    sort(stripe.begin(), stripe.end());

    std::vector<Section> super_section;

    Section tmp_section;
    tmp_section.SetMinStripe(stripe[0]);
    tmp_section.SetMaxStripe(stripe[0]);

    super_section.push_back(tmp_section);

    for (uint32_t i=1,j=0;i<stripe.size();i++) {
      if (stripe[i] == stripe[i-1] + 1) {
        super_section[j].SetMaxStripe(stripe[i]);
      } else {
        tmp_section.SetMinStripe(stripe[i]);
        tmp_section.SetMaxStripe(stripe[i]);
        super_section.push_back(tmp_section);
        j++;
      }
    }

    double region_length = sqrt(stomp_map->Area()/n_region);
    uint8_t region_width =
        static_cast<uint8_t>(region_length*Stomp::Nx0*region_resolution_/360.0);
    if (region_width == 0) region_width = 1;

    std::vector<Section> section;

    int32_t j = -1;
    for (std::vector<Section>::iterator iter=super_section.begin();
         iter!=super_section.end();++iter) {

      for (uint32_t stripe_iter=iter->MinStripe(),section_iter=region_width;
           stripe_iter<=iter->MaxStripe();stripe_iter++) {
        if (section_iter == region_width) {
          tmp_section.SetMinStripe(stripe_iter);
          tmp_section.SetMaxStripe(stripe_iter);
          section.push_back(tmp_section);
          section_iter = 1;
          j++;
        } else {
          section[j].SetMaxStripe(stripe_iter);
          section_iter++;
        }
      }
    }

    double region_area = 0.0, running_area = 0.0;
    double unit_area = Pixel::PixelArea(region_resolution_);
    uint32_t n_pixel = 0;
    int16_t region_iter = 0;
    double mean_area = stomp_map->Area()/region_pix.size();
    double area_break = stomp_map->Area()/n_region;

    // std::cout << "\tAssigning areas...\n\n";
    // std::cout << "\tSample  Pixels  Unmasked Area  Masked Area\n";
    // std::cout << "\t------  ------  -------------  -----------\n";
    for (std::vector<Section>::iterator section_iter=section.begin();
         section_iter!=section.end();++section_iter) {

      for (PixelIterator iter=region_pix.begin();
	   iter!=region_pix.end();++iter) {
        if ((iter->Stripe(region_resolution_) >=
	     section_iter->MinStripe()) &&
            (iter->Stripe(region_resolution_) <=
	     section_iter->MaxStripe())) {
          if ((region_area + 0.75*mean_area < area_break*(region_iter+1)) ||
	      (region_iter == n_region-1)) {
            region_area += iter->Weight()*unit_area;
            region_map_[iter->Pixnum()] = region_iter;
            running_area += iter->Weight()*unit_area;
            n_pixel++;
          } else {
	    // std::cout << "\t" << region_iter << "\t" << n_pixel << "\t" <<
	    // n_pixel*unit_area << "\t\t" << running_area << "\n";
	    region_area_[region_iter] = running_area;

            region_iter++;
            region_area += iter->Weight()*unit_area;
            region_map_[iter->Pixnum()] = region_iter;
            running_area = iter->Weight()*unit_area;
            n_pixel = 1;
          }
        }
      }
    }
    region_area_[region_iter] = running_area;
    // std::cout << "\t" << region_iter << "\t" << n_pixel << "\t" <<
    // n_pixel*unit_area << "\t\t" << running_area << "\n";
  }

  std::vector<uint32_t> region_count_check;

  for (uint16_t i=0;i<n_region;i++) region_count_check.push_back(0);

  for (RegionIterator iter=region_map_.begin();
       iter!=region_map_.end();++iter) {
    if (iter->second < n_region) {
      region_count_check[iter->second]++;
    } else {
      std::cout << "FAIL: Encountered illegal region index: " <<
	iter->second << "\nBailing...\n";
      exit(2);
    }
  }

  n_region_ = static_cast<uint16_t>(region_area_.size());

  return n_region_;
}

bool RegionMap::InitializeRegions(BaseMap* base_map, BaseMap& stomp_map) {
  bool initialized_region_map = true;
  if (!region_map_.empty()) region_map_.clear();

  region_resolution_ = stomp_map.RegionResolution();
  n_region_ = stomp_map.NRegion();

  // Iterate through the current BaseMap to find the region value for each
  // pixel.  If the node is not present in the input map, then
  // we bail and return false.
  PixelVector coverage_pix;
  base_map->Coverage(coverage_pix, stomp_map.RegionResolution());

  for (PixelIterator iter=coverage_pix.begin();
       iter!=coverage_pix.end();++iter) {
    int16_t region = stomp_map.Region(iter->SuperPix(region_resolution_));
    if (region != -1) {
      region_map_[iter->SuperPix(region_resolution_)] = region;
    } else {
      initialized_region_map = false;
      iter = coverage_pix.end();
    }
  }

  if (!initialized_region_map) {
    region_resolution_ = 0;
    n_region_ = -1;
  }

  return initialized_region_map;
}

int16_t RegionMap::FindRegion(AngularCoordinate& ang) {
  Pixel tmp_pix(ang, region_resolution_, 1.0);

  return (region_map_.find(tmp_pix.Pixnum()) != region_map_.end() ?
	  region_map_[tmp_pix.Pixnum()] : -1);
}

void RegionMap::ClearRegions() {
  region_map_.clear();
  n_region_ = 0;
  region_resolution_ = 0;
}

void RegionMap::RegionAreaMap(int16_t region_index, Map& stomp_map) {
  stomp_map.Clear();

  PixelVector region_pix;
  for (RegionIterator iter=Begin();iter!=End();++iter) {
    if (iter->second == region_index) {
      Pixel tmp_pix(Resolution(), iter->first, 1.0);
      region_pix.push_back(tmp_pix);
    }
  }
  stomp_map.Initialize(region_pix);
}

BaseMap::BaseMap() {
  ClearRegions();
}

BaseMap::~BaseMap() {
  ClearRegions();
}

void BaseMap::Coverage(PixelVector& superpix, uint16_t resolution) {
  superpix.clear();
}

bool BaseMap::Covering(Map& stomp_map, uint32_t maximum_pixels) {
  return false;
}

double BaseMap::FindUnmaskedFraction(Pixel& pix) {
  return 0.0;
}

int8_t BaseMap::FindUnmaskedStatus(Pixel& pix) {
  return 0;
}

bool BaseMap::Empty() {
  return true;
}

void BaseMap::Clear() {
  ClearRegions();
}

uint32_t BaseMap::Size() {
  return 0;
}

double BaseMap::Area() {
  return 0.0;
}

uint16_t BaseMap::MinResolution() {
  return Stomp::HPixResolution;
}

uint16_t BaseMap::MaxResolution() {
  return Stomp::MaxPixelResolution;
}

SubMap::SubMap(uint32_t superpixnum) {
  superpixnum_ = superpixnum;
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  Pixel::PixelBound(Stomp::HPixResolution, superpixnum, lambda_min_,
		    lambda_max_, eta_min_, eta_max_);
  z_min_ = sin(lambda_min_*Stomp::DegToRad);
  z_max_ = sin(lambda_max_*Stomp::DegToRad);
  initialized_ = false;
  unsorted_ = false;

  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++) {
    pixel_count_[resolution] = 0;
  }
}

SubMap::~SubMap() {
  if (!pix_.empty()) pix_.clear();
  superpixnum_ = Stomp::MaxSuperpixnum;
  initialized_ = false;
}

void SubMap::AddPixel(Pixel& pix) {
  // If our pixels are input in proper order, then we don't need to resolve
  // things down the line.  Provided that every input pixel comes after the
  // last pixel input, then we're assured that the list is sorted.
  if (!pix_.empty())
    if (!Pixel::LocalOrder(pix_[pix_.size()-1], pix)) unsorted_ = true;

  // If we're dealing with a sorted input list, then we can go ahead and
  // collect our summary statistics as we go.  Otherwise, we don't bother
  // since the results will be over-written by the Resolve method.
  if (!unsorted_) {
    area_ += pix.Area();
    if (pix.Resolution() < min_resolution_) min_resolution_ = pix.Resolution();
    if (pix.Resolution() > max_resolution_) max_resolution_ = pix.Resolution();
    if (pix.Weight() < min_weight_) min_weight_ = pix.Weight();
    if (pix.Weight() > max_weight_) max_weight_ = pix.Weight();
    pixel_count_[pix.Resolution()]++;
  }

  pix_.push_back(pix);
  size_ = pix_.size();
  initialized_ = true;
}

void SubMap::Resolve(bool force_resolve) {
  if (pix_.size() != size_) unsorted_ = true;

  if (unsorted_ || force_resolve) {
    Pixel::ResolveSuperPixel(pix_);

    area_ = 0.0;
    min_resolution_ = Stomp::MaxPixelResolution;
    max_resolution_ = Stomp::HPixResolution;
    min_weight_ = 1.0e30;
    max_weight_ = -1.0e30;
    for (uint16_t resolution=Stomp::HPixResolution, i=0;
	 i<Stomp::ResolutionLevels;resolution*=2, i++) {
      pixel_count_[resolution] = 0;
    }

    for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      area_ += iter->Area();
      if (iter->Resolution() < min_resolution_)
        min_resolution_ = iter->Resolution();
      if (iter->Resolution() > max_resolution_)
        max_resolution_ = iter->Resolution();
      if (iter->Weight() < min_weight_) min_weight_ = iter->Weight();
      if (iter->Weight() > max_weight_) max_weight_ = iter->Weight();
      pixel_count_[iter->Resolution()]++;
    }
  }

  unsorted_ = false;
  size_ = pix_.size();
  if (pix_.size() > 0) initialized_ = true;
}

void SubMap::SetMinimumWeight(double min_weight) {
  PixelVector pix;
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    if (Stomp::DoubleGE(iter->Weight(), min_weight)) pix.push_back(*iter);
  }

  Clear();
  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) AddPixel(*iter);
  Resolve();
}

void SubMap::SetMaximumWeight(double max_weight) {
  PixelVector pix;
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    if (Stomp::DoubleLE(iter->Weight(), max_weight)) pix.push_back(*iter);
  }

  Clear();
  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) AddPixel(*iter);
  Resolve();
}

void SubMap::SetMaximumResolution(uint16_t max_resolution,
				  bool average_weights) {
  PixelVector pix;
  pix.reserve(Size());

  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    Pixel tmp_pix = *iter;
    if (average_weights) {
      if (iter->Resolution() > max_resolution) {
	tmp_pix.SetToSuperPix(max_resolution);
	tmp_pix.SetWeight(FindAverageWeight(tmp_pix));
      }
    } else {
      if (iter->Resolution() > max_resolution) {
	tmp_pix.SetToSuperPix(max_resolution);
	tmp_pix.SetWeight(FindUnmaskedFraction(tmp_pix));
      } else {
	tmp_pix.SetWeight(1.0);
      }
    }
    pix.push_back(tmp_pix);
  }

  Clear();
  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) AddPixel(*iter);
  Resolve();
}

bool SubMap::FindLocation(AngularCoordinate& ang, double& weight) {
  bool keep = false;
  weight = -1.0e-30;

  for (uint16_t resolution=min_resolution_;
       resolution<=max_resolution_;resolution*=2) {
    Pixel tmp_pix(ang,resolution);
    PixelPair iter = equal_range(pix_.begin(), pix_.end(), tmp_pix,
                                 Pixel::SuperPixelBasedOrder);
    if (iter.first != iter.second) {
      keep = true;
      weight = iter.first->Weight();
    }
    if (keep) resolution = max_resolution_*2;
  }

  return keep;
}

double SubMap::FindUnmaskedFraction(Pixel& pix) {
  PixelIterator iter;
  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    Pixel tmp_pix(pix.PixelX0()*2, pix.PixelY0()*2,
		  pix.Resolution()*2, 1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),
                       tmp_pix,Pixel::SuperPixelBasedOrder);
  }

  uint16_t resolution = min_resolution_;
  double unmasked_fraction = 0.0;
  bool found_pixel = false;
  while (resolution <= pix.Resolution() && !found_pixel) {
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);
    PixelPair super_iter = equal_range(pix_.begin(), iter, tmp_pix,
                                       Pixel::SuperPixelBasedOrder);
    if (super_iter.first != super_iter.second) {
      found_pixel = true;
      unmasked_fraction = 1.0;
    }
    resolution *= 2;
  }

  while (iter != pix_.end() && !found_pixel) {
    if (pix.Contains(*iter)) {
      double pixel_fraction =
          static_cast<double> (pix.Resolution()*pix.Resolution())/
          (iter->Resolution()*iter->Resolution());
      unmasked_fraction += pixel_fraction;
    }
    ++iter;
  }

  return unmasked_fraction;
}

int8_t SubMap::FindUnmaskedStatus(Pixel& pix) {
  PixelIterator iter;
  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    Pixel tmp_pix(pix.PixelX0()*2, pix.PixelY0()*2,
		  pix.Resolution()*2, 1.0);
    iter = lower_bound(pix_.begin(), pix_.end(), tmp_pix,
                       Pixel::SuperPixelBasedOrder);
  }

  uint16_t resolution = min_resolution_;
  int8_t unmasked_status = 0;
  while ((resolution <= pix.Resolution()) && (unmasked_status == 0)) {
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);
    PixelPair super_iter = equal_range(pix_.begin(),iter,tmp_pix,
                                       Pixel::SuperPixelBasedOrder);
    if (super_iter.first != super_iter.second) unmasked_status = 1;
    resolution *= 2;
  }

  while ((iter != pix_.end()) && (unmasked_status == 0)) {
    if (pix.Contains(*iter)) unmasked_status = -1;
    ++iter;
  }

  return unmasked_status;
}

double SubMap::FindAverageWeight(Pixel& pix) {
  PixelIterator iter;

  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    Pixel tmp_pix(pix.Resolution()*2,0,pix.Superpixnum(),1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),tmp_pix,
                       Pixel::SuperPixelBasedOrder);
  }

  double unmasked_fraction = 0.0, weighted_average = 0.0;
  bool found_pixel = false;
  uint16_t resolution = min_resolution_;
  while (resolution <= pix.Resolution() && !found_pixel) {
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);
    PixelPair super_iter = equal_range(pix_.begin(), iter, tmp_pix,
                                       Pixel::SuperPixelBasedOrder);
    if (super_iter.first != super_iter.second) {
      found_pixel = true;
      weighted_average = super_iter.first->Weight();
      unmasked_fraction = 1.0;
    }
    resolution *= 2;
  }

  while (iter != pix_.end() && !found_pixel) {
    if (pix.Contains(*iter)) {
      double pixel_fraction =
	static_cast<double> (pix.Resolution()*pix.Resolution())/
	(iter->Resolution()*iter->Resolution());
      unmasked_fraction += pixel_fraction;
      weighted_average += iter->Weight()*pixel_fraction;
    }
    ++iter;
  }

  if (unmasked_fraction > 0.000000001) weighted_average /= unmasked_fraction;

  return weighted_average;
}

void SubMap::FindMatchingPixels(Pixel& pix, PixelVector& match_pix,
				bool use_local_weights) {
  if (!match_pix.empty()) match_pix.clear();

  bool found_pixel = false;
  PixelIterator iter, find_iter;
  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    Pixel tmp_pix(pix.PixelX0()*2, pix.PixelY0()*2, pix.Resolution()*2, 1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),tmp_pix,
                       Pixel::SuperPixelBasedOrder);
  }

  uint16_t resolution = min_resolution_;
  while (resolution <= pix.Resolution() && !found_pixel) {
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);
    find_iter = lower_bound(pix_.begin(), iter, tmp_pix,
                            Pixel::SuperPixelBasedOrder);
    if (Pixel::PixelMatch(*find_iter,tmp_pix)) {
      found_pixel = true;
      tmp_pix = pix;
      if (use_local_weights) tmp_pix.SetWeight(find_iter->Weight());
      match_pix.push_back(tmp_pix);
    }
    resolution *= 2;
  }

  while (iter != pix_.end() && !found_pixel) {
    if (pix.Contains(*iter)) {
      Pixel tmp_pix = *iter;
      tmp_pix = *iter;
      if (!use_local_weights) tmp_pix.SetWeight(pix.Weight());
      match_pix.push_back(tmp_pix);
    }

    ++iter;
  }
}

double SubMap::AverageWeight() {
  double unmasked_fraction = 0.0, weighted_average = 0.0;
  if (initialized_) {
    for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      weighted_average += iter->Weight()*iter->Area();
      unmasked_fraction += iter->Area();
    }
    weighted_average /= unmasked_fraction;
  }
  return weighted_average;
}

void SubMap::Soften(PixelVector& output_pix, uint16_t max_resolution,
		    bool average_weights) {
  if (!output_pix.empty()) output_pix.clear();
  output_pix.reserve(pix_.size());

  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    Pixel tmp_pix = *iter;
    if (average_weights) {
      if (iter->Resolution() > max_resolution) {
	tmp_pix.SetToSuperPix(max_resolution);
	tmp_pix.SetWeight(FindAverageWeight(tmp_pix));
      }
    } else {
      if (iter->Resolution() > max_resolution) {
	tmp_pix.SetToSuperPix(max_resolution);
	tmp_pix.SetWeight(FindUnmaskedFraction(tmp_pix));
      } else {
	tmp_pix.SetWeight(1.0);
      }
    }
    output_pix.push_back(tmp_pix);
  }

  Pixel::ResolveSuperPixel(output_pix);
}

bool SubMap::Add(Map& stomp_map, bool drop_single) {
  PixelVector keep_pix;
  PixelVector resolve_pix;

  // Iterate over all of our pixels and check each against the input map.
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    int8_t status = stomp_map.FindUnmaskedStatus(*iter);

    // If the pixel is completely outside the input map, then we can keep
    // the whole pixel, provided that we're not dropping area that's not in
    // both Maps.
    if (status == 0 && !drop_single) {
      keep_pix.push_back(*iter);
    }

    // If the pixel is completely inside of the input map, then we add it to
    // the keep array.  There's a complication here in that we need to account
    // for cases where the current pixel may be covering multiple pixels with
    // different weights in the input map.
    if (status == 1) {
      PixelVector match_pix;
      stomp_map.FindMatchingPixels(*iter, match_pix);
      for (PixelIterator match_iter=match_pix.begin();
	   match_iter!=match_pix.end();++match_iter) {
	match_iter->SetWeight(match_iter->Weight() + iter->Weight());
	keep_pix.push_back(*match_iter);
      }
    }

    // If there's partial overlap, then we need to refine the pixel and check
    // again until we find the parts that don't overlap.
    if (status == -1) {
      PixelVector sub_pix;
      iter->SubPix(2*iter->Resolution(), sub_pix);
      for (PixelIterator sub_iter=sub_pix.begin();
	   sub_iter!=sub_pix.end();++sub_iter) {
	sub_iter->SetWeight(iter->Weight());
	resolve_pix.push_back(*sub_iter);
      }
    }
  }

  // Now we check those pixels that need resolving and iterate until there are
  // no more to check.
  while (resolve_pix.size() > 0) {
    PixelVector tmp_pix;
    tmp_pix.reserve(resolve_pix.size());
    for (PixelIterator iter=resolve_pix.begin();
	 iter!=resolve_pix.end();++iter) tmp_pix.push_back(*iter);

    resolve_pix.clear();

    for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter) {
      int8_t status = stomp_map.FindUnmaskedStatus(*iter);

      if (status == 0 && !drop_single) {
	keep_pix.push_back(*iter);
      }

      if (status == 1) {
	PixelVector match_pix;
	stomp_map.FindMatchingPixels(*iter, match_pix);
	for (PixelIterator match_iter=match_pix.begin();
	     match_iter!=match_pix.end();++match_iter) {
	  match_iter->SetWeight(match_iter->Weight() + iter->Weight());
	  keep_pix.push_back(*match_iter);
	}
      }

      if (status == -1) {
	PixelVector sub_pix;
	iter->SubPix(2*iter->Resolution(), sub_pix);
	for (PixelIterator sub_iter=sub_pix.begin();
	     sub_iter!=sub_pix.end();++sub_iter) {
	  sub_iter->SetWeight(iter->Weight());
	  resolve_pix.push_back(*sub_iter);
	}
      }
    }
  }

  // That covers the area in our current map.  However, if we're not dropping
  // area that is contained in just one map, we need to find those pixels in
  // the input Map that didn't overlap with anything in our current Map and
  // add those to the array of keep pixels.
  if (!drop_single) {
    PixelVector stomp_pix;
    stomp_map.Pixels(stomp_pix, Superpixnum());

    for (PixelIterator iter=stomp_pix.begin();iter!=stomp_pix.end();++iter) {
      // Only keep those pixels that are completely outside of our current Map.
      if (FindUnmaskedStatus(*iter)) keep_pix.push_back(*iter);
    }
  }
    
  // Now we clear out our current set of pixels and replace them by the ones
  // that weren't in the input map.
  Clear();

  for (PixelIterator iter=keep_pix.begin();iter!=keep_pix.end();++iter) {
    AddPixel(*iter);
  }

  if (unsorted_) Resolve();

  return true;
}

bool SubMap::Multiply(Map& stomp_map, bool drop_single) {
  PixelVector keep_pix;
  PixelVector resolve_pix;

  // Iterate over all of our pixels and check each against the input map.
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    int8_t status = stomp_map.FindUnmaskedStatus(*iter);

    // If the pixel is completely outside the input map, then we can keep
    // the whole pixel, provided that we're not dropping area that's not in
    // both Maps.
    if (status == 0 && !drop_single) {
      keep_pix.push_back(*iter);
    }

    // If the pixel is completely inside of the input map, then we add it to
    // the keep array.  There's a complication here in that we need to account
    // for cases where the current pixel may be covering multiple pixels with
    // different weights in the input map.
    if (status == 1) {
      PixelVector match_pix;
      stomp_map.FindMatchingPixels(*iter, match_pix);
      for (PixelIterator match_iter=match_pix.begin();
	   match_iter!=match_pix.end();++match_iter) {
	match_iter->SetWeight(match_iter->Weight()*iter->Weight());
	keep_pix.push_back(*match_iter);
      }
    }

    // If there's partial overlap, then we need to refine the pixel and check
    // again until we find the parts that don't overlap.
    if (status == -1) {
      PixelVector sub_pix;
      iter->SubPix(2*iter->Resolution(), sub_pix);
      for (PixelIterator sub_iter=sub_pix.begin();
	   sub_iter!=sub_pix.end();++sub_iter) {
	sub_iter->SetWeight(iter->Weight());
	resolve_pix.push_back(*sub_iter);
      }
    }
  }

  // Now we check those pixels that need resolving and iterate until there are
  // no more to check.
  while (resolve_pix.size() > 0) {
    PixelVector tmp_pix;
    tmp_pix.reserve(resolve_pix.size());
    for (PixelIterator iter=resolve_pix.begin();
	 iter!=resolve_pix.end();++iter) tmp_pix.push_back(*iter);

    resolve_pix.clear();

    for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter) {
      int8_t status = stomp_map.FindUnmaskedStatus(*iter);

      if (status == 0 && !drop_single) {
	keep_pix.push_back(*iter);
      }

      if (status == 1) {
	PixelVector match_pix;
	stomp_map.FindMatchingPixels(*iter, match_pix);
	for (PixelIterator match_iter=match_pix.begin();
	     match_iter!=match_pix.end();++match_iter) {
	  match_iter->SetWeight(match_iter->Weight()*iter->Weight());
	  keep_pix.push_back(*match_iter);
	}
      }

      if (status == -1) {
	PixelVector sub_pix;
	iter->SubPix(2*iter->Resolution(), sub_pix);
	for (PixelIterator sub_iter=sub_pix.begin();
	     sub_iter!=sub_pix.end();++sub_iter) {
	  sub_iter->SetWeight(iter->Weight());
	  resolve_pix.push_back(*sub_iter);
	}
      }
    }
  }

  // That covers the area in our current map.  However, if we're not dropping
  // area that is contained in just one map, we need to find those pixels in
  // the input Map that didn't overlap with anything in our current Map and
  // add those to the array of keep pixels.
  if (!drop_single) {
    PixelVector stomp_pix;
    stomp_map.Pixels(stomp_pix, Superpixnum());

    for (PixelIterator iter=stomp_pix.begin();iter!=stomp_pix.end();++iter) {
      // Only keep those pixels that are completely outside of our current Map.
      if (FindUnmaskedStatus(*iter)) keep_pix.push_back(*iter);
    }
  }

  // Now we clear out our current set of pixels and replace them by the ones
  // that weren't in the input map.
  Clear();

  for (PixelIterator iter=keep_pix.begin();iter!=keep_pix.end();++iter) {
    AddPixel(*iter);
  }

  if (unsorted_) Resolve();

  return true;
}

bool SubMap::Exclude(Map& stomp_map) {

  PixelVector keep_pix;
  PixelVector resolve_pix;

  // Iterate over all of our pixels and check each against the input map.
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    int8_t status = stomp_map.FindUnmaskedStatus(*iter);

    // If the pixel is completely outside the input map, then we can keep
    // the whole pixel.
    if (status == 0) {
      keep_pix.push_back(*iter);
    }

    // If there's partial overlap, then we need to refine the pixel and check
    // again until we find the parts that don't overlap.
    if (status == -1) {
      PixelVector sub_pix;
      iter->SubPix(2*iter->Resolution(), sub_pix);
      for (PixelIterator sub_iter=sub_pix.begin();
	   sub_iter!=sub_pix.end();++sub_iter) {
	sub_iter->SetWeight(iter->Weight());
	resolve_pix.push_back(*sub_iter);
      }
    }
  }

  // Now we check those pixels that need resolving and iterate until there are
  // no more to check.
  while (resolve_pix.size() > 0) {
    PixelVector tmp_pix;
    tmp_pix.reserve(resolve_pix.size());
    for (PixelIterator iter=resolve_pix.begin();
	 iter!=resolve_pix.end();++iter) tmp_pix.push_back(*iter);

    resolve_pix.clear();

    for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter) {
      int8_t status = stomp_map.FindUnmaskedStatus(*iter);

      if (status == 0) {
	keep_pix.push_back(*iter);
      }

      if (status == -1) {
	PixelVector sub_pix;
	iter->SubPix(2*iter->Resolution(), sub_pix);
	for (PixelIterator sub_iter=sub_pix.begin();
	     sub_iter!=sub_pix.end();++sub_iter) {
	  sub_iter->SetWeight(iter->Weight());
	  resolve_pix.push_back(*sub_iter);
	}
      }
    }
  }
    
  // Now we clear out our current set of pixels and replace them by the ones
  // that weren't in the input map.
  Clear();

  for (PixelIterator iter=keep_pix.begin();iter!=keep_pix.end();++iter) {
    AddPixel(*iter);
  }

  if (unsorted_) Resolve();

  return true;
}

void SubMap::ScaleWeight(const double weight_scale) {
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter)
    iter->SetWeight(iter->Weight()*weight_scale);
}

void SubMap::AddConstantWeight(const double add_weight) {
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter)
    iter->SetWeight(iter->Weight()+add_weight);
}

void SubMap::InvertWeight() {
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    if ((iter->Weight() > 1.0e-15) || (iter->Weight() < -1.0e-15)) {
      iter->SetWeight(1.0/iter->Weight());
    } else {
      iter->SetWeight(0.0);
    }
  }
}

void SubMap::Pixels(PixelVector& pix) {
  if (!pix.empty()) pix.clear();
  pix.reserve(Size());
  for (PixelIterator iter=pix_.begin();iter!=pix_.end();++iter)
    pix.push_back(*iter);
}

void SubMap::Clear() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  if (!pix_.empty()) pix_.clear();
  initialized_ = false;
  unsorted_ = false;
}

Map::Map() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  ClearRegions();

  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++)
    pixel_count_[resolution] = 0;

  sub_map_.reserve(Stomp::MaxSuperpixnum);

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    sub_map_.push_back(SubMap(k));

  begin_ = MapIterator(0, sub_map_[0].Begin());
  end_ = begin_;
}

Map::Map(PixelVector& pix, bool force_resolve) {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  ClearRegions();
  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++) {
    pixel_count_[resolution] = 0;
  }

  sub_map_.reserve(Stomp::MaxSuperpixnum);

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    sub_map_.push_back(SubMap(k));

  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter)
    sub_map_[iter->Superpixnum()].AddPixel(*iter);

  bool found_beginning = false;
  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Initialized()) {
      if (iter->Unsorted() || force_resolve) iter->Resolve(force_resolve);

      if (!found_beginning) {
	begin_ = MapIterator(iter->Superpixnum(), iter->Begin());
	found_beginning = true;
      }
      end_ = MapIterator(iter->Superpixnum(), iter->End());

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
      for (uint16_t resolution=Stomp::HPixResolution, i=0;
	   i<Stomp::ResolutionLevels;resolution*=2, i++) {
	pixel_count_[resolution] += iter->PixelCount(resolution);
      }
    }
  }
}

Map::Map(std::string& InputFile, bool hpixel_format, bool weighted_map) {
  Read(InputFile, hpixel_format, weighted_map);
}

Map::~Map() {
  min_resolution_ = max_resolution_ = 0;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  Clear();
}

bool Map::Initialize() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  ClearRegions();
  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++) {
    pixel_count_[resolution] = 0;
  }

  bool found_valid_superpixel = false;
  bool found_beginning = false;
  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Initialized()) {
      found_valid_superpixel = true;
      if (iter->Unsorted()) iter->Resolve();

      if (!found_beginning) {
	begin_ = MapIterator(iter->Superpixnum(), iter->Begin());
	found_beginning = true;
      }
      end_ = MapIterator(iter->Superpixnum(), iter->End());

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
      for (uint16_t resolution=Stomp::HPixResolution, i=0;
	   i<Stomp::ResolutionLevels;resolution*=2, i++) {
	pixel_count_[resolution] += iter->PixelCount(resolution);
      }
    }
  }

  return found_valid_superpixel;
}

bool Map::Initialize(PixelVector& pix, bool force_resolve) {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  ClearRegions();
  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++) {
    pixel_count_[resolution] = 0;
  }

  if (!sub_map_.empty()) {
    for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter)
      iter->Clear();
    sub_map_.clear();
  }

  sub_map_.reserve(Stomp::MaxSuperpixnum);

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    SubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
    uint32_t k = iter->Superpixnum();

    sub_map_[k].AddPixel(*iter);
  }

  bool found_valid_superpixel = false;
  bool found_beginning = false;
  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Initialized()) {
      if (iter->Unsorted() || force_resolve) iter->Resolve(force_resolve);

      found_valid_superpixel = true;

      if (!found_beginning) {
	begin_ = MapIterator(iter->Superpixnum(), iter->Begin());
	found_beginning = true;
      }
      end_ = MapIterator(iter->Superpixnum(), iter->End());

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
      for (uint16_t resolution=Stomp::HPixResolution, i=0;
	   i<Stomp::ResolutionLevels;resolution*=2, i++) {
	pixel_count_[resolution] += iter->PixelCount(resolution);
      }
    }
  }

  return found_valid_superpixel;
}

void Map::Coverage(PixelVector& superpix, uint16_t resolution) {
  if (!superpix.empty()) superpix.clear();

  if (resolution == Stomp::HPixResolution) {
    // If we're dealing with a coverage map at superpixel resolution (the
    // default behavior), then this is easy.  Just iterate over the submaps
    // and keep those that have been initialized.
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
	// We store the unmasked fraction of each superpixel in the weight
	// value in case that's useful.
	Pixel tmp_pix(Stomp::HPixResolution, k,
		      sub_map_[k].Area()/Stomp::HPixArea);
	superpix.push_back(tmp_pix);
      }
    }
  } else {
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
	Pixel tmp_pix(Stomp::HPixResolution, k, 1.0);

	PixelVector sub_pix;
	tmp_pix.SubPix(resolution, sub_pix);
	for (PixelIterator iter=sub_pix.begin();iter!=sub_pix.end();++iter) {
	  // For each of the pixels in the superpixel, we check its status
	  // against the current map.  This is faster than finding the unmasked
	  // fraction directly and immediately tells us which pixels we can
	  // eliminate and which of those we do keep require further
	  // calculations to find the unmasked fraction.
	  int8_t unmasked_status = FindUnmaskedStatus(*iter);
	  if (unmasked_status != 0) {
	    if (unmasked_status == 1) {
	      iter->SetWeight(1.0);
	    } else {
	      iter->SetWeight(FindUnmaskedFraction(*iter));
	    }
	    superpix.push_back(*iter);
	  }
	}
      }
    }
    sort(superpix.begin(), superpix.end(), Pixel::SuperPixelBasedOrder);
  }
}

bool Map::Covering(Map& stomp_map, uint32_t maximum_pixels) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  PixelVector pix;
  Coverage(pix);

  bool met_pixel_requirement;
  if (pix.size() > maximum_pixels) {
    // If the number of requested pixels is smaller than the number of
    // superpixels in the map, then we'd never be able to meet that requirement.
    // In this case, we set the output Map to the coarsest possible case
    // and return false.
    met_pixel_requirement = false;

    FindAverageWeight(pix);
    stomp_map.Initialize(pix);
  } else {
    // Ok, in this case, we can definitely produce a map that has at most
    // maximum_pixels in it.
    met_pixel_requirement = true;

    // To possibly save us as much work as possible, we check to see if
    // maximum_pixels is larger than our current set.  If so, we just return
    // a copy of the current map.
    if (maximum_pixels > Size()) {
      pix.clear();
      pix.reserve(Size());
      Pixels(pix);
      stomp_map.Initialize(pix);
    } else {
      // Ok, in this case, we have to do actual work.  As a rough rule of
      // thumb, we should expect that the number of pixels in any given
      // resolution level would double if we were to combine all of the pixels
      // at finer resolution into the current level.  So, we proceed from
      // coarse to fine until adding twice a given level would take us over
      // the maximum_pixels limit.  Then we re-sample all of the pixels below
      // that level and check that against our limit, iterating again at a
      // coarser resolution limit if necessary.  This should minimize
      // the number of times that we need to re-create the pixel map.
      uint16_t maximum_resolution = Stomp::HPixResolution;
      uint32_t reduced_map_pixels = pixel_count_[maximum_resolution];

      while (reduced_map_pixels + 2*pixel_count_[2*maximum_resolution] <
	     maximum_pixels &&
	     maximum_resolution < Stomp::MaxPixelResolution/2 ) {
	reduced_map_pixels += pixel_count_[maximum_resolution];
	maximum_resolution *= 2;
      }

      reduced_map_pixels = maximum_pixels + 1;
      while ((reduced_map_pixels > maximum_pixels) &&
	     (maximum_resolution >= Stomp::HPixResolution)) {
	pix.clear();
	pix.reserve(Size());
	Pixels(pix);

	for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
	  if (iter->Resolution() > maximum_resolution) {
	    iter->SuperPix(maximum_resolution);
	  }
	}

	// Resolve the array of pixels ignoring their current weight values.
	// The consequence of this is that we may end up under-sampling any
	// scalar field that's been encoded onto the current map and that we
	// won't be producing a map that contains the most resolution possible
	// given the pixel limit.  However, given the usage, these are probably
	// not show-stoppers.
	Pixel::ResolvePixel(pix, true);

	reduced_map_pixels = pix.size();

	if (reduced_map_pixels < maximum_pixels) {
	  FindAverageWeight(pix);
	  stomp_map.Initialize(pix);
	} else {
	  maximum_resolution /= 2;
	}
      }
    }
  }

  return met_pixel_requirement;
}

void Map::Soften(Map& stomp_map, uint16_t maximum_resolution,
		 bool average_weights) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  PixelVector pix;
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized()) {
      PixelVector tmp_pix;
      sub_map_[k].Soften(tmp_pix, maximum_resolution, average_weights);

      for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
	pix.push_back(*iter);
    }
  }

  stomp_map.Initialize(pix);
}

void Map::Soften(uint16_t maximum_resolution, bool average_weights) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized()) {
      sub_map_[k].SetMaximumResolution(maximum_resolution, average_weights);
    }
  }
  Initialize();
}

void Map::SetMinimumWeight(double min_weight) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized()) {
      sub_map_[k].SetMinimumWeight(min_weight);
    }
  }
  Initialize();
}

void Map::SetMaximumWeight(double max_weight) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized()) {
      sub_map_[k].SetMaximumWeight(max_weight);
    }
  }
  Initialize();
}

bool Map::RegionOnlyMap(int16_t region_index, Map& stomp_map) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  bool map_generation_success = false;
  if ((region_index >= 0) && (region_index < NRegion())) {
    // First, we need to create a copy of our current map.
    PixelVector pix;
    Pixels(pix);
    stomp_map.Initialize(pix);
    pix.clear();

    // Now we generate a Map for the input region.
    Map region_map;
    RegionAreaMap(region_index, region_map);

    // And finally, we find the intersection of these two maps.  This should
    // just be the part of our current map in the specified region.
    map_generation_success = stomp_map.IntersectMap(region_map);
    region_map.Clear();
  }

  return map_generation_success;
}

bool Map::RegionExcludedMap(int16_t region_index, Map& stomp_map) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  bool map_generation_success = false;
  if ((region_index >= 0) && (region_index < NRegion())) {
    // First, we need to create a copy of our current map.
    PixelVector pix;
    Pixels(pix);
    stomp_map.Initialize(pix);
    pix.clear();

    // Now we generate a Map for the input region.
    Map region_map;
    RegionAreaMap(region_index, region_map);

    // And finally, we exclude the region map from the map copy.  This should
    // just be the part of our current map outside the specified region.
    map_generation_success = stomp_map.ExcludeMap(region_map);
    region_map.Clear();
  }

  return map_generation_success;
}

bool Map::FindLocation(AngularCoordinate& ang) {
  bool keep = false;
  double weight;

  uint32_t k;
  Pixel::Ang2Pix(Stomp::HPixResolution, ang, k);

  if (sub_map_[k].Initialized()) keep = sub_map_[k].FindLocation(ang, weight);

  return keep;
}

bool Map::FindLocation(AngularCoordinate& ang, double& weight) {
  bool keep = false;

  uint32_t k;
  Pixel::Ang2Pix(Stomp::HPixResolution, ang, k);

  if (sub_map_[k].Initialized()) keep = sub_map_[k].FindLocation(ang, weight);

  return keep;
}

double Map::FindLocationWeight(AngularCoordinate& ang) {
  bool keep = false;
  double weight = -1.0e-30;

  uint32_t k;
  Pixel::Ang2Pix(Stomp::HPixResolution,ang,k);

  if (sub_map_[k].Initialized()) keep = sub_map_[k].FindLocation(ang, weight);

  return weight;
}

double Map::FindUnmaskedFraction(Pixel& pix) {
  double unmasked_fraction = 0.0;

  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized())
    unmasked_fraction = sub_map_[k].FindUnmaskedFraction(pix);

  return unmasked_fraction;
}

void Map::FindUnmaskedFraction(PixelVector& pix,
			       std::vector<double>& unmasked_fraction) {

  if (!unmasked_fraction.empty()) unmasked_fraction.clear();

  unmasked_fraction.reserve(pix.size());

  for (uint32_t i=0;i<pix.size();i++){
    double pixel_unmasked_fraction = 0.0;
    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_unmasked_fraction = sub_map_[k].FindUnmaskedFraction(pix[i]);

    unmasked_fraction.push_back(pixel_unmasked_fraction);
  }
}

void Map::FindUnmaskedFraction(PixelVector& pix) {
  for (uint32_t i=0;i<pix.size();i++){
    double pixel_unmasked_fraction = 0.0;
    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_unmasked_fraction = sub_map_[k].FindUnmaskedFraction(pix[i]);
    pix[i].SetWeight(pixel_unmasked_fraction);
  }
}

double Map::FindUnmaskedFraction(Map& stomp_map) {
  double total_unmasked_area = 0.0;
  for (MapIterator iter=stomp_map.Begin();
       iter!=stomp_map.End();stomp_map.Iterate(&iter)) {
    double pixel_unmasked_fraction = 0.0;
    uint32_t k = iter.second->Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_unmasked_fraction =
	sub_map_[k].FindUnmaskedFraction(*(iter.second));
    total_unmasked_area += pixel_unmasked_fraction*iter.second->Area();
  }

  return total_unmasked_area/stomp_map.Area();
}

int8_t Map::FindUnmaskedStatus(Pixel& pix) {
  int8_t unmasked_status = 0;

  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized())
    unmasked_status = sub_map_[k].FindUnmaskedStatus(pix);

  return unmasked_status;
}

void Map::FindUnmaskedStatus(PixelVector& pix,
			     std::vector<int8_t>& unmasked_status) {

  if (!unmasked_status.empty()) unmasked_status.clear();
  unmasked_status.reserve(pix.size());

  for (uint32_t i=0;i<pix.size();i++){
    int8_t pixel_unmasked_status = 0;
    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_unmasked_status = sub_map_[k].FindUnmaskedStatus(pix[i]);

    unmasked_status.push_back(pixel_unmasked_status);
  }
}

int8_t Map::FindUnmaskedStatus(Map& stomp_map) {
  // Get the status for the first pixel in the map to seed our initial status.
  MapIterator iter=stomp_map.Begin();
  int8_t map_unmasked_status = 0;
  uint32_t k = iter.second->Superpixnum();

  if (sub_map_[k].Initialized())
    map_unmasked_status = sub_map_[k].FindUnmaskedStatus(*(iter.second));

  stomp_map.Iterate(&iter);

  // Now iterate over the rest of the map to figure out the global status.
  for (;iter!=stomp_map.End();stomp_map.Iterate(&iter)) {
    int8_t unmasked_status = 0;
    k = iter.second->Superpixnum();

    if (sub_map_[k].Initialized())
      unmasked_status = sub_map_[k].FindUnmaskedStatus(*(iter.second));

    if (map_unmasked_status == 1) {
      // If we currently thought that the input Map was completely inside of our
      // Map, but find that this Pixel is either outside the Map or only
      // partially inside the Map, then we switch the global status to partial.
      if (unmasked_status == 0 || unmasked_status == -1)
	map_unmasked_status = -1;
    }

    if (map_unmasked_status == 0) {
      // If we currently thought that the input Map was completely outside the
      // Map, but find that this Pixel is either fully or partially contained
      // in the Map, then we switch the global status to partial.
      if (unmasked_status == 1 || unmasked_status == -1)
	map_unmasked_status = -1;
    }

    if (map_unmasked_status == -1) {
      // If we find that the input Map's unmasked status is partial, then no
      // further testing will change that status.  At this point, we can break
      // our loop and return the global status.
      break;
    }
  }

  return map_unmasked_status;
}

double Map::FindAverageWeight(Pixel& pix) {
  double weighted_average = 0.0;
  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized())
    weighted_average = sub_map_[k].FindAverageWeight(pix);

  return weighted_average;
}

void Map::FindAverageWeight(PixelVector& pix,
			    std::vector<double>& weighted_average) {
  if (!weighted_average.empty()) weighted_average.clear();

  for (uint32_t i=0;i<pix.size();i++) {
    double pixel_weighted_average = 0.0;
    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_weighted_average = sub_map_[k].FindAverageWeight(pix[i]);

    weighted_average.push_back(pixel_weighted_average);
  }
}

void Map::FindAverageWeight(PixelVector& pix) {
  for (uint32_t i=0;i<pix.size();i++) {
    double pixel_weighted_average = 0.0;
    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_weighted_average = sub_map_[k].FindAverageWeight(pix[i]);
    pix[i].SetWeight(pixel_weighted_average);
  }
}

void Map::FindMatchingPixels(Pixel& pix, PixelVector& match_pix,
			     bool use_local_weights) {
  if (!match_pix.empty()) match_pix.clear();

  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    PixelVector tmp_pix;

    sub_map_[k].FindMatchingPixels(pix,tmp_pix,use_local_weights);

    if (!tmp_pix.empty())
      for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
        match_pix.push_back(*iter);
  }
}

void Map::FindMatchingPixels(PixelVector& pix, PixelVector& match_pix,
			     bool use_local_weights) {
  if (!match_pix.empty()) match_pix.clear();

  for (uint32_t i=0;i<pix.size();i++) {

    uint32_t k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized()) {
      PixelVector tmp_pix;

      sub_map_[k].FindMatchingPixels(pix[i],tmp_pix,use_local_weights);

      if (!tmp_pix.empty())
        for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
          match_pix.push_back(*iter);
    }
  }
}

void Map::GenerateRandomPoints(AngularVector& ang, uint32_t n_point,
			       bool use_weighted_sampling) {
  if (!ang.empty()) ang.clear();
  ang.reserve(n_point);

  double minimum_probability = -0.0001;
  double probability_slope = 0.0;

  if (use_weighted_sampling) {
    if (max_weight_ - min_weight_ < 0.0001) {
      use_weighted_sampling = false;
    } else {
      minimum_probability = 1.0/(max_weight_ - min_weight_ + 1.0);
      probability_slope =
          (1.0 - minimum_probability)/(max_weight_ - min_weight_);
    }
  }

  PixelVector superpix;
  Coverage(superpix);

  MTRand mtrand;
  mtrand.seed();

  for (uint32_t m=0;m<n_point;m++) {
    bool keep = false;
    double lambda, eta, z, weight, probability_limit;
    AngularCoordinate tmp_ang(0.0,0.0);
    uint32_t n,k;

    while (!keep) {
      n = mtrand.randInt(superpix.size()-1);
      k = superpix[n].Superpixnum();

      z = sub_map_[k].ZMin() + mtrand.rand(sub_map_[k].ZMax() -
					   sub_map_[k].ZMin());
      lambda = asin(z)*Stomp::RadToDeg;
      eta = sub_map_[k].EtaMin() + mtrand.rand(sub_map_[k].EtaMax() -
					       sub_map_[k].EtaMin());
      tmp_ang.SetSurveyCoordinates(lambda,eta);

      keep = sub_map_[k].FindLocation(tmp_ang,weight);

      if (use_weighted_sampling && keep) {
        probability_limit =
            minimum_probability + (weight - min_weight_)*probability_slope;
        if (mtrand.rand(1.0) > probability_limit) keep = false;
      }
    }

    ang.push_back(tmp_ang);
  }
}

void Map::GenerateRandomPoints(WAngularVector& ang, WAngularVector& input_ang) {
  if (!ang.empty()) ang.clear();
  ang.reserve(input_ang.size());

  PixelVector superpix;
  Coverage(superpix);

  MTRand mtrand;
  mtrand.seed();

  WeightedAngularCoordinate tmp_ang;
  for (uint32_t m=0;m<input_ang.size();m++) {
    bool keep = false;
    double lambda, eta, z, map_weight;
    uint32_t n,k;

    while (!keep) {
      n = mtrand.randInt(superpix.size()-1);
      k = superpix[n].Superpixnum();

      z = sub_map_[k].ZMin() + mtrand.rand(sub_map_[k].ZMax() -
					   sub_map_[k].ZMin());
      lambda = asin(z)*Stomp::RadToDeg;
      eta = sub_map_[k].EtaMin() + mtrand.rand(sub_map_[k].EtaMax() -
					       sub_map_[k].EtaMin());
      tmp_ang.SetSurveyCoordinates(lambda,eta);

      keep = sub_map_[k].FindLocation(tmp_ang, map_weight);
    }
    tmp_ang.SetWeight(input_ang[m].Weight());
    ang.push_back(tmp_ang);
  }
}

void Map::GenerateRandomPoints(WAngularVector& ang,
			       std::vector<double>& weights) {
  if (!ang.empty()) ang.clear();
  ang.reserve(weights.size());

  PixelVector superpix;
  Coverage(superpix);

  MTRand mtrand;
  mtrand.seed();

  WeightedAngularCoordinate tmp_ang;
  for (uint32_t m=0;m<weights.size();m++) {
    bool keep = false;
    double lambda, eta, z, map_weight;
    uint32_t n,k;

    while (!keep) {
      n = mtrand.randInt(superpix.size()-1);
      k = superpix[n].Superpixnum();

      z = sub_map_[k].ZMin() + mtrand.rand(sub_map_[k].ZMax() -
					   sub_map_[k].ZMin());
      lambda = asin(z)*Stomp::RadToDeg;
      eta = sub_map_[k].EtaMin() + mtrand.rand(sub_map_[k].EtaMax() -
					       sub_map_[k].EtaMin());
      tmp_ang.SetSurveyCoordinates(lambda,eta);

      keep = sub_map_[k].FindLocation(tmp_ang, map_weight);
    }
    tmp_ang.SetWeight(weights[m]);
    ang.push_back(tmp_ang);
  }
}

bool Map::Write(std::string& OutputFile, bool hpixel_format,
		bool weighted_map) {
  std::ofstream output_file(OutputFile.c_str());

  if (output_file.is_open()) {
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
        PixelVector pix;

        Pixels(pix,k);

        for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
          if (hpixel_format) {
            if (weighted_map) {
              output_file << iter->HPixnum() << " " <<
                  iter->Superpixnum() << " " <<
                  iter->Resolution() << " " <<
                  iter->Weight() << "\n";
            } else {
              output_file << iter->HPixnum() << " " <<
                  iter->Superpixnum() << " " <<
                  iter->Resolution() << "\n";
            }
          } else {
            if (weighted_map) {
              output_file << iter->Pixnum() << " " <<
                  iter->Resolution() << " " <<
                  iter->Weight() << "\n";
            } else {
              output_file << iter->Pixnum() << " " <<
                  iter->Resolution() << "\n";
            }
          }
        }
      }
    }

    output_file.close();

    return true;
  } else {
    return false;
  }
}

bool Map::Read(std::string& InputFile, bool hpixel_format,
		bool weighted_map) {
	Clear();

	std::ifstream input_file(InputFile.c_str());

	uint32_t hpixnum, superpixnum, pixnum;
	uint16_t resolution;
	int tmpres;
	double weight;
	bool found_file = false;

	if (input_file) {
		found_file = true;
		while (!input_file.eof()) {
			if (hpixel_format) {
				if (weighted_map) {
					input_file >> hpixnum >> superpixnum >> tmpres >> weight;
				} else {
					input_file >> hpixnum >> superpixnum >> tmpres;
					weight = 1.0;
				}
			} else {
				if (weighted_map) {
					input_file >> pixnum >> tmpres >> weight;
				} else {
					input_file >> pixnum >> tmpres;
					weight = 1.0;
				}
			}

			// to deal with old -1 resolutions in stripe entries
			resolution = (tmpres > 0) ? tmpres : 0;
			if (!input_file.eof() 
					&& (resolution % 2 == 0) 
					&& (resolution > 0) ) {
				if (!hpixel_format) {
					Pixel::Pix2HPix(resolution, pixnum, hpixnum, superpixnum);
				}
				Pixel tmp_pix(resolution, hpixnum, superpixnum, weight);
				sub_map_[superpixnum].AddPixel(tmp_pix);
			}
		}

		input_file.close();

		bool found_beginning = false;
		for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
			if (iter->Initialized()) {
				if (iter->Unsorted()) iter->Resolve();

				if (!found_beginning) {
					begin_ = MapIterator(iter->Superpixnum(), iter->Begin());
					found_beginning = true;
				}
				end_ = MapIterator(iter->Superpixnum(), iter->End());

				area_ += iter->Area();
				size_ += iter->Size();
				if (min_resolution_ > iter->MinResolution())
					min_resolution_ = iter->MinResolution();
				if (max_resolution_ < iter->MaxResolution())
					max_resolution_ = iter->MaxResolution();
				if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
				if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
				for (uint16_t resolution_iter=Stomp::HPixResolution, i=0;
						i<Stomp::ResolutionLevels;resolution_iter*=2, i++) {
					pixel_count_[resolution_iter] += iter->PixelCount(resolution_iter);
				}
			}
		}

		if (!found_beginning) found_file = false;
	} else {
		std::cout << InputFile << " does not exist!.  No Map ingested\n";
	}

	return found_file;
}

double Map::AverageWeight() {
  double unmasked_fraction = 0.0, weighted_average = 0.0;

  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Initialized()) {
      weighted_average += iter->AverageWeight()*iter->Area();
      unmasked_fraction += iter->Area();
    }
  }

  if (unmasked_fraction > 0.000000001) weighted_average /= unmasked_fraction;

  return weighted_average;
}

bool Map::IngestMap(PixelVector& pix, bool destroy_copy) {
  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
    uint32_t k = iter->Superpixnum();
    sub_map_[k].AddPixel(*iter);
  }

  if (destroy_copy) pix.clear();

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    if (sub_map_[k].Unsorted()) sub_map_[k].Resolve();

  return Initialize();
}

bool Map::IngestMap(Map& stomp_map, bool destroy_copy) {
  PixelVector super_pix;

  stomp_map.Coverage(super_pix);

  for (PixelIterator iter=super_pix.begin();iter!=super_pix.end();++iter) {
    PixelVector tmp_pix;

    stomp_map.Pixels(tmp_pix,iter->Superpixnum());

    for (uint32_t i=0;i<tmp_pix.size();i++) {
      sub_map_[iter->Superpixnum()].AddPixel(tmp_pix[i]);
    }
  }

  if (destroy_copy) stomp_map.Clear();

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    if (sub_map_[k].Unsorted()) sub_map_[k].Resolve();

  return Initialize();
}

bool Map::IntersectMap(PixelVector& pix) {
  Map stomp_map;

  stomp_map.Initialize(pix);

  return IntersectMap(stomp_map);
}

bool Map::IntersectMap(Map& stomp_map) {
  bool found_overlapping_area = false;

  uint32_t superpixnum = 0;

  // First, just check to see that we've got some overlapping area between
  // the two maps.
  while (superpixnum < Stomp::MaxSuperpixnum && !found_overlapping_area) {
    if (sub_map_[superpixnum].Initialized() &&
	stomp_map.ContainsSuperpixel(superpixnum)) {
      if (Area(superpixnum) < stomp_map.Area(superpixnum)) {
	PixelVector tmp_pix;
        PixelVector match_pix;

        sub_map_[superpixnum].Pixels(tmp_pix);

	// FindMatchingPixels using the weights from this map
	stomp_map.FindMatchingPixels(tmp_pix, match_pix, false);

	if (!match_pix.empty()) found_overlapping_area = true;
      } else {
	PixelVector tmp_pix;
        PixelVector match_pix;

        stomp_map.Pixels(tmp_pix, superpixnum);

	// FindMatchingPixels using the weights from this map
	FindMatchingPixels(tmp_pix, match_pix, true);

	if (!match_pix.empty()) found_overlapping_area = true;
      }
    }
    superpixnum++;
  }

  // Provided that we've got some overlap, now do a full calculation for the
  // whole map.
  if (found_overlapping_area) {
    for (superpixnum=0;superpixnum<Stomp::MaxSuperpixnum;superpixnum++) {
      if (sub_map_[superpixnum].Initialized() &&
	  stomp_map.ContainsSuperpixel(superpixnum)) {
        if (Area(superpixnum) < stomp_map.Area(superpixnum)) {
          PixelVector tmp_pix;
          PixelVector match_pix;

          sub_map_[superpixnum].Pixels(tmp_pix);

          stomp_map.FindMatchingPixels(tmp_pix, match_pix, false);

          sub_map_[superpixnum].Clear();

          if (!match_pix.empty()) {
            found_overlapping_area = true;

            for (PixelIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[superpixnum].AddPixel(*match_iter);
            }
            sub_map_[superpixnum].Resolve();

            match_pix.clear();
          }

	  tmp_pix.clear();
        } else {
          PixelVector tmp_pix;
          PixelVector match_pix;

          stomp_map.Pixels(tmp_pix, superpixnum);

          FindMatchingPixels(tmp_pix, match_pix, true);

          sub_map_[superpixnum].Clear();

          if (!match_pix.empty()) {
            found_overlapping_area = true;

            for (PixelIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[superpixnum].AddPixel(*match_iter);
            }
            sub_map_[superpixnum].Resolve();

            match_pix.clear();
          }

	  tmp_pix.clear();
        }
      } else {
	// If there are no pixels in the input map for this superpixel, then
	// clear it out.
        if (sub_map_[superpixnum].Initialized())
	  sub_map_[superpixnum].Clear();
      }
    }
    found_overlapping_area = Initialize();
  }

  return found_overlapping_area;
}

bool Map::AddMap(PixelVector& pix, bool drop_single) {
  Map stomp_map;
  stomp_map.Initialize(pix);

  return AddMap(stomp_map,drop_single);
}

bool Map::AddMap(Map& stomp_map, bool drop_single) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      // Ok, we've got 2 maps in this superpixel, so we have to break
      // both down and calculate the overlap.
      sub_map_[k].Add(stomp_map, drop_single);
    } else {
      // Ok, only one map covers this superpixel, so we can just copy
      // all of the pixels directly into the final map.  If it's only in
      // our current map, then we don't want to do anything, so we skip that
      // case (unless we're dropping non-overlapping area, in which case we
      // clear that superpixel out).

      if (drop_single) {
        if (sub_map_[k].Initialized()) sub_map_[k].Clear();
      } else {
        if (stomp_map.ContainsSuperpixel(k)) {
          PixelVector added_pix;

          stomp_map.Pixels(added_pix,k);

          for (PixelIterator iter=added_pix.begin();
               iter!=added_pix.end();++iter) sub_map_[k].AddPixel(*iter);

          sub_map_[k].Resolve();
        }
      }
    }
  }

  return Initialize();
}

bool Map::MultiplyMap(PixelVector& pix, bool drop_single) {
  Map stomp_map;
  stomp_map.Initialize(pix);

  return MultiplyMap(stomp_map,drop_single);
}

bool Map::MultiplyMap(Map& stomp_map, bool drop_single) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      // Ok, we've got 2 maps in this superpixel, so we have to break
      // both down and calculate the overlap.
      sub_map_[k].Multiply(stomp_map, drop_single);
    } else {
      // Ok, only one map covers this superpixel, so we can just copy
      // all of the pixels directly into the final map.  If it's only in
      // our current map, then we don't want to do anything, so we skip that
      // case (unless we're dropping non-overlapping area, in which case we
      // clear that superpixel out).

      if (drop_single) {
        if (sub_map_[k].Initialized()) sub_map_[k].Clear();
      } else {
        if (stomp_map.ContainsSuperpixel(k)) {
          PixelVector multi_pix;

          stomp_map.Pixels(multi_pix,k);

          for (PixelIterator iter=multi_pix.begin();
               iter!=multi_pix.end();++iter) sub_map_[k].AddPixel(*iter);

          sub_map_[k].Resolve();
        }
      }
    }
  }

  return Initialize();
}

bool Map::ExcludeMap(PixelVector& pix, bool destroy_copy) {
  Map stomp_map;
  stomp_map.Initialize(pix);

  if (destroy_copy) pix.clear();

  return ExcludeMap(stomp_map,destroy_copy);
}

bool Map::ExcludeMap(Map& stomp_map, bool destroy_copy) {
  PixelVector super_pix;
  stomp_map.Coverage(super_pix);

  for (PixelIterator iter=super_pix.begin();iter!=super_pix.end();++iter) {
    uint32_t superpixnum = iter->Superpixnum();
    if (sub_map_[superpixnum].Initialized()) {
      sub_map_[superpixnum].Exclude(stomp_map);
    }
  }

  if (destroy_copy) stomp_map.Clear();

  return Initialize();
}

bool Map::ImprintMap(PixelVector& pix) {
  Map stomp_map;
  stomp_map.Initialize(pix);

  return ImprintMap(stomp_map);
}

bool Map::ImprintMap(Map& stomp_map) {
  bool found_overlapping_area = false;

  uint32_t k = 0;
  while (k<Stomp::MaxSuperpixnum && !found_overlapping_area) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      if (Area(k) < stomp_map.Area(k)) {
	PixelVector tmp_pix;
        PixelVector match_pix;

        sub_map_[k].Pixels(tmp_pix);

	stomp_map.FindMatchingPixels(tmp_pix,match_pix,false);

	if (!match_pix.empty()) found_overlapping_area = true;
      } else {
	PixelVector tmp_pix;
        PixelVector match_pix;

        stomp_map.Pixels(tmp_pix,k);

	FindMatchingPixels(tmp_pix, match_pix, true);

	if (!match_pix.empty()) found_overlapping_area = true;
      }
    }
    k++;
  }

  if (found_overlapping_area) {
    for (k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
        if (Area(k) < stomp_map.Area(k)) {
          PixelVector tmp_pix;
          PixelVector match_pix;

          sub_map_[k].Pixels(tmp_pix);

          stomp_map.FindMatchingPixels(tmp_pix,match_pix,true);

          sub_map_[k].Clear();

          if (!match_pix.empty()) {
            found_overlapping_area = true;

            for (PixelIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[k].AddPixel(*match_iter);
            }
            sub_map_[k].Resolve();

            match_pix.clear();
          }
        } else {
          PixelVector tmp_pix;
          PixelVector match_pix;

          stomp_map.Pixels(tmp_pix,k);

          FindMatchingPixels(tmp_pix,match_pix,false);

          sub_map_[k].Clear();

          if (!match_pix.empty()) {
            found_overlapping_area = true;

            for (PixelIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[k].AddPixel(*match_iter);
            }
            sub_map_[k].Resolve();

            match_pix.clear();
          }
        }
      } else {
        if (sub_map_[k].Initialized()) sub_map_[k].Clear();
      }
    }
    found_overlapping_area = Initialize();
  }

  return found_overlapping_area;
}

void Map::ScaleWeight(const double weight_scale) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    if (sub_map_[k].Initialized()) sub_map_[k].ScaleWeight(weight_scale);
}

void Map::AddConstantWeight(const double add_weight) {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    if (sub_map_[k].Initialized()) sub_map_[k].AddConstantWeight(add_weight);
}

void Map::InvertWeight() {
  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    if (sub_map_[k].Initialized()) sub_map_[k].InvertWeight();
}

void Map::Pixels(PixelVector& pix, uint32_t superpixnum) {
  if (!pix.empty()) pix.clear();

  if (superpixnum < Stomp::MaxSuperpixnum) {
    sub_map_[superpixnum].Pixels(pix);
  } else {
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
        PixelVector tmp_pix;

        sub_map_[k].Pixels(tmp_pix);

        for (PixelIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
          pix.push_back(*iter);
      }
    }
  }
}

void Map::Iterate(MapIterator* iter) {
  ++iter->second;
  if (iter->second == sub_map_[iter->first].End() &&
      iter->first != end_.first) {
    bool found_next_iterator = false;
    while (iter->first < Stomp::MaxSuperpixnum && !found_next_iterator) {
      iter->first++;
      if (sub_map_[iter->first].Initialized()) {
	iter->second = sub_map_[iter->first].Begin();
	found_next_iterator = true;
      }
    }
  }
}

void Map::Clear() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution;
  max_resolution_ = Stomp::HPixResolution;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  ClearRegions();

  for (uint16_t resolution=Stomp::HPixResolution, i=0;
       i<Stomp::ResolutionLevels;resolution*=2, i++)
    pixel_count_[resolution] = 0;

  if (!sub_map_.empty()) {
    for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter)
      iter->Clear();
    sub_map_.clear();
  }

  sub_map_.reserve(Stomp::MaxSuperpixnum);

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    sub_map_.push_back(SubMap(k));

  begin_ = end_;
}

ScalarSubMap::ScalarSubMap(uint32_t superpixnum) {
  superpixnum_ = superpixnum;
  initialized_ = false;
  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  size_ = 0;
}

ScalarSubMap::~ScalarSubMap() {
  superpixnum_ = Stomp::MaxSuperpixnum;
  initialized_ = false;
  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  size_ = 0;
}

Section::Section() {
  stripe_min_ = stripe_max_ = 0;
}

Section::~Section() {
  stripe_min_ = stripe_max_ = 0;
}

ScalarMap::ScalarMap() {
  area_ = 0.0;
  resolution_ = 0;
  total_points_ = 0;
  mean_intensity_ = 0.0;
  total_intensity_ = 0.0;
  if (!pix_.empty()) pix_.clear();
  if (!sub_map_.empty()) sub_map_.clear();
  ClearRegions();
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;
  initialized_sub_map_ = false;
  map_type_ = ScalarField;
}

ScalarMap::ScalarMap(Map& stomp_map, uint16_t input_resolution,
		     ScalarMapType scalar_map_type,
		     double min_unmasked_fraction,
		     bool use_map_weight_as_intensity) {
  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  map_type_ = scalar_map_type;

  if (use_map_weight_as_intensity && !(map_type_ != ScalarField)) {
    std::cout <<
      "WARNING: Converting MapType to ScalarField to sample " <<
      "input Map Weight\n";
    map_type_ = ScalarField;
  };

  PixelVector superpix;
  stomp_map.Coverage(superpix);

  for (PixelIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    PixelVector sub_pix;
    iter->SubPix(resolution_,sub_pix);

    for (PixelIterator sub_iter=sub_pix.begin();
         sub_iter!=sub_pix.end();++sub_iter) {
      double unmasked_fraction = stomp_map.FindUnmaskedFraction(*sub_iter);
      double initial_intensity = 0.0;
      if (unmasked_fraction > unmasked_fraction_minimum_) {
        if (use_map_weight_as_intensity)
          initial_intensity = stomp_map.FindAverageWeight(*sub_iter);
	ScalarPixel tmp_pix(sub_iter->PixelX(), sub_iter->PixelY(),
			    sub_iter->Resolution(), unmasked_fraction,
			    initial_intensity, 0);
	pix_.push_back(tmp_pix);
      }
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(),pix_.end(),Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
    area_ += iter->Area()*iter->Weight();

  initialized_sub_map_ = _InitializeSubMap();
}

ScalarMap::ScalarMap(ScalarMap& scalar_map,
		     uint16_t input_resolution,
		     double min_unmasked_fraction) {
  if (input_resolution > scalar_map.Resolution()) {
    std::cout << "Cannot make higher resolution density map " <<
      "by resampling. Exiting.\n";
    exit(1);
  }

  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  map_type_ = scalar_map.MapType();

  if (scalar_map.Resolution() == resolution_) {
    pix_.reserve(scalar_map.Size());
    for (ScalarIterator iter=scalar_map.Begin();
	 iter!=scalar_map.End();++iter) pix_.push_back(*iter);
  } else {
    PixelVector superpix;
    scalar_map.Coverage(superpix);

    uint32_t x_min, x_max, y_min, y_max;
    ScalarPixel tmp_pix;
    tmp_pix.SetResolution(resolution_);

    for (PixelIterator iter=superpix.begin();iter!=superpix.end();++iter) {
      iter->SubPix(resolution_,x_min,x_max,y_min,y_max);
      for (uint32_t y=y_min;y<=y_max;y++) {
	for (uint32_t x=x_min;x<=x_max;x++) {
	  tmp_pix.SetPixnumFromXY(x,y);
	  scalar_map.Resample(tmp_pix);
	  if (tmp_pix.Weight() > unmasked_fraction_minimum_)
	    pix_.push_back(tmp_pix);
        }
      }
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(), pix_.end(), Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    area_ += iter->Area()*iter->Weight();
    total_intensity_ += iter->Intensity();
    total_points_ += iter->NPoints();
  }

  initialized_sub_map_ = _InitializeSubMap();
}

ScalarMap::ScalarMap(ScalarVector& pix,
		     ScalarMapType scalar_map_type,
		     double min_unmasked_fraction) {

  resolution_ = pix[0].Resolution();
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  map_type_ = scalar_map_type;

  pix_.reserve(pix.size());

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix.begin();iter!=pix.end();++iter) {
    if (iter->Resolution() != resolution_) {
      std::cout << "Incompatible resolutions in ScalarPixel list.  Exiting.\n";
      exit(2);
    }
    area_ += iter->Area()*iter->Weight();
    total_intensity_ += iter->Intensity();
    total_points_ += iter->NPoints();
    pix_.push_back(*iter);
  }

  sort(pix_.begin(), pix_.end(), Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  initialized_sub_map_ = _InitializeSubMap();
}

ScalarMap::ScalarMap(Map& stomp_map,
		     AngularCoordinate& center, double theta_max,
		     uint16_t input_resolution,
		     ScalarMapType scalar_map_type,
		     double min_unmasked_fraction,
		     double theta_min) {

  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  map_type_ = scalar_map_type;

  ScalarPixel tmp_pix(center,resolution_,0.0,0.0,0);

  PixelVector pix;
  tmp_pix.WithinAnnulus(theta_min,theta_max,pix);

  for (PixelIterator iter=pix.begin();iter!=pix.end();++iter) {
    double unmasked_fraction = stomp_map.FindUnmaskedFraction(*iter);
    if (unmasked_fraction > unmasked_fraction_minimum_) {
      tmp_pix.SetPixnumFromXY(iter->PixelX(), iter->PixelY());
      tmp_pix.SetWeight(unmasked_fraction);
      pix_.push_back(tmp_pix);
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(),pix_.end(),Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
    area_ += iter->Area()*iter->Weight();

  initialized_sub_map_ = _InitializeSubMap();
}

ScalarMap::~ScalarMap() {
  area_ = 0.0;
  resolution_ = 0;
  mean_intensity_ = 0.0;
  total_intensity_ = 0.0;
  if (!pix_.empty()) pix_.clear();
  if (!sub_map_.empty()) sub_map_.clear();
  ClearRegions();
}

bool ScalarMap::_InitializeSubMap() {
  if (!sub_map_.empty()) sub_map_.clear();

  sub_map_.reserve(Stomp::MaxSuperpixnum);

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
    ScalarSubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++)
    sub_map_[k].SetNull(pix_.end());

  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    uint32_t k = iter->Superpixnum();
    sub_map_[k].AddToArea(iter->Resolution(), iter->Weight());
    sub_map_[k].AddToIntensity(iter->Intensity(), iter->NPoints());
    if (!sub_map_[k].Initialized()) {
      sub_map_[k].SetBegin(iter);
    } else {
      sub_map_[k].SetEnd(iter);
    }
  }

  return true;
}

void ScalarMap::InitializeFromMap(Map& stomp_map, uint16_t input_resolution,
				  bool use_map_weight_as_intensity) {
  uint16_t current_resolution = resolution_;
  Clear();

  if (input_resolution != 0) {
    SetResolution(input_resolution);
  } else {
    SetResolution(current_resolution);
  }

  if (use_map_weight_as_intensity && !(map_type_ != ScalarField)) {
    std::cout <<
      "WARNING: Converting MapType to ScalarField to sample " <<
      "input Map Weight\n";
    map_type_ = ScalarField;
  };

  PixelVector superpix;
  stomp_map.Coverage(superpix);

  for (PixelIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    PixelVector sub_pix;
    iter->SubPix(resolution_,sub_pix);

    for (PixelIterator sub_iter=sub_pix.begin();
         sub_iter!=sub_pix.end();++sub_iter) {
      double unmasked_fraction = stomp_map.FindUnmaskedFraction(*sub_iter);
      double initial_intensity = 0.0;
      if (unmasked_fraction > unmasked_fraction_minimum_) {
        if (use_map_weight_as_intensity)
          initial_intensity = stomp_map.FindAverageWeight(*sub_iter);
	ScalarPixel tmp_pix(sub_iter->PixelX(), sub_iter->PixelY(),
			    sub_iter->Resolution(), unmasked_fraction,
			    initial_intensity, 0);
	pix_.push_back(tmp_pix);
      }
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(), pix_.end(), Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    area_ += iter->Area()*iter->Weight();
    total_intensity_ += iter->Intensity();
  }

  initialized_sub_map_ = _InitializeSubMap();
}

void ScalarMap::InitializeFromScalarMap(ScalarMap& scalar_map,
					uint16_t input_resolution) {
  if (input_resolution > scalar_map.Resolution()) {
    std::cout << "Cannot make higher resolution density map " <<
      "by resampling. Exiting.\n";
    exit(1);
  }

  uint16_t current_resolution = resolution_;
  Clear();

  if (input_resolution != 0) {
    SetResolution(input_resolution);
  } else {
    SetResolution(current_resolution);
  }

  map_type_ = scalar_map.MapType();

  if (scalar_map.Resolution() == resolution_) {
    pix_.reserve(scalar_map.Size());
    for (ScalarIterator iter=scalar_map.Begin();
	 iter!=scalar_map.End();++iter) pix_.push_back(*iter);
  } else {
    PixelVector superpix;
    scalar_map.Coverage(superpix);

    uint32_t x_min, x_max, y_min, y_max;
    ScalarPixel tmp_pix;
    tmp_pix.SetResolution(resolution_);

    for (PixelIterator iter=superpix.begin();iter!=superpix.end();++iter) {
      iter->SubPix(resolution_,x_min,x_max,y_min,y_max);
      for (uint32_t y=y_min;y<=y_max;y++) {
	for (uint32_t x=x_min;x<=x_max;x++) {
	  tmp_pix.SetPixnumFromXY(x,y);
	  scalar_map.Resample(tmp_pix);
	  if (tmp_pix.Weight() > unmasked_fraction_minimum_)
	    pix_.push_back(tmp_pix);
        }
      }
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(), pix_.end(), Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    area_ += iter->Area()*iter->Weight();
    total_intensity_ += iter->Intensity();
    total_points_ += iter->NPoints();
  }

  initialized_sub_map_ = _InitializeSubMap();
}

void ScalarMap::InitializeFromScalarPixels(ScalarVector& pix,
					   ScalarMapType scalar_map_type) {

  resolution_ = pix[0].Resolution();
  map_type_ = scalar_map_type;

  if (!pix_.empty()) pix_.clear();
  pix_.reserve(pix.size());

  area_ = 0.0;
  total_intensity_ = 0.0;
  total_points_ = 0;
  for (ScalarIterator iter=pix.begin();iter!=pix.end();++iter) {
    if (iter->Resolution() != resolution_) {
      std::cout << "Incompatible resolutions in ScalarPixel list.  Exiting.\n";
      exit(2);
    }
    area_ += iter->Area()*iter->Weight();
    total_intensity_ += iter->Intensity();
    total_points_ += iter->NPoints();
    pix_.push_back(*iter);
  }

  sort(pix_.begin(), pix_.end(), Pixel::SuperPixelBasedOrder);
  mean_intensity_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_intensity_ = false;

  initialized_sub_map_ = _InitializeSubMap();
}

bool ScalarMap::AddToMap(AngularCoordinate& ang, double object_weight) {
  ScalarPixel tmp_pix(ang,resolution_,object_weight,1);

  uint32_t k = tmp_pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    ScalarPair iter;
    iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
		       Pixel::SuperPixelBasedOrder);
    if (iter.first != iter.second) {
      if (map_type_ == ScalarField) {
	iter.first->AddToIntensity(object_weight, 0);
	sub_map_[k].AddToIntensity(object_weight, 0);
      } else {
	iter.first->AddToIntensity(object_weight, 1);
	sub_map_[k].AddToIntensity(object_weight, 1);
      }
      total_intensity_ += object_weight;
      total_points_++;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool ScalarMap::AddToMap(WeightedAngularCoordinate& ang) {
  ScalarPixel tmp_pix(ang,resolution_,ang.Weight());

  uint32_t k = tmp_pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    ScalarPair iter;
    iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
		       Pixel::SuperPixelBasedOrder);
    if (iter.first != iter.second) {
      if (map_type_ == ScalarField) {
	iter.first->AddToIntensity(ang.Weight(), 0);
	sub_map_[k].AddToIntensity(ang.Weight(), 0);
      } else {
	iter.first->AddToIntensity(ang.Weight(), 1);
	sub_map_[k].AddToIntensity(ang.Weight(), 1);
      }
      total_intensity_ += ang.Weight();
      total_points_++;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool ScalarMap::AddToMap(Pixel& pix) {
  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized() && (pix.Resolution() <= resolution_) &&
      (map_type_ == ScalarField)) {

    PixelVector pixVec;
    pix.SubPix(resolution_, pixVec);
    for (PixelIterator pix_iter=pixVec.begin();
	 pix_iter!=pixVec.end();++pix_iter) {

      ScalarPixel tmp_pix(pix_iter->PixelX(), pix_iter->PixelY(),
			  pix_iter->Resolution());

      ScalarPair iter;
      iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
			 Pixel::SuperPixelBasedOrder);
      if (iter.first != iter.second) {
	iter.first->SetIntensity(pix.Weight());
	sub_map_[k].AddToIntensity(pix.Weight());
	sub_map_[k].SetNPoints(0);
	total_intensity_ += pix.Weight();
      }
    }
    return true;
  } else {
    return false;
  }
}

void ScalarMap::Coverage(PixelVector& superpix, uint16_t resolution) {
  if (!superpix.empty()) superpix.clear();

  if (resolution > resolution_) {
    std::cout << "WARNING: Requested resolution is higher than " <<
      "the map resolution!\nReseting to map resolution...\n";
    resolution = resolution_;
  }

  if (resolution == Stomp::HPixResolution) {
    // If we're dealing with a coverage map at superpixel resolution (the
    // default behavior), then this is easy.  Just iterate over the submaps
    // and keep those that have been initialized.
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
	// We store the unmasked fraction of each superpixel in the weight
	// value in case that's useful.
	Pixel tmp_pix(Stomp::HPixResolution, k,
		      sub_map_[k].Area()/Stomp::HPixArea);
	superpix.push_back(tmp_pix);
      }
    }
  } else {
    for (uint32_t k=0;k<Stomp::MaxSuperpixnum;k++) {
      if (sub_map_[k].Initialized()) {
	Pixel tmp_pix(Stomp::HPixResolution, k, 1.0);

	PixelVector sub_pix;
	tmp_pix.SubPix(resolution, sub_pix);

	for (PixelIterator iter=sub_pix.begin();iter!=sub_pix.end();++iter) {
	  // For each of the pixels in the superpixel, we check its status
	  // against the current map.  This is faster than finding the unmasked
	  // fraction directly and immediately tells us which pixels we can
	  // eliminate and which of those we do keep require further
	  // calculations to find the unmasked fraction.
	  int8_t unmasked_status = FindUnmaskedStatus(*iter);
	  if (unmasked_status != 0) {
	    iter->SetWeight(FindUnmaskedFraction(*iter));
	    superpix.push_back(*iter);
	  }
	}
      }
    }
    sort(superpix.begin(), superpix.end(), Pixel::SuperPixelBasedOrder);
  }
}

bool ScalarMap::Covering(Map& stomp_map, uint32_t maximum_pixels) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  PixelVector pix;
  Coverage(pix);

  bool met_pixel_requirement;
  if (pix.size() > maximum_pixels) {
    // If the number of requested pixels is smaller than the number of
    // superpixels in the map, then we'd never be able to meet that requirement.
    // In this case, we set the output Map to the coarsest possible case
    // and return false.
    met_pixel_requirement = false;

    stomp_map.Initialize(pix);
  } else {
    // Ok, in this case, we can definitely produce a map that has at most
    // maximum_pixels in it.
    met_pixel_requirement = true;

    // To possibly save ourselves some effort, we start by making the Map
    // equivalent of our current map.
    Coverage(pix, resolution_);
    stomp_map.Initialize(pix);

    if (maximum_pixels < stomp_map.Size()) {
      // If our Map is still too large, we can use the Map's
      // Covering method to do our work for us.
      Map tmp_map;
      met_pixel_requirement = stomp_map.Covering(tmp_map, maximum_pixels);
      if (tmp_map.Size() < maximum_pixels) {
	stomp_map = tmp_map;
      }
    }
  }

  return met_pixel_requirement;
}

int8_t ScalarMap::FindUnmaskedStatus(Pixel& pix) {
  // By default, there is no overlap between the map and the input pixel.
  int8_t unmasked_status = 0;

  uint32_t k = pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    // Provided that the input pixel is contained in a Superpixnum where we've
    // got pixels, we have three cases: the input pixel is at lower, equal or
    // higher resolution.  The last two cases can be handled in the same code
    // block since we're looking for matching pixels in our current set.  Note
    // that the higher resolution case output doesn't exactly mean the same
    // thing as it does in the Map class since the pixels in ScalarMaps all have
    // an unmasked fraction which isn't necessarily 1.
    if (pix.Resolution() >= resolution_) {
      Pixel tmp_pix = pix;
      tmp_pix.SetToSuperPix(resolution_);

      ScalarPair iter = equal_range(sub_map_[k].Begin(),
				    sub_map_[k].End(), tmp_pix,
				    Pixel::SuperPixelBasedOrder);
      if (iter.first != iter.second) unmasked_status = 1;
    }

    // If the input pixel is larger than the ScalarMap pixels, then we scan
    // through the pixels in the SubScalarMap to see if any are contained in
    // the input pixel.
    if (pix.Resolution() < resolution_) {
      ScalarIterator iter = sub_map_[k].Begin();

      while (iter!=sub_map_[k].End() && unmasked_status == 0) {
	if (pix.Contains(*iter)) unmasked_status = -1;
	++iter;
      }
    }
  }

  return unmasked_status;
}

double ScalarMap::FindUnmaskedFraction(Pixel& pix) {
  ScalarPixel scalar_pix(pix.PixelX(), pix.PixelY(), pix.Resolution());

  Resample(scalar_pix);

  return scalar_pix.Weight();
}

void ScalarMap::Resample(ScalarPixel& pix) {
  double unmasked_fraction = 0.0, total_intensity = 0.0;
  double weighted_intensity = 0.0;
  uint32_t total_points = 0;

  if (pix.Resolution() > resolution_) {
    unmasked_fraction = -1.0;
    total_intensity = -1.0;
    weighted_intensity = -1.0;
    total_points = 0;
  } else {
    uint32_t k = pix.Superpixnum();

    if (sub_map_[k].Initialized()) {
      if (pix.Resolution() == resolution_) {
        ScalarPixel tmp_pix(pix.PixelX(),pix.PixelY(),pix.Resolution());

        ScalarPair iter = equal_range(sub_map_[k].Begin(),
				      sub_map_[k].End(),tmp_pix,
				      Pixel::SuperPixelBasedOrder);
        if (iter.first != iter.second) {
          unmasked_fraction = iter.first->Weight();
	  total_intensity = iter.first->Intensity();
	  weighted_intensity = total_intensity*unmasked_fraction;
	  total_points = iter.first->NPoints();
        }
      } else {
        uint32_t y_min, y_max, x_min, x_max;
        double pixel_fraction =
	  1.0*pix.Resolution()*pix.Resolution()/(resolution_*resolution_);

        pix.SubPix(resolution_,x_min,x_max,y_min,y_max);

        for (uint32_t y=y_min;y<=y_max;y++) {
          Pixel tmp_left(x_min,y,resolution_);
          Pixel tmp_right(x_max,y,resolution_);

          // Ok, we know that tmp_left and tmp_right are in the
          // same superpixel so we can use strict order to make sure
          // that we don't go past tmp_right.
          if (tmp_right.HPixnum() >= sub_map_[k].Begin()->HPixnum()) {
            while (tmp_left.HPixnum() < sub_map_[k].Begin()->HPixnum())
              tmp_left.Iterate();

            ScalarIterator iter =
	      lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
			  Pixel::SuperPixelBasedOrder);
            if (iter != sub_map_[k].End()) {
              while (iter->PixelY() < y) ++iter;

              while (iter != sub_map_[k].End()) {
                if (iter->HPixnum() <= tmp_right.HPixnum()) {
                  unmasked_fraction += pixel_fraction*iter->Weight();
		  total_intensity += iter->Intensity();
		  weighted_intensity +=
		    iter->Intensity()*pixel_fraction*iter->Weight();
		  total_points += iter->NPoints();
                  ++iter;
                } else {
                  iter = sub_map_[k].End();
                }
              }
            }
          }
        }
      }
    }
  }

  if (unmasked_fraction > 0.0000001) {
    // If we normalize weighted_intensity by the unmasked fraction, we have an
    // area-averaged value of the intensity over the pixel.
    weighted_intensity /= unmasked_fraction;

    // If our pixels were encoding the over-density of the scalar fields, then
    // we need to convert those values back into the raw values.
    if (converted_to_overdensity_) {
      if (map_type_ == DensityField) {
	weighted_intensity =
	  weighted_intensity*mean_intensity_ + mean_intensity_;
      } else {
	weighted_intensity += mean_intensity_;
      }
      if (map_type_ == DensityField) {
	// For a DensityField, we want to return the total intensity for all of
	// the area subtended by the input pixel.  If we were dealing with a raw
	// map, then that value is already in total intensity.  Since we've
	// got an over-density map, then we need to convert the weighted
	// average into a total by incorporating the unmasked area.
	total_intensity = weighted_intensity*unmasked_fraction*pix.Area();
      }
      if (map_type_ == SampledField) {
	// Likewise for a SampledField, but the normalization is based on the
	// total number of points in the pixel.
	total_intensity = weighted_intensity*total_points;
      }
    }

    if (map_type_ == ScalarField) {
      // For a ScalarField, we want the average value over the area indicated
      // by the input pixel.
      total_intensity = weighted_intensity;
    }
  } else {
    unmasked_fraction = 0.0;
    total_intensity = 0.0;
    total_points = 0;
  }

  pix.SetWeight(unmasked_fraction);
  pix.SetIntensity(total_intensity);
  pix.SetNPoints(total_points);
}

double ScalarMap::FindIntensity(Pixel& pix) {
  ScalarPixel scalar_pix(pix.PixelX(), pix.PixelY(), pix.Resolution());

  Resample(scalar_pix);

  return scalar_pix.Intensity();
}

double ScalarMap::FindDensity(Pixel& pix) {
  ScalarPixel scalar_pix(pix.PixelX(), pix.PixelY(), pix.Resolution());

  Resample(scalar_pix);

  return scalar_pix.Intensity()/(scalar_pix.Weight()*scalar_pix.Area());
}

double ScalarMap::FindPointDensity(Pixel& pix) {
  ScalarPixel scalar_pix(pix.PixelX(), pix.PixelY(), pix.Resolution());

  Resample(scalar_pix);

  return 1.0*scalar_pix.NPoints()/(scalar_pix.Weight()*scalar_pix.Area());
}

double ScalarMap::FindLocalArea(AngularCoordinate& ang,
				double theta_max, double theta_min) {
  Pixel center_pix(ang,resolution_,0.0);

  PixelVector pixVec;
  if (theta_min < 0.0) {
    center_pix.WithinRadius(theta_max, pixVec);
  } else {
    center_pix.WithinAnnulus(theta_min, theta_max, pixVec);
  }

  double total_area = 0.0;
  for (PixelIterator iter=pixVec.begin();iter!=pixVec.end();++iter) {
    total_area += FindUnmaskedFraction(*iter);
  }

  if (total_area > 0.0000001) {
    return total_area*center_pix.Area();
  } else {
    return 0.0;
  }
}

double ScalarMap::FindLocalIntensity(AngularCoordinate& ang,
				     double theta_max, double theta_min) {
  Pixel center_pix(ang,resolution_,0.0);

  PixelVector pixVec;
  if (theta_min < 0.0) {
    center_pix.WithinRadius(theta_max, pixVec);
  } else {
    center_pix.WithinAnnulus(theta_min, theta_max, pixVec);
  }

  double total_intensity = 0.0;
  for (PixelIterator iter=pixVec.begin();iter!=pixVec.end();++iter) {
    total_intensity += FindIntensity(*iter);
  }

  return total_intensity;
}

double ScalarMap::FindLocalDensity(AngularCoordinate& ang,
				   double theta_max, double theta_min) {
  Pixel center_pix(ang,resolution_,0.0);

  PixelVector pixVec;
  if (theta_min < 0.0) {
    center_pix.WithinRadius(theta_max, pixVec);
  } else {
    center_pix.WithinAnnulus(theta_min, theta_max, pixVec);
  }

  double total_area = 0.0;
  double total_intensity = 0.0;
  for (PixelIterator iter=pixVec.begin();iter!=pixVec.end();++iter) {
    total_area += FindUnmaskedFraction(*iter);
    total_intensity += FindIntensity(*iter);
  }

  if (total_area > 0.0000001) {
    return total_intensity/(total_area*center_pix.Area());
  } else {
    return 0.0;
  }
}

double ScalarMap::FindLocalPointDensity(AngularCoordinate& ang,
					double theta_max, double theta_min) {
  Pixel center_pix(ang,resolution_,0.0);

  PixelVector pixVec;
  if (theta_min < 0.0) {
    center_pix.WithinRadius(theta_max, pixVec);
  } else {
    center_pix.WithinAnnulus(theta_min, theta_max, pixVec);
  }

  double total_area = 0.0;
  double total_point_density = 0.0;
  for (PixelIterator iter=pixVec.begin();iter!=pixVec.end();++iter) {
    double area = FindUnmaskedFraction(*iter);
    if (area > 0.0000001) {
      total_area += area;
      total_point_density += FindPointDensity(*iter)*area*center_pix.Area();
    }
  }

  if (total_area > 0.0000001) {
    return total_point_density/(total_area*center_pix.Area());
  } else {
    return 0.0;
  }
}

void ScalarMap::CalculateMeanIntensity() {
  double sum_pixel = 0.0;
  mean_intensity_ = 0.0;

  if (map_type_ == ScalarField) {
    for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      mean_intensity_ += iter->Intensity()*iter->Weight();
      sum_pixel += iter->Weight();
    }
  }
  if (map_type_ == DensityField) {
    for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      mean_intensity_ += iter->Intensity()/(iter->Area()*iter->Weight());
      sum_pixel += 1.0;
    }
  }
  if (map_type_ == SampledField) {
    for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      mean_intensity_ += iter->MeanIntensity()*iter->Weight();
      sum_pixel += iter->Weight();
    }
  }

  mean_intensity_ /= sum_pixel;
  calculated_mean_intensity_ = true;
}

void ScalarMap::ConvertToOverDensity() {
  if (!calculated_mean_intensity_) CalculateMeanIntensity();

  // Only do this conversion if we've got raw intensity values in our map.
  if (!converted_to_overdensity_) {
    if (map_type_ == DensityField) {
      for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
	iter->ConvertToFractionalOverDensity(mean_intensity_);
    } else {
      for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
	iter->ConvertToOverDensity(mean_intensity_);
    }
  }

  converted_to_overdensity_ = true;
}

void ScalarMap::ConvertFromOverDensity() {
  if (!calculated_mean_intensity_) CalculateMeanIntensity();

  // Only do this conversion if we've got over-density values in our map.
  if (converted_to_overdensity_) {
    if (map_type_ == DensityField) {
      for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
	iter->ConvertFromFractionalOverDensity(mean_intensity_);
    } else {
      for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
	iter->ConvertFromOverDensity(mean_intensity_);
    }
  }

  converted_to_overdensity_ = false;
}

bool ScalarMap::ImprintMap(Map& stomp_map) {
  PixelVector pixVec;

  pixVec.reserve(pix_.size());

  for (ScalarIterator iter=pix_.begin();iter!=pix_.end();++iter)
    pixVec.push_back(Pixel(iter->PixelX(), iter->PixelY(),
			   iter->Resolution(), iter->Intensity()));

  return stomp_map.ImprintMap(pixVec);
}

void ScalarMap::AutoCorrelate(AngularCorrelation& wtheta) {
  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    bool convert_back_to_raw = false;
    if (!converted_to_overdensity_) {
      ConvertToOverDensity();
      convert_back_to_raw = true;
    }

    for (ThetaIterator theta_iter=theta_begin;
	 theta_iter!=theta_end;++theta_iter) AutoCorrelate(theta_iter);

    if (convert_back_to_raw) ConvertFromOverDensity();
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }
}

void ScalarMap::AutoCorrelate(ThetaIterator theta_iter) {
  bool convert_back_to_raw = false;
  if (!converted_to_overdensity_) {
    ConvertToOverDensity();
    convert_back_to_raw = true;
  }

  theta_iter->ResetPixelWtheta();

  uint32_t y_min, y_max, k;
  std::vector<uint32_t> x_min, x_max;
  ScalarIterator iter;
  ScalarPixel tmp_left, tmp_right;
  double costheta = 0.0;
  double theta = theta_iter->ThetaMax();
  tmp_left.SetResolution(resolution_);
  tmp_right.SetResolution(resolution_);

  for (ScalarIterator map_iter=pix_.begin();
       map_iter!=pix_.end();++map_iter) {
    map_iter->XYBounds(theta,x_min,x_max,y_min,y_max,true);

    for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
      tmp_left.SetPixnumFromXY(x_min[n],y);
      tmp_right.SetPixnumFromXY(x_max[n],y);
      k = tmp_left.Superpixnum();

      if (Pixel::SuperPixelBasedOrder(*map_iter,tmp_right)) {
	if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

	  // This is the same schema for iterating through the bounding
	  // pixels as in FindLocalArea and FindLocalDensity.
	  while (k != tmp_right.Superpixnum()) {
	    if (sub_map_[k].Initialized()) {
	      if (sub_map_[k].Begin()->PixelY() <= y) {
		iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
				   tmp_left,
				   Pixel::SuperPixelBasedOrder);
		while ((iter->PixelY() == y) &&
		       (iter != sub_map_[k].End())) {
		  if (Pixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		    costheta =
		      map_iter->UnitSphereX()*iter->UnitSphereX() +
		      map_iter->UnitSphereY()*iter->UnitSphereY() +
		      map_iter->UnitSphereZ()*iter->UnitSphereZ();
		    if (theta_iter->WithinCosBounds(costheta)) {
		      theta_iter->AddToPixelWtheta(map_iter->Intensity()*
						   map_iter->Weight()*
						   iter->Intensity()*
						   iter->Weight(),
						   map_iter->Weight()*
						   iter->Weight());
		    }
		  }
		  ++iter;
		}
	      }
	      tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	      tmp_left.Iterate();
	      k = tmp_left.Superpixnum();
	    } else {
	      while (!sub_map_[k].Initialized() &&
		     k != tmp_right.Superpixnum()) {
		tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
		tmp_left.Iterate();
		k = tmp_left.Superpixnum();
	      }
	    }
	  }
	}

	if (sub_map_[k].Initialized()) {
	  if (Pixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
					       tmp_left) ||
	      Pixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
			       tmp_left,
			       Pixel::SuperPixelBasedOrder);
	    while (iter != sub_map_[k].End()) {
	      if (Pixel::SuperPixelBasedOrder(*iter,tmp_right)) {
		if (Pixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		  costheta =
		    map_iter->UnitSphereX()*iter->UnitSphereX() +
		    map_iter->UnitSphereY()*iter->UnitSphereY() +
		    map_iter->UnitSphereZ()*iter->UnitSphereZ();
		  if (theta_iter->WithinCosBounds(costheta)) {
		    theta_iter->AddToPixelWtheta(map_iter->Intensity()*
						 map_iter->Weight()*
						 iter->Intensity()*
						 iter->Weight(),
						 map_iter->Weight()*
						 iter->Weight());
		  }
		}
		++iter;
	      } else {
		iter = sub_map_[k].End();
	      }
	    }
	  }
	}
      }
    }
  }

  if (convert_back_to_raw) ConvertFromOverDensity();
}

void ScalarMap::AutoCorrelateWithRegions(AngularCorrelation& wtheta) {
  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    bool convert_back_to_raw = false;
    if (!converted_to_overdensity_) {
      ConvertToOverDensity();
      convert_back_to_raw = true;
    }

    for (ThetaIterator theta_iter=theta_begin;
	 theta_iter!=theta_end;++theta_iter)
      AutoCorrelateWithRegions(theta_iter);

    if (convert_back_to_raw) ConvertFromOverDensity();
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }
}

void ScalarMap::AutoCorrelateWithRegions(ThetaIterator theta_iter) {
  bool convert_back_to_raw = false;
  if (!converted_to_overdensity_) {
    ConvertToOverDensity();
    convert_back_to_raw = true;
  }

  if (theta_iter->NRegion() != NRegion()) {
    theta_iter->ClearRegions();
    theta_iter->InitializeRegions(NRegion());
  }

  theta_iter->ResetPixelWtheta();

  uint32_t y_min, y_max, k;
  uint32_t map_region, pix_region;
  std::vector<uint32_t> x_min, x_max;
  ScalarIterator iter;
  ScalarPixel tmp_left, tmp_right;
  double costheta = 0.0;
  double theta = theta_iter->ThetaMax();
  tmp_left.SetResolution(resolution_);
  tmp_right.SetResolution(resolution_);

  for (ScalarIterator map_iter=pix_.begin();
       map_iter!=pix_.end();++map_iter) {
    map_iter->XYBounds(theta,x_min,x_max,y_min,y_max,true);
    map_region = Region(map_iter->SuperPix(RegionResolution()));

    for (uint32_t y=y_min,n=0;y<=y_max;y++,n++) {
      tmp_left.SetPixnumFromXY(x_min[n],y);
      tmp_right.SetPixnumFromXY(x_max[n],y);
      k = tmp_left.Superpixnum();

      if (Pixel::SuperPixelBasedOrder(*map_iter,tmp_right)) {
	if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

	  // This is the same schema for iterating through the bounding
	  // pixels as in FindLocalArea and FindLocalDensity.
	  while (k != tmp_right.Superpixnum()) {
	    if (sub_map_[k].Initialized()) {
	      if (sub_map_[k].Begin()->PixelY() <= y) {
		iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
				   tmp_left,
				   Pixel::SuperPixelBasedOrder);
		while ((iter->PixelY() == y) &&
		       (iter != sub_map_[k].End())) {
		  if (Pixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		    costheta =
		      map_iter->UnitSphereX()*iter->UnitSphereX() +
		      map_iter->UnitSphereY()*iter->UnitSphereY() +
		      map_iter->UnitSphereZ()*iter->UnitSphereZ();
		    if (theta_iter->WithinCosBounds(costheta)) {
		      pix_region =
			Region(iter->SuperPix(RegionResolution()));
		      theta_iter->AddToPixelWtheta(map_iter->Intensity()*
						   map_iter->Weight()*
						   iter->Intensity()*
						   iter->Weight(),
						   map_iter->Weight()*
						   iter->Weight(),
						   map_region, pix_region);
		    }
		  }
		  ++iter;
		}
	      }
	      tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	      tmp_left.Iterate();
	      k = tmp_left.Superpixnum();
	    } else {
	      while (!sub_map_[k].Initialized() &&
		     k != tmp_right.Superpixnum()) {
		tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
		tmp_left.Iterate();
		k = tmp_left.Superpixnum();
	      }
	    }
	  }
	}

	if (sub_map_[k].Initialized()) {
	  if (Pixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
					       tmp_left) ||
	      Pixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
			       tmp_left,
			       Pixel::SuperPixelBasedOrder);
	    while (iter != sub_map_[k].End()) {
	      if (Pixel::SuperPixelBasedOrder(*iter,tmp_right)) {
		if (Pixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		  costheta =
		    map_iter->UnitSphereX()*iter->UnitSphereX() +
		    map_iter->UnitSphereY()*iter->UnitSphereY() +
		    map_iter->UnitSphereZ()*iter->UnitSphereZ();
		  if (theta_iter->WithinCosBounds(costheta)) {
		    pix_region =
		      Region(iter->SuperPix(RegionResolution()));
		    theta_iter->AddToPixelWtheta(map_iter->Intensity()*
						 map_iter->Weight()*
						 iter->Intensity()*
						 iter->Weight(),
						 map_iter->Weight()*
						 iter->Weight(),
						 map_region, pix_region);
		  }
		}
		++iter;
	      } else {
		iter = sub_map_[k].End();
	      }
	    }
	  }
	}
      }
    }
  }
  if (convert_back_to_raw) ConvertFromOverDensity();
}

void ScalarMap::CrossCorrelate(ScalarMap& scalar_map,
			       AngularCorrelation& wtheta) {
  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    bool convert_back_to_raw = false;
    if (!converted_to_overdensity_) {
      ConvertToOverDensity();
      convert_back_to_raw = true;
    }

    bool convert_input_map_back_to_raw = false;
    if (!scalar_map.IsOverDensityMap()) {
      scalar_map.ConvertToOverDensity();
      convert_input_map_back_to_raw = true;
    }

    for (ThetaIterator theta_iter=theta_begin;
	 theta_iter!=theta_end;++theta_iter)
      CrossCorrelate(scalar_map,theta_iter);

    if (convert_back_to_raw) ConvertFromOverDensity();
    if (convert_input_map_back_to_raw) scalar_map.ConvertFromOverDensity();
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }
}

void ScalarMap::CrossCorrelate(ScalarMap& scalar_map,
			       ThetaIterator theta_iter) {
  if (resolution_ != scalar_map.Resolution()) {
    std::cout << "Map resolutions must match!  Exiting...\n";
    exit(1);
  }

  bool convert_back_to_raw = false;
  if (!converted_to_overdensity_) {
    ConvertToOverDensity();
    convert_back_to_raw = true;
  }

  bool convert_input_map_back_to_raw = false;
  if (!scalar_map.IsOverDensityMap()) {
    scalar_map.ConvertToOverDensity();
    convert_input_map_back_to_raw = true;
  }

  theta_iter->ResetPixelWtheta();

  uint32_t y_min, y_max, x_min, x_max, k;
  double costheta = 0.0;
  double theta = theta_iter->ThetaMax();
  ScalarIterator iter;
  ScalarPixel tmp_left, tmp_right;
  tmp_left.SetResolution(resolution_);
  tmp_right.SetResolution(resolution_);

  for (ScalarIterator map_iter=scalar_map.Begin();
       map_iter!=scalar_map.End();++map_iter) {
    map_iter->XYBounds(theta, x_min, x_max, y_min, y_max, true);

    for (uint32_t y=y_min;y<=y_max;y++) {
      tmp_left.SetPixnumFromXY(x_min,y);
      tmp_right.SetPixnumFromXY(x_max,y);
      k = tmp_left.Superpixnum();

      if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

	// This is the same schema for iterating through the bounding
	// pixels as in FindLocalArea and FindLocalDensity.

	while (k != tmp_right.Superpixnum()) {
	  if (sub_map_[k].Initialized()) {
	    if (sub_map_[k].Begin()->PixelY() <= y) {
	      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
				 Pixel::SuperPixelBasedOrder);
	      while ((iter->PixelY() == y) &&
		     (iter != sub_map_[k].End())) {
		costheta =
		  map_iter->UnitSphereX()*iter->UnitSphereX() +
		  map_iter->UnitSphereY()*iter->UnitSphereY() +
		  map_iter->UnitSphereZ()*iter->UnitSphereZ();
		if (theta_iter->WithinCosBounds(costheta))
		  theta_iter->AddToPixelWtheta(map_iter->Intensity()*
					       map_iter->Weight()*
					       iter->Intensity()*
					       iter->Weight(),
					       map_iter->Weight()*
					       iter->Weight());
		++iter;
	      }
	    }
	    tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	    tmp_left.Iterate();
	    k = tmp_left.Superpixnum();
	  } else {
	    while (!sub_map_[k].Initialized() &&
		   k != tmp_right.Superpixnum()) {
	      tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	      tmp_left.Iterate();
	      k = tmp_left.Superpixnum();
	    }
	  }
	}

	if (sub_map_[k].Initialized()) {
	  if (Pixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
					       tmp_left) ||
	      Pixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(), tmp_left,
			       Pixel::SuperPixelBasedOrder);
	    while (iter != sub_map_[k].End()) {
	      if (Pixel::SuperPixelBasedOrder(*iter,tmp_right)) {
		costheta =
		  map_iter->UnitSphereX()*iter->UnitSphereX() +
		  map_iter->UnitSphereY()*iter->UnitSphereY() +
		  map_iter->UnitSphereZ()*iter->UnitSphereZ();
		if (theta_iter->WithinCosBounds(costheta))
		  theta_iter->AddToPixelWtheta(map_iter->Intensity()*
					       map_iter->Weight()*
					       iter->Intensity()*
					       iter->Weight(),
					       map_iter->Weight()*
					       iter->Weight());
		++iter;
	      } else {
		iter = sub_map_[k].End();
	      }
	    }
	  }
	}
      }
    }
  }

  if (convert_back_to_raw) ConvertFromOverDensity();
  if (convert_input_map_back_to_raw) scalar_map.ConvertFromOverDensity();
}

void ScalarMap::CrossCorrelateWithRegions(ScalarMap& scalar_map,
					   AngularCorrelation& wtheta) {
  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    bool convert_back_to_raw = false;
    if (!converted_to_overdensity_) {
      ConvertToOverDensity();
      convert_back_to_raw = true;
    }

    bool convert_input_map_back_to_raw = false;
    if (!scalar_map.IsOverDensityMap()) {
      scalar_map.ConvertToOverDensity();
      convert_input_map_back_to_raw = true;
    }

    for (ThetaIterator theta_iter=theta_begin;
	 theta_iter!=theta_end;++theta_iter)
      CrossCorrelateWithRegions(scalar_map,theta_iter);

    if (convert_back_to_raw) ConvertFromOverDensity();
    if (convert_input_map_back_to_raw) scalar_map.ConvertFromOverDensity();
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }
}

void ScalarMap::CrossCorrelateWithRegions(ScalarMap& scalar_map,
					   ThetaIterator theta_iter) {
  if (resolution_ != scalar_map.Resolution()) {
    std::cout << "Map resolutions must match!  Exiting...\n";
    exit(1);
  }

  bool convert_back_to_raw = false;
  if (!converted_to_overdensity_) {
    ConvertToOverDensity();
    convert_back_to_raw = true;
  }

  bool convert_input_map_back_to_raw = false;
  if (!scalar_map.IsOverDensityMap()) {
    scalar_map.ConvertToOverDensity();
    convert_input_map_back_to_raw = true;
  }

  if (theta_iter->NRegion() != NRegion()) {
    theta_iter->ClearRegions();
    theta_iter->InitializeRegions(NRegion());
  }

  theta_iter->ResetPixelWtheta();

  uint32_t y_min, y_max, x_min, x_max, k;
  uint32_t map_region, pix_region;
  double costheta = 0.0;
  double theta = theta_iter->ThetaMax();
  ScalarIterator iter;
  ScalarPixel tmp_left, tmp_right;
  tmp_left.SetResolution(resolution_);
  tmp_right.SetResolution(resolution_);

  for (ScalarIterator map_iter=scalar_map.Begin();
       map_iter!=scalar_map.End();++map_iter) {
    map_iter->XYBounds(theta, x_min, x_max, y_min, y_max, true);
    map_region = Region(map_iter->SuperPix(RegionResolution()));

    for (uint32_t y=y_min;y<=y_max;y++) {
      tmp_left.SetPixnumFromXY(x_min,y);
      tmp_right.SetPixnumFromXY(x_max,y);
      k = tmp_left.Superpixnum();

      if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

	// This is the same schema for iterating through the bounding
	// pixels as in FindLocalArea and FindLocalDensity.

	while (k != tmp_right.Superpixnum()) {
	  if (sub_map_[k].Initialized()) {
	    if (sub_map_[k].Begin()->PixelY() <= y) {
	      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
				 Pixel::SuperPixelBasedOrder);
	      while ((iter->PixelY() == y) &&
		     (iter != sub_map_[k].End())) {
		costheta =
		  map_iter->UnitSphereX()*iter->UnitSphereX() +
		  map_iter->UnitSphereY()*iter->UnitSphereY() +
		  map_iter->UnitSphereZ()*iter->UnitSphereZ();
		if (theta_iter->WithinCosBounds(costheta)) {
		  pix_region =
		    Region(iter->SuperPix(RegionResolution()));
		  theta_iter->AddToPixelWtheta(map_iter->Intensity()*
					       map_iter->Weight()*
					       iter->Intensity()*
					       iter->Weight(),
					       map_iter->Weight()*
					       iter->Weight(),
					       map_region, pix_region);
		}
		++iter;
	      }
	    }
	    tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	    tmp_left.Iterate();
	    k = tmp_left.Superpixnum();
	  } else {
	    while (!sub_map_[k].Initialized() &&
		   k != tmp_right.Superpixnum()) {
	      tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	      tmp_left.Iterate();
	      k = tmp_left.Superpixnum();
	    }
	  }
	}

	if (sub_map_[k].Initialized()) {
	  if (Pixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
					       tmp_left) ||
	      Pixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(), tmp_left,
			       Pixel::SuperPixelBasedOrder);
	    while (iter != sub_map_[k].End()) {
	      if (Pixel::SuperPixelBasedOrder(*iter,tmp_right)) {
		costheta =
		  map_iter->UnitSphereX()*iter->UnitSphereX() +
		  map_iter->UnitSphereY()*iter->UnitSphereY() +
		  map_iter->UnitSphereZ()*iter->UnitSphereZ();
		if (theta_iter->WithinCosBounds(costheta)) {
		  pix_region =
		    Region(iter->SuperPix(RegionResolution()));
		  theta_iter->AddToPixelWtheta(map_iter->Intensity()*
					       map_iter->Weight()*
					       iter->Intensity()*
					       iter->Weight(),
					       map_iter->Weight()*
					       iter->Weight(),
					       map_region, pix_region);
		}
		++iter;
	      } else {
		iter = sub_map_[k].End();
	      }
	    }
	  }
	}
      }
    }
  }

  if (convert_back_to_raw) ConvertFromOverDensity();
  if (convert_input_map_back_to_raw) scalar_map.ConvertFromOverDensity();
}

TreeMap::TreeMap(uint16_t input_resolution, uint16_t maximum_points) {
  resolution_ = input_resolution;
  maximum_points_ = maximum_points;
  weight_ = 0.0;
  point_count_ = 0;
  modified_ = false;
  area_ = 0.0;
  ClearRegions();
}

TreeMap::~TreeMap() {
  Clear();
  resolution_ = 0;
  maximum_points_ = 0;
  weight_ = 0.0;
  point_count_ = 0;
  modified_ = false;
  area_ = 0.0;
  ClearRegions();
}

bool TreeMap::AddPoint(WeightedAngularCoordinate* ang) {
  Pixel pix;
  pix.SetResolution(resolution_);
  pix.SetPixnumFromAng(*ang);

  TreeDictIterator iter = tree_map_.find(pix.Pixnum());
  if (iter == tree_map_.end()) {
    // If we didn't find the pixnum key in the map, then we need to add this
    // pixnum to the map and re-do the search.
    tree_map_.insert(std::pair<uint32_t,
		     TreePixel *>(pix.Pixnum(),
				  new TreePixel(pix.PixelX(), pix.PixelY(),
						resolution_, maximum_points_)));
    iter = tree_map_.find(pix.Pixnum());
    if (iter == tree_map_.end()) {
      std::cout << "Creating new TreeMap node failed. Exiting.\n";
      exit(2);
    }
  }
  bool added_point = (*iter).second->AddPoint(ang);
  if (added_point) {
    point_count_++;
    weight_ += ang->Weight();
    if (ang->HasFields()) {
      for (FieldIterator iter=ang->FieldBegin();iter!=ang->FieldEnd();++iter) {
	if (field_total_.find(iter->first) != field_total_.end()) {
	  field_total_[iter->first] += iter->second;
	} else {
	  field_total_[iter->first] = iter->second;
	}
      }
    }
  }

  modified_ = true;

  return added_point;
}

void TreeMap::Coverage(PixelVector& superpix, uint16_t resolution) {
  if (!superpix.empty()) superpix.clear();

  if (resolution > resolution_) {
    std::cout << "WARNING: Requested resolution is higher than " <<
      "the map resolution!\nReseting to map resolution...\n";
    resolution = resolution_;
  }

  // We need to make a vector of pixels that cover the current TreeMap area.
  // If the requested resolution is the same as our base node resolution, then
  // this is simple.
  if (resolution_ == resolution) {
    superpix.reserve(tree_map_.size());

    for (TreeDictIterator iter=tree_map_.begin();iter!=tree_map_.end();++iter) {
      Pixel pix(iter->second->PixelX(), iter->second->PixelY(),
		iter->second->Resolution(), iter->second->Coverage());
      superpix.push_back(pix);
    }
  } else {
    // If that's not the case, then we need to do some work.  First, the case
    // where the requested resolution is coarser than our base nodes.
    if (resolution < resolution_) {
      // We need to find the unique superpixels covered by the map and output
      // those.  We can use a temporary TreeDict to do this quickly.
      TreeDict tmp_map;
      for (TreeDictIterator iter=tree_map_.begin();
	   iter!=tree_map_.end();++iter) {
	TreePixel* pix =
	  new TreePixel(iter->second->PixelX(), iter->second->PixelY(),
			iter->second->Resolution(), 0);
	pix->SetToSuperPix(resolution);
	if (tmp_map.find(pix->Pixnum()) == tmp_map.end()) {
	  tmp_map[pix->Pixnum()] = pix;
	}
      }

      superpix.reserve(tmp_map.size());
      for (TreeDictIterator iter=tree_map_.begin();
	   iter!=tree_map_.end();++iter) {
	Pixel pix(iter->second->PixelX(), iter->second->PixelY(),
		  iter->second->Resolution(), 1.0);
	pix.SetWeight(FindUnmaskedFraction(pix));
	superpix.push_back(pix);
      }
    } else {
      // If the requested map is at higher resolution, then we iterate over
      // our map, finding the sub-pixels at the requested resolution.
      for (TreeDictIterator iter=tree_map_.begin();
	   iter!=tree_map_.end();++iter) {
	PixelVector sub_pix;
	iter->second->SubPix(resolution, sub_pix);

	for (PixelIterator sub_iter=sub_pix.begin();
	     sub_iter!=sub_pix.end();++sub_iter) {
	  // For each of the pixels in the superpixel, we check its status
	  // against the current map.  This is faster than finding the unmasked
	  // fraction directly and immediately tells us which pixels we can
	  // eliminate and which of those we do keep require further
	  // calculations to find the unmasked fraction.
	  int8_t unmasked_status = FindUnmaskedStatus(*sub_iter);
	  if (unmasked_status != 0) {
	    sub_iter->SetWeight(FindUnmaskedFraction(*sub_iter));
	    superpix.push_back(*sub_iter);
	  }
	}
      }
    }
  }

  // Sort them into the expected order before sending them back since we don't
  // know a priori what the order is coming out of the map object.
  sort(superpix.begin(), superpix.end(), Pixel::SuperPixelBasedOrder);
}

bool TreeMap::Covering(Map& stomp_map, uint32_t maximum_pixels) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  PixelVector pix;
  Coverage(pix);

  bool met_pixel_requirement;
  if (pix.size() > maximum_pixels) {
    // If the number of requested pixels is smaller than the number of
    // superpixels in the map, then we'd never be able to meet that requirement.
    // In this case, we set the output Map to the coarsest possible case
    // and return false.
    met_pixel_requirement = false;

    stomp_map.Initialize(pix);
  } else {
    // Ok, in this case, we can definitely produce a map that has at most
    // maximum_pixels in it.
    met_pixel_requirement = true;

    // To possibly save ourselves some effort, we start by making the Map
    // equivalent of our base level nodes.
    NodeMap(stomp_map);

    if (maximum_pixels < stomp_map.Size()) {
      // If our Map is still too large, we can use the Map's
      // Covering method to do our work for us.
      Map tmp_map;
      met_pixel_requirement = stomp_map.Covering(tmp_map, maximum_pixels);
      if (tmp_map.Size() < maximum_pixels) {
	stomp_map = tmp_map;
      }
    }
  }

  return met_pixel_requirement;
}

double TreeMap::FindUnmaskedFraction(Pixel& pix) {
  double unmasked_fraction = 0.0;

  if (pix.Resolution() >= resolution_) {
    // If our input pixel is the size of our base-node or smaller, then we
    // can use each node's Coverage method to find the unmasked fraction,
    // provideded that a matching node can be found.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end()) {
      unmasked_fraction = iter->second->Coverage(pix);
    }
  } else {
    // If that's not the case, then we need to find the subpixels of the input
    // pixel that match our base node resolution and iterate over them.
    double pixel_fraction =
      static_cast<double> (pix.Resolution()*pix.Resolution())/
      (resolution_*resolution_);

    PixelVector sub_pix;
    pix.SubPix(resolution_, sub_pix);

    for (PixelIterator sub_iter=sub_pix.begin();
	 sub_iter!=sub_pix.end();++sub_iter) {
      TreeDictIterator iter = tree_map_.find(sub_iter->Pixnum());
      if (iter != tree_map_.end()) {
	unmasked_fraction += pixel_fraction*iter->second->Coverage(pix);
      }
    }
  }

  return unmasked_fraction;
}

int8_t TreeMap::FindUnmaskedStatus(Pixel& pix) {
  int8_t unmasked_status = 0;

  // Since we don't have a strong notion of the exact geometry of our map,
  // the return values won't be as exact as they are in the Map or ScalarMap
  // classes.  The important thing, though, is returning a non-zero value if
  // we have reason to believe that there is data in the input pixel.
  if (pix.Resolution() >= resolution_) {
    // If our input pixel is the size of our base-node or smaller, then we
    // just need to find the containing node.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end()) {
      unmasked_status = 1;
    }
  } else {
    // If that's not the case, then we need to find the subpixels of the input
    // pixel that match our base node resolution and iterate over them.
    PixelVector sub_pix;
    pix.SubPix(resolution_, sub_pix);
    PixelIterator sub_iter=sub_pix.begin();

    while (sub_iter!=sub_pix.end() && unmasked_status == 0) {
      TreeDictIterator iter = tree_map_.find(sub_iter->Pixnum());
      if (iter != tree_map_.end()) unmasked_status = -1;

      ++sub_iter;
    }
  }

  return unmasked_status;
}

void TreeMap::NodeMap(Map& stomp_map) {
  if (!stomp_map.Empty()) stomp_map.Clear();

  PixelVector pix;
  pix.reserve(tree_map_.size());
  for (TreeDictIterator iter=tree_map_.begin();iter!=tree_map_.end();++iter) {
    Pixel tmp_pix(iter->second->PixelX(), iter->second->PixelY(),
		   iter->second->Resolution(), 1.0);
    pix.push_back(tmp_pix);
  }

  stomp_map.Initialize(pix);
}

uint16_t TreeMap::Nodes() {
  uint16_t total_nodes = 0;
  for (TreeDictIterator iter=tree_map_.begin();
       iter!=tree_map_.end();++iter) total_nodes += iter->second->Nodes();

  return total_nodes;
}

double TreeMap::Area() {
  if (modified_) CalculateArea();

  return area_;
}

void TreeMap::CalculateArea() {
  area_ = 0.0;
  for (TreeDictIterator iter=tree_map_.begin();
       iter!=tree_map_.end();++iter) {
    area_ += iter->second->Coverage()*iter->second->Area();
  }

  modified_ = false;
}

uint32_t TreeMap::NPoints(Pixel& pix) {
  uint32_t total_points = 0;

  // First we check to see if the input pixel is larger than our base-nodes.
  if (pix.Resolution() < resolution_) {
    // If this is the case, then we need to iterate over all of the sub-pixels
    // at the base-node resolution to see if there is any overlap and return
    // the sum.  We can just take the total since we know that the sub-pixel
    // covers the entire node.
    PixelVector pixVec;
    pix.SubPix(resolution_, pixVec);
    for (PixelIterator pix_iter=pixVec.begin();
	 pix_iter!=pixVec.end();++pix_iter) {
      TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
      if (iter != tree_map_.end())
	total_points += tree_map_[pix_iter->Pixnum()]->NPoints();
    }
  } else {
    // If the input pixel is the same size as our nodes or smaller, then we
    // look for the appropriate node and return the results from that node.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end())
      total_points = tree_map_[tmp_pix.Pixnum()]->NPoints(pix);
  }

  return total_points;
}

void TreeMap::Points(WAngularVector& w_ang) {
  if (!w_ang.empty()) w_ang.clear();

  // Fairly simple, just iterate over all of the base-nodes and return the
  // aggregate vector.  We can speed things up a bit since we know the total
  // number of points in the map.
  w_ang.reserve(point_count_);

  for (TreeDictIterator iter=tree_map_.begin();
       iter!=tree_map_.end();++iter) {
    WAngularVector tmp_ang;
    iter->second->Points(tmp_ang);
    for (WAngularIterator ang_iter=tmp_ang.begin();
	 ang_iter!=tmp_ang.end();++ang_iter) w_ang.push_back(*ang_iter);
  }
}

void TreeMap::Points(WAngularVector& w_ang, Pixel& pix) {
  if (!w_ang.empty()) w_ang.clear();

  // First we check to see if the input pixel is larger than our base-nodes.
  if (pix.Resolution() < resolution_) {
    // If this is the case, then we need to iterate over all of the sub-pixels
    // at the base-node resolution to see if there is any overlap and return
    // the aggregate.  We can just take all of the points in each node since
    // we know that the sub-pixel covers the entire node.
    PixelVector pixVec;
    pix.SubPix(resolution_, pixVec);
    for (PixelIterator pix_iter=pixVec.begin();
	 pix_iter!=pixVec.end();++pix_iter) {
      TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
      if (iter != tree_map_.end()) {
	WAngularVector tmp_ang;

	iter->second->Points(tmp_ang);
	for (WAngularIterator ang_iter=tmp_ang.begin();
	     ang_iter!=tmp_ang.end();++ang_iter) w_ang.push_back(*ang_iter);
      }
    }
  } else {
    // If the input pixel is the same size as our nodes or smaller, then we
    // look for the appropriate node and return the results from that node.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end()) {
      WAngularVector tmp_ang;

      iter->second->Points(tmp_ang, pix);
      for (WAngularIterator ang_iter=tmp_ang.begin();
	   ang_iter!=tmp_ang.end();++ang_iter) w_ang.push_back(*ang_iter);
    }
  }
}

double TreeMap::Weight(Pixel& pix) {
  double total_weight = 0;

  // First we check to see if the input pixel is larger than our base-nodes.
  if (pix.Resolution() < resolution_) {
    // If this is the case, then we need to iterate over all of the sub-pixels
    // at the base-node resolution to see if there is any overlap and return
    // the sum.  We can just take the total since we know that the sub-pixel
    // covers the entire node.
    PixelVector pixVec;
    pix.SubPix(resolution_, pixVec);
    for (PixelIterator pix_iter=pixVec.begin();
	 pix_iter!=pixVec.end();++pix_iter) {
      TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
      if (iter != tree_map_.end())
	total_weight += tree_map_[pix_iter->Pixnum()]->Weight();
    }
  } else {
    // If the input pixel is the same size as our nodes or smaller, then we
    // look for the appropriate node and return the results for that node.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end())
      total_weight = tree_map_[tmp_pix.Pixnum()]->PixelWeight(pix);
  }

  return total_weight;
}

double TreeMap::FieldTotal(const std::string& field_name, Pixel& pix) {
  double field_total = 0;

  // First we check to see if the input pixel is larger than our base-nodes.
  if (pix.Resolution() < resolution_) {
    // If this is the case, then we need to iterate over all of the sub-pixels
    // at the base-node resolution to see if there is any overlap and return
    // the sum.  We can just take the total since we know that the sub-pixel
    // covers the entire node.
    PixelVector pixVec;
    pix.SubPix(resolution_, pixVec);
    for (PixelIterator pix_iter=pixVec.begin();
	 pix_iter!=pixVec.end();++pix_iter) {
      TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
      if (iter != tree_map_.end())
	field_total += tree_map_[pix_iter->Pixnum()]->FieldTotal(field_name);
    }
  } else {
    // If the input pixel is the same size as our nodes or smaller, then we
    // look for the appropriate node and return the results for that node.
    Pixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution_);

    TreeDictIterator iter = tree_map_.find(tmp_pix.Pixnum());
    if (iter != tree_map_.end())
      field_total = tree_map_[tmp_pix.Pixnum()]->FieldTotal(field_name, pix);
  }

  return field_total;
}

uint32_t TreeMap::FindPairs(AngularCoordinate& ang, AngularBin& theta) {
  uint32_t pair_count = 0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(ang, theta.ThetaMax(), pix);

  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      pair_count += tree_map_[pix_iter->Pixnum()]->FindPairs(ang, theta);
  }
  return pair_count;
}

uint32_t TreeMap::FindPairs(AngularCoordinate& ang,
				 double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindPairs(ang, theta);
}

uint32_t TreeMap::FindPairs(AngularCoordinate& ang, double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindPairs(ang, theta);
}

void TreeMap::FindPairs(AngularVector& ang, AngularBin& theta) {
  uint32_t n_pairs = 0;

  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    n_pairs = FindPairs(*ang_iter, theta);
  }
}

void TreeMap::FindPairs(AngularVector& ang, AngularCorrelation& wtheta) {
  uint32_t n_pairs = 0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      n_pairs = FindPairs(*ang_iter, *theta_iter);
    }
  }
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta) {
  double total_weight = 0.0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(ang, theta.ThetaMax(), pix);

  // Now we iterate through the possibilities and add their contributions to
  // the total.
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      total_weight +=
	tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(ang, theta);
  }
  return total_weight;
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang,
				  double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(ang, theta);
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang, double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(ang, theta);
}

void TreeMap::FindWeightedPairs(AngularVector& ang, AngularBin& theta) {
  double total_weight = 0.0;

  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta);
  }
}

void TreeMap::FindWeightedPairs(AngularVector& ang,
				AngularCorrelation& wtheta) {
  double total_weight = 0.0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter);
    }
  }
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  AngularBin& theta) {
  double total_weight = 0.0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(w_ang, theta.ThetaMax(), pix);

  // Now we iterate through the possibilities and add their contributions to
  // the total.
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      total_weight +=
	tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(w_ang, theta);
  }
  return total_weight;
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  double theta_min, double theta_max) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, theta);
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  double theta_max) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, theta);
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta) {
  double total_weight = 0.0;

  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta);
  }
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang,
				AngularCorrelation& wtheta) {
  double total_weight = 0.0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter);
    }
  }
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
				  const std::string& field_name) {
  double total_weight = 0.0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(ang, theta.ThetaMax(), pix);

  // Now we iterate through the possibilities and add their contributions to
  // the total.
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      total_weight +=
	tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(ang, theta,
							 field_name);
  }
  return total_weight;
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang,
				  double theta_min, double theta_max,
				  const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(ang, theta, field_name);
}

double TreeMap::FindWeightedPairs(AngularCoordinate& ang, double theta_max,
				  const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(ang, theta, field_name);
}

void TreeMap::FindWeightedPairs(AngularVector& ang, AngularBin& theta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, field_name);
  }
}

void TreeMap::FindWeightedPairs(AngularVector& ang,
				AngularCorrelation& wtheta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (AngularIterator ang_iter=ang.begin();
	 ang_iter!=ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter, field_name);
    }
  }
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  AngularBin& theta,
				  const std::string& field_name) {
  double total_weight = 0.0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(w_ang, theta.ThetaMax(), pix);

  // Now we iterate through the possibilities and add their contributions to
  // the total.
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      total_weight +=
	tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(w_ang, theta,
							 field_name);
  }
  return total_weight;
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  double theta_min, double theta_max,
				  const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, theta, field_name);
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  double theta_max,
				  const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, theta, field_name);
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, theta, field_name);
  }
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang,
				AngularCorrelation& wtheta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, *theta_iter, field_name);
    }
  }
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  const std::string& ang_field_name,
				  AngularBin& theta,
				  const std::string& field_name) {
  double total_weight = 0.0;

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  center_pix.BoundingRadius(w_ang, theta.ThetaMax(), pix);

  // Now we iterate through the possibilities and add their contributions to
  // the total.
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end())
      total_weight +=
	tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(w_ang, ang_field_name,
							 theta, field_name);
  }
  return total_weight;
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  const std::string& ang_field_name,
				  double theta_min, double theta_max,
				  const std::string& field_name) {
  AngularBin theta(theta_min, theta_max);
  return FindWeightedPairs(w_ang, ang_field_name, theta, field_name);
}

double TreeMap::FindWeightedPairs(WeightedAngularCoordinate& w_ang,
				  const std::string& ang_field_name,
				  double theta_max,
				  const std::string& field_name) {
  AngularBin theta(0.0, theta_max);
  return FindWeightedPairs(w_ang, ang_field_name, theta, field_name);
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang,
				const std::string& ang_field_name,
				AngularBin& theta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    total_weight = FindWeightedPairs(*ang_iter, ang_field_name,
				     theta, field_name);
  }
}

void TreeMap::FindWeightedPairs(WAngularVector& w_ang,
				const std::string& ang_field_name,
				AngularCorrelation& wtheta,
				const std::string& field_name) {
  double total_weight = 0.0;

  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter) {
    for (WAngularIterator ang_iter=w_ang.begin();
	 ang_iter!=w_ang.end();++ang_iter) {
      total_weight = FindWeightedPairs(*ang_iter, ang_field_name,
				       *theta_iter, field_name);
    }
  }
}

void TreeMap::FindPairsWithRegions(AngularVector& ang, AngularBin& theta) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  uint32_t n_pair = 0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  n_pair = tree_map_[pix_iter->Pixnum()]->FindPairs(*ang_iter,
							    theta, region);
      }
    }
  }
}

void TreeMap::FindPairsWithRegions(AngularVector& ang,
				   AngularCorrelation& wtheta) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindPairsWithRegions(ang, *theta_iter);
}

void TreeMap::FindWeightedPairsWithRegions(AngularVector& ang,
					   AngularBin& theta) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  double total_weight = 0.0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  total_weight =
	    tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(*ang_iter,
							     theta, region);
      }
    }
  }
}

void TreeMap::FindWeightedPairsWithRegions(AngularVector& ang,
					   AngularCorrelation& wtheta) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindWeightedPairsWithRegions(ang, *theta_iter);
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   AngularBin& theta) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  total_weight =
	    tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(*ang_iter,
							     theta, region);
      }
    }
  }
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   AngularCorrelation& wtheta) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindWeightedPairsWithRegions(w_ang, *theta_iter);
}

void TreeMap::FindWeightedPairsWithRegions(AngularVector& ang,
					   AngularBin& theta,
					   const std::string& field_name) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  double total_weight = 0.0;
  for (AngularIterator ang_iter=ang.begin();ang_iter!=ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  total_weight =
	    tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(*ang_iter, theta,
							     field_name,region);
      }
    }
  }
}

void TreeMap::FindWeightedPairsWithRegions(AngularVector& ang,
					   AngularCorrelation& wtheta,
					   const std::string& field_name) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindWeightedPairsWithRegions(ang, *theta_iter, field_name);
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   AngularBin& theta,
					   const std::string& field_name) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  total_weight =
	    tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(*ang_iter, theta,
							     field_name,region);
      }
    }
  }
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   AngularCorrelation& wtheta,
					   const std::string& field_name) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindWeightedPairsWithRegions(w_ang, *theta_iter, field_name);
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   const std::string& ang_field_name,
					   AngularBin& theta,
					   const std::string& field_name) {
  if (!RegionsInitialized()) {
    std::cout <<
      "Must initialize regions before calling FindPairsWithRegions\n" <<
      "Exiting...\n";
    exit(2);
  }

  // First we need to find out which pixels this angular bin possibly touches.
  Pixel center_pix;
  center_pix.SetResolution(resolution_);
  PixelVector pix;
  double total_weight = 0.0;
  for (WAngularIterator ang_iter=w_ang.begin();
       ang_iter!=w_ang.end();++ang_iter) {
    center_pix.BoundingRadius(*ang_iter, theta.ThetaMax(), pix);
    uint16_t region = Region(center_pix.Pixnum());

    for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
      if (region == Region(pix_iter->Pixnum())) {
	TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
	if (iter != tree_map_.end())
	  total_weight =
	    tree_map_[pix_iter->Pixnum()]->FindWeightedPairs(*ang_iter,
							     ang_field_name,
							     theta, field_name,
							     region);
      }
    }
  }
}

void TreeMap::FindWeightedPairsWithRegions(WAngularVector& w_ang,
					   const std::string& ang_field_name,
					   AngularCorrelation& wtheta,
					   const std::string& field_name) {
  for (ThetaIterator theta_iter=wtheta.Begin(0);
       theta_iter!=wtheta.End(0);++theta_iter)
    FindWeightedPairsWithRegions(w_ang, ang_field_name,
				 *theta_iter, field_name);
}

uint16_t TreeMap::FindKNearestNeighbors(AngularCoordinate& ang,
					uint8_t n_neighbors,
					WAngularVector& neighbor_ang) {
  TreeNeighbor neighbors(ang, n_neighbors);

  _NeighborRecursion(ang, neighbors);

  neighbors.NearestNeighbors(neighbor_ang, false);

  return neighbors.NodesVisited();
}

uint16_t TreeMap::FindNearestNeighbor(AngularCoordinate& ang,
				    WeightedAngularCoordinate& neighbor_ang) {
  WAngularVector angVec;

  uint16_t nodes_visited = FindKNearestNeighbors(ang, 1, angVec);

  neighbor_ang = angVec[0];

  return nodes_visited;
}

double TreeMap::KNearestNeighborDistance(AngularCoordinate& ang,
					 uint8_t n_neighbors,
					 uint16_t& nodes_visited) {

  TreeNeighbor neighbors(ang, n_neighbors);

  _NeighborRecursion(ang, neighbors);

  nodes_visited = neighbors.NodesVisited();

  return neighbors.MaxAngularDistance();
}

void TreeMap::_NeighborRecursion(AngularCoordinate& ang,
				 TreeNeighbor& neighbors) {

  // First we need to find out if the input point is within our map area.
  Pixel center_pix(ang, resolution_);
  TreeDictIterator iter = tree_map_.find(center_pix.Pixnum());

  // If a node containing this point exists, then start finding neighbors there.
  if (iter != tree_map_.end())
    tree_map_[center_pix.Pixnum()]->_NeighborRecursion(ang, neighbors);

  // That should give us back a TreeNeighbor object that contains a workable
  // set of neighbors and a search radius for possible matches.  Now we just
  // need to iterate over those nodes that didn't contain the input point
  // to verify that there can't be any points in their sub-nodes which might
  // be closer to the input point.
  //
  // There's also the possibility that the input point is completely outside
  // our tree.  In that case (where the number of neighbors in the
  // TreeNeighbor object is less than the maximum), we want to check
  // all nodes.
  PixelVector pix;
  if (neighbors.Neighbors() == neighbors.MaxNeighbors()) {
    // We've got a starting list of neighbors, so we only have to look at
    // nodes within our current range.
    center_pix.BoundingRadius(ang, neighbors.MaxAngularDistance(), pix);
  } else {
    // The point is outside of the map area, so we have to check all of the
    // nodes.
    Coverage(pix, resolution_);
  }

  // Now we construct a priority queue so that we're search the nodes closest
  // to the input point first.
  PixelQueue pix_queue;
  for (PixelIterator pix_iter=pix.begin();pix_iter!=pix.end();++pix_iter) {
    TreeDictIterator iter = tree_map_.find(pix_iter->Pixnum());
    if (iter != tree_map_.end() && !pix_iter->Contains(ang)) {
      double min_edge_distance, max_edge_distance;
      tree_map_[pix_iter->Pixnum()]->_EdgeDistances(ang, min_edge_distance,
						    max_edge_distance);
      DistancePixelPair dist_pair(min_edge_distance,
				  tree_map_[pix_iter->Pixnum()]);
      pix_queue.push(dist_pair);
    }
  }

  // And iterate over that queue to check for neighbors.
  while (!pix_queue.empty()) {
    double pix_distance = pix_queue.top().first;
    TreePixel* pix_iter = pix_queue.top().second;
    if (pix_distance < neighbors.MaxDistance()) {
      pix_iter->_NeighborRecursion(ang, neighbors);
    }
    pix_queue.pop();
  }
}

FootprintBound::FootprintBound() {
  area_ = 0.0;
  pixel_area_ = 0.0;
  lammin_ = etamin_ = 200.0;
  lammax_ = etamax_ = -200.0;
  x_min_ = x_max_ = y_min_ = y_max_ = 0;
  max_resolution_level_ = Stomp::MaxPixelLevel;
  found_starting_resolution_ = false;
  found_xy_bounds_ = false;
}

FootprintBound::~FootprintBound() {
  if (!pix_.empty()) pix_.clear();

  area_ = 0.0;
  pixel_area_ = 0.0;
  lammin_ = etamin_ = 200.0;
  lammax_ = etamax_ = -200.0;
  x_min_ = x_max_ = y_min_ = y_max_ = 0;
  max_resolution_level_ = 0;
  found_starting_resolution_ = false;
  found_xy_bounds_ = false;
}

bool FootprintBound::CheckPoint(AngularCoordinate& ang) {
  return true;
}

bool FootprintBound::FindAngularBounds() {
  lammin_ = -90.0;
  lammax_ = 90.0;
  etamin_ = -180.0;
  etamax_ = 180.0;

  return true;
}

bool FootprintBound::FindArea() {
  return Stomp::HPixArea*Stomp::MaxSuperpixnum;
}

double FootprintBound::ScorePixel(Pixel& pix) {

  double inv_nx = 1.0/static_cast<double>(Stomp::Nx0*pix.Resolution());
  double inv_ny = 1.0/static_cast<double>(Stomp::Ny0*pix.Resolution());
  double x = static_cast<double>(pix.PixelX());
  double y = static_cast<double>(pix.PixelY());

  double lammid = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.5)*inv_ny);
  double lammin = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+1.0)*inv_ny);
  double lammax = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.0)*inv_ny);
  double lam_quart = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.75)*inv_ny);
  double lam_three = 90.0 - Stomp::RadToDeg*acos(1.0-2.0*(y+0.25)*inv_ny);

  double etamid = Stomp::RadToDeg*(2.0*Stomp::Pi*(x+0.5))*inv_nx +
      Stomp::EtaOffSet;
  if (etamid >= 180.0) etamid -= 360.0;
  if (etamid <= -180.0) etamid += 360.0;

  double etamin = Stomp::RadToDeg*(2.0*Stomp::Pi*(x+0.0))*inv_nx +
      Stomp::EtaOffSet;
  if (etamin >= 180.0) etamin -= 360.0;
  if (etamin <= -180.0) etamin += 360.0;

  double etamax = Stomp::RadToDeg*(2.0*Stomp::Pi*(x+1.0))*inv_nx +
      Stomp::EtaOffSet;
  if (etamax >= 180.0) etamax -= 360.0;
  if (etamax <= -180.0) etamax += 360.0;

  double eta_quart = Stomp::RadToDeg*(2.0*Stomp::Pi*(x+0.25))*inv_nx +
      Stomp::EtaOffSet;
  if (eta_quart >= 180.0) eta_quart -= 360.0;
  if (eta_quart <= -180.0) eta_quart += 360.0;

  double eta_three = Stomp::RadToDeg*(2.0*Stomp::Pi*(x+0.75))*inv_nx +
      Stomp::EtaOffSet;
  if (eta_three >= 180.0) eta_three -= 360.0;
  if (eta_three <= -180.0) eta_three += 360.0;

  double score = 0.0;

  AngularCoordinate ang(lammid,etamid,AngularCoordinate::Survey);
  if (CheckPoint(ang)) score -= 4.0;

  ang.SetSurveyCoordinates(lam_quart,etamid);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lam_three,etamid);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lammid,eta_quart);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lammid,eta_quart);
  if (CheckPoint(ang)) score -= 3.0;

  ang.SetSurveyCoordinates(lam_quart,eta_quart);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lam_three,eta_quart);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lam_quart,eta_three);
  if (CheckPoint(ang)) score -= 3.0;
  ang.SetSurveyCoordinates(lam_three,eta_three);
  if (CheckPoint(ang)) score -= 3.0;

  ang.SetSurveyCoordinates(lammid,etamax);
  if (CheckPoint(ang)) score -= 2.0;
  ang.SetSurveyCoordinates(lammid,etamin);
  if (CheckPoint(ang)) score -= 2.0;
  ang.SetSurveyCoordinates(lammax,etamid);
  if (CheckPoint(ang)) score -= 2.0;
  ang.SetSurveyCoordinates(lammin,etamid);
  if (CheckPoint(ang)) score -= 2.0;

  ang.SetSurveyCoordinates(lammax,etamax);
  if (CheckPoint(ang)) score -= 1.0;
  ang.SetSurveyCoordinates(lammax,etamin);
  if (CheckPoint(ang)) score -= 1.0;
  ang.SetSurveyCoordinates(lammin,etamax);
  if (CheckPoint(ang)) score -= 1.0;
  ang.SetSurveyCoordinates(lammin,etamin);
  if (CheckPoint(ang)) score -= 1.0;

  return score/40.0;
}

uint8_t FootprintBound::FindStartingResolutionLevel() {
  uint16_t max_resolution = static_cast<uint16_t>(1 << max_resolution_level_);

  double min_area = Pixel::PixelArea(max_resolution);

  if (area_ < 10.0*min_area) {
    return -1;
  }

  uint16_t starting_resolution = Stomp::HPixResolution;

  // We want to start things off with the coarsest possible resolution to
  // save time, but we have to be careful that we're not so coarse that we
  // miss parts of the footprint.  This finds the resolution that has pixels
  // about 1/100th the area of the footprint.
  while (area_/Pixel::PixelArea(starting_resolution) <= 100.0)
    starting_resolution *= 2;

  // We've checked against the possibility that our maximum resolution is too
  // coarse to resolve the area above.  If it happens that our starting
  // resolution is higher than our maximum resolution (i.e. the area is
  // between 10 and 100 times the area of our maximum resolution pixel), we
  // reset the starting resolution to the maximum resolution.
  if (starting_resolution > max_resolution)
    starting_resolution = max_resolution;

  if (starting_resolution % 2 == 0) found_starting_resolution_ = true;

  return Stomp::MostSignificantBit(starting_resolution);
}

bool FootprintBound::FindXYBounds(const uint8_t resolution_level) {
  uint16_t resolution = static_cast<uint16_t>(1 << resolution_level);
  uint32_t nx = Stomp::Nx0*resolution, ny = Stomp::Ny0*resolution;
  Pixel::AreaIndex(resolution,lammin_,lammax_,etamin_,etamax_,
		   x_min_, x_max_, y_min_, y_max_);

  // Checking top border
  bool found_pixel = true;
  bool boundary_failure = false;

  Pixel tmp_pix;
  tmp_pix.SetResolution(resolution);

  uint8_t n_iter = 0;
  uint8_t max_iter = 20;
  while (found_pixel && n_iter < max_iter) {
    found_pixel = false;
    uint32_t y = y_max_, nx_pix;

    if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
      nx_pix = nx - x_min_ + x_max_ + 1;
    } else {
      nx_pix = x_max_ - x_min_ + 1;
    }

    for (uint32_t m=0,x=x_min_;m<nx_pix;m++,x++) {
      if (x == nx) x = 0;

      tmp_pix.SetPixnumFromXY(x,y);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        m = nx_pix + 1;
      }
    }

    if (found_pixel) {
      // The exception to that case is if we've already reached the maximum
      // y index for the pixels.  In that case, we're just done.
      if (y_max_ < ny - 1) {
        y_max_++;
      } else {
        found_pixel = false;
      }
    }
    n_iter++;
  }
  if (n_iter == max_iter) boundary_failure = true;

  // Checking bottom border
  found_pixel = true;
  n_iter = 0;
  while (!boundary_failure && found_pixel && n_iter < max_iter) {
    found_pixel = false;
    uint32_t y = y_min_, nx_pix;

    if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
      nx_pix = nx - x_min_ + x_max_ + 1;
    } else {
      nx_pix = x_max_ - x_min_ + 1;
    }

    for (uint32_t m=0,x=x_min_;m<nx_pix;m++,x++) {
      if (x == nx) x = 0;

      tmp_pix.SetPixnumFromXY(x,y);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        m = nx_pix + 1;
      }
    }

    if (found_pixel) {
      // The exception to that case is if we've already reached the minimum
      // y index for the pixels.  In that case, we're just done.
      if (y_min_ > 0) {
        y_min_--;
      } else {
        found_pixel = false;
      }
    }
    n_iter++;
  }

  if (n_iter == max_iter) boundary_failure = true;

  // Checking left border
  found_pixel = true;
  n_iter = 0;
  while (!boundary_failure && found_pixel && n_iter < max_iter) {
    found_pixel = false;
    uint32_t x = x_min_;

    for (uint32_t y=y_min_;y<=y_max_;y++) {

      tmp_pix.SetPixnumFromXY(x,y);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        y = y_max_ + 1;
      }
    }

    if (found_pixel) {
      if (x_min_ == 0) {
        x_min_ = nx - 1;
      } else {
        x_min_--;
      }
    }
    n_iter++;
  }
  if (n_iter == max_iter) boundary_failure = true;

  // Checking right border
  found_pixel = true;
  n_iter = 0;
  while (!boundary_failure && found_pixel && n_iter < max_iter) {
    found_pixel = false;
    uint32_t x = x_max_;

    for (uint32_t y=y_min_;y<=y_max_;y++) {

      tmp_pix.SetPixnumFromXY(x,y);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        y = y_max_ + 1;
      }
    }

    if (found_pixel) {
      if (x_max_ == nx - 1) {
        x_max_ = 0;
      } else {
        x_max_++;
      }
    }
    n_iter++;
  }

  if (n_iter == max_iter) boundary_failure = true;

  return !boundary_failure;
}

bool FootprintBound::Pixelize() {

  if (!pix_.empty()) pix_.clear();
  pixel_area_ = 0.0;

  uint8_t starting_resolution_level = FindStartingResolutionLevel();

  if ((starting_resolution_level < Stomp::HPixLevel) ||
      (starting_resolution_level > max_resolution_level_)) {
    return false;
  }

  // We need to be careful around the poles since the pixels there get
  // very distorted.
  if ((lammin_ > 85.0) || (lammax_ < -85.0))
    starting_resolution_level = Stomp::MostSignificantBit(512);

  if (FindXYBounds(starting_resolution_level)) {

    PixelVector resolve_pix, previous_pix;

    for (uint8_t resolution_level=starting_resolution_level;
         resolution_level<=max_resolution_level_;resolution_level++) {
      uint16_t resolution = static_cast<uint16_t>(1 << resolution_level);

      unsigned n_keep = 0;
      uint32_t nx = Stomp::Nx0*resolution;
      Pixel tmp_pix;
      tmp_pix.SetResolution(resolution);

      double score;
      AngularCoordinate ang;

      if (resolution_level == starting_resolution_level) {
        resolve_pix.clear();
        previous_pix.clear();

        uint32_t nx_pix;
        if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
          nx_pix = nx - x_min_ + x_max_ + 1;
        } else {
          nx_pix = x_max_ - x_min_ + 1;
        }

        for (uint32_t y=y_min_;y<=y_max_;y++) {
          for (uint32_t m=0,x=x_min_;m<nx_pix;m++,x++) {
            if (x==nx) x = 0;
            tmp_pix.SetPixnumFromXY(x,y);

            score = ScorePixel(tmp_pix);

            if (score < -0.99999) {
              tmp_pix.SetWeight(weight_);
              AddToPixelizedArea(resolution);
              pix_.push_back(tmp_pix);
              n_keep++;
            } else {
              if (score < -0.00001) {
                tmp_pix.SetWeight(score);
                resolve_pix.push_back(tmp_pix);
              }
            }
            previous_pix.push_back(tmp_pix);
          }
        }
      } else {
        if (resolve_pix.size() == 0) {
          std::cout << "Missed all pixels in initial search; trying again...\n";
          for (PixelIterator iter=previous_pix.begin();
               iter!=previous_pix.end();++iter) {
            PixelVector sub_pix;
            iter->SubPix(resolution,sub_pix);
            for (PixelIterator sub_iter=sub_pix.begin();
                 sub_iter!=sub_pix.end();++sub_iter)
              resolve_pix.push_back(*sub_iter);
          }
        }

        previous_pix.clear();

	previous_pix.reserve(resolve_pix.size());
        for (PixelIterator iter=resolve_pix.begin();
             iter!=resolve_pix.end();++iter) previous_pix.push_back(*iter);

        resolve_pix.clear();

        uint32_t x_min, x_max, y_min, y_max;

        for (PixelIterator iter=previous_pix.begin();
             iter!=previous_pix.end();++iter) {

          iter->SubPix(resolution,x_min,x_max,y_min,y_max);

          for (uint32_t y=y_min;y<=y_max;y++) {
            for (uint32_t x=x_min;x<=x_max;x++) {
              tmp_pix.SetPixnumFromXY(x,y);

              score = ScorePixel(tmp_pix);

              if (score < -0.99999) {
                tmp_pix.SetWeight(weight_);
                AddToPixelizedArea(resolution);
                pix_.push_back(tmp_pix);
                n_keep++;
              } else {
                if (score < -0.00001) {
                  tmp_pix.SetWeight(score);
                  resolve_pix.push_back(tmp_pix);
                }
              }
            }
          }
        }
      }
    }

    previous_pix.clear();

    uint16_t max_resolution =
      static_cast<uint16_t>(1 << max_resolution_level_);
    if (area_ > pixel_area_) {
      sort(resolve_pix.begin(),resolve_pix.end(),Pixel::WeightedOrder);

      uint32_t n=0;
      double ur_weight = resolve_pix[n].Weight();
      while ((n < resolve_pix.size()) &&
             ((area_ > pixel_area_) ||
              ((resolve_pix[n].Weight() < ur_weight + 0.1) &&
               (resolve_pix[n].Weight() > ur_weight - 0.1)))) {
        ur_weight = resolve_pix[n].Weight();
        resolve_pix[n].SetWeight(weight_);
        AddToPixelizedArea(max_resolution);
        pix_.push_back(resolve_pix[n]);
        n++;
      }
    }

    Pixel::ResolvePixel(pix_);

    return true;
  } else {
    return false;
  }
}

CircleBound::CircleBound(const AngularCoordinate& ang,
                         double radius, double weight) {
  SetWeight(weight);

  ang_ = ang;
  radius_ = radius;
  sin2radius_ = sin(radius*Stomp::DegToRad)*sin(radius*Stomp::DegToRad);

  FindArea();
  FindAngularBounds();
  SetMaxResolution();
}

CircleBound::~CircleBound() {
  Clear();
  radius_ = sin2radius_ = 0.0;
}

bool CircleBound::FindAngularBounds() {

  double lammin = ang_.Lambda() - radius_;
  if (Stomp::DoubleLE(lammin, -90.0)) lammin = -90.0;

  double lammax = ang_.Lambda() + radius_;
  if (Stomp::DoubleGE(lammax, 90.0)) lammax = 90.0;

  // double eta_multiplier =
  // AngularCoordinate::EtaMultiplier(0.5*(lammax+lammin));
  double eta_multiplier = 1.0;

  double etamin = ang_.Eta() - radius_*eta_multiplier;
  if (Stomp::DoubleGT(etamin, 180.0)) etamin -= 360.0;
  if (Stomp::DoubleLT(etamin, -180.0)) etamin += 360.0;

  double etamax = ang_.Eta() + radius_*eta_multiplier;
  if (Stomp::DoubleGT(etamax, 180.0)) etamax -= 360.0;
  if (Stomp::DoubleLT(etamax, -180.0)) etamax += 360.0;

  SetAngularBounds(lammin,lammax,etamin,etamax);

  return true;
}

bool CircleBound::FindArea() {
  SetArea((1.0 -
           cos(radius_*Stomp::DegToRad))*2.0*Stomp::Pi*Stomp::StradToDeg);
  return true;
}

bool CircleBound::CheckPoint(AngularCoordinate& ang) {

  double costheta =
      ang.UnitSphereX()*ang_.UnitSphereX() +
      ang.UnitSphereY()*ang_.UnitSphereY() +
      ang.UnitSphereZ()*ang_.UnitSphereZ();

  if (1.0-costheta*costheta <= sin2radius_ + 1.0e-10) return true;

  return false;
}

WedgeBound::WedgeBound(const AngularCoordinate& ang, double radius,
		       double position_angle_min, double position_angle_max,
		       double weight, AngularCoordinate::Sphere sphere) {
  SetWeight(weight);

  ang_ = ang;
  radius_ = radius;
  sin2radius_ = sin(radius*Stomp::DegToRad)*sin(radius*Stomp::DegToRad);
  if (Stomp::DoubleLT(position_angle_min, position_angle_max)) {
    position_angle_max_ = position_angle_max;
    position_angle_min_ = position_angle_min;
  } else {
    position_angle_max_ = position_angle_min;
    position_angle_min_ = position_angle_max;
  }
  sphere_ = sphere;

  FindArea();
  FindAngularBounds();
  SetMaxResolution();
}

WedgeBound::~WedgeBound() {
  Clear();
  radius_ = sin2radius_ = position_angle_min_ = position_angle_max_ = 0.0;
}

bool WedgeBound::FindAngularBounds() {
  // We should be able to define our bounds based on the three points that
  // define the wedge.
  double lammin = ang_.Lambda();
  double lammax = ang_.Lambda();
  double etamin = ang_.Eta();
  double etamax = ang_.Eta();

  // Now we define a new point directly north of the center of the circle.
  AngularCoordinate start_ang;
  switch(sphere_) {
  case AngularCoordinate::Survey:
    start_ang.SetSurveyCoordinates(ang_.Lambda()+radius_, ang_.Eta());
    break;
  case AngularCoordinate::Equatorial:
    start_ang.SetEquatorialCoordinates(ang_.RA(), ang_.DEC()+radius_);
    break;
  case AngularCoordinate::Galactic:
    start_ang.SetGalacticCoordinates(ang_.GalLon(), ang_.GalLat()+radius_);
    break;
  }

  // Using that point as a reference, we can rotate to our minimum and
  // maximum position angles.
  AngularCoordinate new_ang;
  start_ang.Rotate(ang_, position_angle_min_, new_ang);
  
  if (Stomp::DoubleLE(new_ang.Lambda(), lammin)) lammin = new_ang.Lambda();
  if (Stomp::DoubleGE(new_ang.Lambda(), lammax)) lammax = new_ang.Lambda();

  if (Stomp::DoubleLE(new_ang.Eta(), etamin)) etamin = new_ang.Eta();
  if (Stomp::DoubleGE(new_ang.Eta(), etamax)) etamax = new_ang.Eta();

  start_ang.Rotate(ang_, position_angle_max_, new_ang);

  if (Stomp::DoubleLE(new_ang.Lambda(), lammin)) lammin = new_ang.Lambda();
  if (Stomp::DoubleGE(new_ang.Lambda(), lammax)) lammax = new_ang.Lambda();

  if (Stomp::DoubleLE(new_ang.Eta(), etamin)) etamin = new_ang.Eta();
  if (Stomp::DoubleGE(new_ang.Eta(), etamax)) etamax = new_ang.Eta();

  SetAngularBounds(lammin,lammax,etamin,etamax);

  return true;
}

bool WedgeBound::FindArea() {
  double circle_area =
    (1.0 - cos(radius_*Stomp::DegToRad))*2.0*Stomp::Pi*Stomp::StradToDeg;
  SetArea(circle_area*(position_angle_max_ - position_angle_min_)/360.0);
  return true;
}

bool WedgeBound::CheckPoint(AngularCoordinate& ang) {

  double costheta =
    ang.UnitSphereX()*ang_.UnitSphereX() +
    ang.UnitSphereY()*ang_.UnitSphereY() +
    ang.UnitSphereZ()*ang_.UnitSphereZ();

  if (1.0-costheta*costheta <= sin2radius_ + 1.0e-10) {
    double position_angle = ang_.PositionAngle(ang);
    if (Stomp::DoubleGE(position_angle, position_angle_min_) &&
	Stomp::DoubleLE(position_angle, position_angle_max_))
      return true;
  }
  return false;
}

PolygonBound::PolygonBound(AngularVector& ang, double weight) {

  SetWeight(weight);

  for (AngularIterator iter=ang.begin();iter!=ang.end();++iter)
    ang_.push_back(*iter);

  n_vert_ = ang_.size();

  x_.reserve(n_vert_);
  y_.reserve(n_vert_);
  z_.reserve(n_vert_);
  dot_.reserve(n_vert_);

  for (uint32_t i=0;i<n_vert_;i++) {

    std::vector<double> tmp_x, tmp_y, tmp_z;

    for (uint32_t j=0;j<n_vert_;j++) {
      tmp_x.push_back(ang_[j].UnitSphereX());
      tmp_y.push_back(ang_[j].UnitSphereY());
      tmp_z.push_back(ang_[j].UnitSphereZ());
    }

    for (uint32_t j=0;j<n_vert_;j++) {
      if (j == n_vert_ - 1) {
        x_.push_back(tmp_y[j]*tmp_z[0] - tmp_y[0]*tmp_z[j]);
        y_.push_back(tmp_z[j]*tmp_x[0] - tmp_z[0]*tmp_x[j]);
        z_.push_back(tmp_x[j]*tmp_y[0] - tmp_x[0]*tmp_y[j]);
      } else {
        x_.push_back(tmp_y[j]*tmp_z[j+1] - tmp_y[j+1]*tmp_z[j]);
        y_.push_back(tmp_z[j]*tmp_x[j+1] - tmp_z[j+1]*tmp_x[j]);
        z_.push_back(tmp_x[j]*tmp_y[j+1] - tmp_x[j+1]*tmp_y[j]);
      }

      double amplitude = sqrt(x_[j]*x_[j] + y_[j]*y_[j] + z_[j]*z_[j]);

      x_[j] /= amplitude;
      y_[j] /= amplitude;
      z_[j] /= amplitude;

      dot_.push_back(1.0); // This assumes that we're not at constant DEC.
    }
  }

  FindArea();
  FindAngularBounds();
  SetMaxResolution();
}

PolygonBound::~PolygonBound() {
  ang_.clear();
  x_.clear();
  y_.clear();
  z_.clear();
  dot_.clear();
  n_vert_ = 0;
}

bool PolygonBound::FindAngularBounds() {

  double lammin = 100.0, lammax = -100.0, etamin = 200.0, etamax = -200.0;

  for (uint32_t i=0;i<n_vert_;i++) {
    if (ang_[i].Lambda() < lammin) lammin = ang_[i].Lambda();
    if (ang_[i].Lambda() > lammax) lammax = ang_[i].Lambda();
    if (ang_[i].Eta() < etamin) etamin = ang_[i].Eta();
    if (ang_[i].Eta() > etamax) etamax = ang_[i].Eta();
  }

  SetAngularBounds(lammin,lammax,etamin,etamax);

  return true;
}

bool PolygonBound::FindArea() {

  double sum = 0.0;

  for (uint32_t j=0,k=1;j<n_vert_;j++,k++) {
    if (k == n_vert_) k = 0;

    double cm = (-x_[j]*x_[k] - y_[j]*y_[k] - z_[j]*z_[k]);

    sum += acos(cm);
  }

  double tmp_area = (sum - (n_vert_ - 2)*Stomp::Pi)*Stomp::StradToDeg;

  if (tmp_area > 4.0*Stomp::Pi*Stomp::StradToDeg) {
    std::cout << "Polygon area is over half the sphere.  This is bad.\n";
    return false;
  }

  SetArea(tmp_area);

  return true;
}

bool PolygonBound::CheckPoint(AngularCoordinate& ang) {
  bool in_polygon = true;

  uint32_t n=0;
  while ((n < n_vert_) && (in_polygon)) {

    in_polygon = false;
    double dot = 1.0 - x_[n]*ang.UnitSphereX() -
        y_[n]*ang.UnitSphereY() - z_[n]*ang.UnitSphereZ();
    if (DoubleLE(dot_[n],0.0)) {
      if (DoubleLE(fabs(dot_[n]), dot)) in_polygon = true;
    } else {
      if (DoubleGE(dot_[n], dot)) in_polygon = true;
    }
    n++;
  }

  return in_polygon;
}

} // end namespace Stomp
