#include "stomp_util.h"

StompPixel::StompPixel() {
  resolution_ = -1;
  y_ = 0;
  x_ = 0;
  weight_ = 0.0;
}

StompPixel::StompPixel(const int input_resolution,
                       const unsigned long input_pixnum,
                       const double input_weight) {

  if ((input_resolution < Stomp::HPixResolution()) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution()))
    std::cout << "Invalid resolution value.\n ";

  if (input_pixnum > Stomp::MaxPixnum())
    std::cout << "Invalid pixel index value.\n ";

  resolution_ = input_resolution;
  y_ = input_pixnum/(Stomp::Nx0()*resolution_);
  x_ = input_pixnum - Stomp::Nx0()*resolution_*y_;
  weight_ = input_weight;
}

StompPixel::StompPixel(const int input_resolution,
                       const unsigned long input_hpixnum,
                       const unsigned long input_superpixnum,
                       const double input_weight) {

  if ((input_resolution < Stomp::HPixResolution()) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution()))
    std::cout << "Invalid resolution value.\n ";

  if (input_hpixnum > Stomp::MaxPixnum())
    std::cout << "Invalid hpixel index value.\n ";

  if (input_superpixnum > Stomp::MaxSuperpixnum())
    std::cout << "Invalid superpixel index value.\n ";

  resolution_ = input_resolution;

  int hnx = resolution_/Stomp::HPixResolution();

  unsigned long y0 = input_superpixnum/(Stomp::Nx0()*Stomp::HPixResolution());
  unsigned long x0 =
      input_superpixnum - y0*Stomp::Nx0()*Stomp::HPixResolution();

  y0 *= hnx;
  x0 *= hnx;

  unsigned long tmp_y = input_hpixnum/hnx;
  unsigned long tmp_x = input_hpixnum - hnx*tmp_y;

  x_ = tmp_x + x0;
  y_ = tmp_y + y0;
  weight_ = input_weight;
}

StompPixel:: StompPixel(const unsigned long input_x,
                        const unsigned long input_y,
                        const int input_resolution,
                        const double input_weight) {

  if ((input_resolution < Stomp::HPixResolution()) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution()))
    std::cout << "Invalid resolution value.\n ";

  if (input_x > Stomp::Nx0()*input_resolution)
    std::cout << "Invalid x index value.\n ";

  if (input_y > Stomp::Ny0()*input_resolution)
    std::cout << "Invalid x index value.\n ";

  resolution_ = input_resolution;
  x_ = input_x;
  y_ = input_y;
  weight_ = input_weight;
}

StompPixel::StompPixel(AngularCoordinate& ang, const int input_resolution,
                       const double input_weight) {

  if ((input_resolution < Stomp::HPixResolution()) ||
      (input_resolution%2 != 0) ||
      (input_resolution > Stomp::MaxPixelResolution())) {
    std::cout << "Invalid resolution value.\n ";
  }

  resolution_ = input_resolution;

  double eta = (ang.Eta() - Stomp::EtaOffSet())*Stomp::Deg2Rad();

  if (eta <= 0.0) eta += 2.0*Stomp::Pi();

  eta /= 2.0*Stomp::Pi();

  x_ = static_cast<unsigned long>(Stomp::Nx0()*resolution_*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::Deg2Rad();

  if (lambda >= Stomp::Pi()) {
    y_ = Stomp::Ny0()*resolution_ - 1;
  } else {
    y_ = static_cast<unsigned long>(Stomp::Ny0()*resolution_*
                                    ((1.0 - cos(lambda))/2.0));
  }

  weight_ = input_weight;
}

StompPixel::~StompPixel() {
  x_ = y_ = 0;
  resolution_ = -1;
  weight_ = 0.0;
}

void StompPixel::Ang2Pix(int input_resolution, AngularCoordinate& ang,
                         unsigned long& output_pixnum) {
  double lambda = ang.Lambda();
  double eta = ang.Eta();
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  eta -= Stomp::EtaOffSet();

  eta *= Stomp::Deg2Rad();

  if (eta <= 0.0) eta += 2.0*Stomp::Pi();

  eta /= 2.0*Stomp::Pi();
  unsigned long i = static_cast<unsigned long>(nx*eta);

  lambda = (90.0 - lambda)*Stomp::Deg2Rad();

  unsigned long j;
  if (lambda >= Stomp::Pi()) {
    j = ny - 1;
  } else {
    j = static_cast<unsigned long>(ny*((1.0 - cos(lambda))/2.0));
  }

  output_pixnum = nx*j + i;
}

void StompPixel::SetPixnumFromAng(AngularCoordinate& ang) {

  double eta = (ang.Eta() - Stomp::EtaOffSet())*Stomp::Deg2Rad();

  if (eta <= 0.0) eta += 2.0*Stomp::Pi();

  eta /= 2.0*Stomp::Pi();
  x_ = static_cast<unsigned long>(Stomp::Nx0()*resolution_*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::Deg2Rad();

  if (lambda >= Stomp::Pi()) {
    y_ = Stomp::Ny0()*resolution_ - 1;
  } else {
    y_ = static_cast<unsigned long>(Stomp::Ny0()*resolution_*
                                    ((1.0 - cos(lambda))/2.0));
  }
}

void StompPixel::Pix2Ang(int input_resolution, unsigned long input_pixnum,
                         AngularCoordinate& ang) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  unsigned long y = input_pixnum/nx;
  unsigned long x = input_pixnum - nx*y;

  ang.SetSurveyCoordinates(90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.5)/ny),
			   Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+0.5))/nx +
			   Stomp::EtaOffSet());
}

void StompPixel::Pix2HPix(int input_resolution,
                          unsigned long input_pixnum,
                          unsigned long& output_hpixnum,
                          unsigned long& output_superpixnum) {
  unsigned long nx = Stomp::Nx0()*input_resolution;

  unsigned long y = input_pixnum/nx;
  unsigned long x = input_pixnum - nx*y;

  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long x0 = x/hnx;
  unsigned long y0 = y/hnx;

  x -= x0*hnx;
  y -= y0*hnx;

  output_hpixnum = hnx*y + x;
  output_superpixnum = Stomp::Nx0()*Stomp::HPixResolution()*y0 + x0;
}

void StompPixel::SuperPix(int hi_resolution,unsigned long hi_pixnum,
                          int lo_resolution, unsigned long& lo_pixnum) {
  if (hi_resolution < lo_resolution) {
    std::cout << "Can't go from low resolution to higher resolution.\n ";
    exit(1);
  } else {
    unsigned long nx_hi = Stomp::Nx0()*hi_resolution;
    unsigned long nx_lo = Stomp::Nx0()*lo_resolution;

    int ratio = hi_resolution/lo_resolution;

    unsigned long j = hi_pixnum/nx_hi;
    unsigned long i = hi_pixnum - nx_hi*j;

    i /= ratio;
    j /= ratio;

    lo_pixnum = nx_lo*j + i;
  }
}

bool StompPixel::SetToSuperPix(int lo_resolution) {
  if (resolution_ < lo_resolution) {
    std::cout << "Illegal resolution value: " << lo_resolution <<
        " < " << resolution_;
    return false;
  }

  x_ /= resolution_/lo_resolution;
  y_ /= resolution_/lo_resolution;

  resolution_ = lo_resolution;

  return true;
}

void StompPixel::NextSubPix(int input_resolution, unsigned long input_pixnum,
                            unsigned long& sub_pixnum1,
                            unsigned long& sub_pixnum2,
                            unsigned long& sub_pixnum3,
                            unsigned long& sub_pixnum4) {
  unsigned long nx_hi = 2*Stomp::Nx0()*input_resolution;
  unsigned long nx_lo = Stomp::Nx0()*input_resolution;

  unsigned long j = input_pixnum/nx_lo;
  unsigned long i = input_pixnum - nx_lo*j;

  sub_pixnum1 = nx_hi*(2*j) + 2*i;
  sub_pixnum2 = nx_hi*(2*j) + 2*i + 1;
  sub_pixnum3 = nx_hi*(2*j + 1) + 2*i;
  sub_pixnum4 = nx_hi*(2*j + 1) + 2*i + 1;
}

void StompPixel::SubPix(int lo_resolution, unsigned long lo_pixnum,
                        int hi_resolution, unsigned long& x_min,
                        unsigned long& x_max, unsigned long& y_min,
                        unsigned long& y_max) {

  unsigned long nx_hi = Stomp::Nx0()*hi_resolution;

  if (lo_resolution == hi_resolution) {
    y_min = lo_pixnum/nx_hi;
    y_max = lo_pixnum/nx_hi;
    x_min = lo_pixnum - nx_hi*y_min;
    x_max = lo_pixnum - nx_hi*y_max;
  } else {
    unsigned long tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4;
    int tmp_res;

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

void StompPixel::SubPix(int hi_resolution, StompVector& pix) {

  if (pix.empty() == false) pix.clear();

  unsigned long x_min, x_max, y_min, y_max;

  if (resolution_ == hi_resolution) {
    y_min = y_max = y_;
    x_min = x_max = x_;
  } else {
    int tmp_res;

    x_min = x_max = x_;
    y_min = y_max = y_;
    for (tmp_res=resolution_;tmp_res<hi_resolution;tmp_res*=2) {
      x_min *= 2;
      y_min *= 2;

      x_max = 2*x_max + 1;
      y_max = 2*y_max + 1;
    }
  }

  for (unsigned long y=y_min;y<=y_max;y++) {
    for (unsigned long x=x_min;x<=x_max;x++) {
      StompPixel tmp_pix(x,y,hi_resolution);
      pix.push_back(tmp_pix);
    }
  }
}

void StompPixel::SubPix(int hi_resolution, unsigned long& x_min,
			unsigned long& x_max, unsigned long& y_min,
			unsigned long& y_max) {

  if (resolution_ == hi_resolution) {
    y_min = y_max = y_;
    x_min = x_max = x_;
  } else {
    int tmp_res;

    x_min = x_max = x_;
    y_min = y_max = y_;
    for (tmp_res=resolution_;tmp_res<hi_resolution;tmp_res*=2) {
      x_min *= 2;
      y_min *= 2;

      x_max = 2*x_max + 1;
      y_max = 2*y_max + 1;
    }
  }
}

void StompPixel::PixelBound(int input_resolution, unsigned long input_pixnum,
                            double& lammin, double& lammax, double& etamin,
                            double& etamax) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  unsigned long y = input_pixnum/nx;
  unsigned long x = input_pixnum - nx*y;

  lammin = 90.0 - Stomp::Rad2Deg()*acos(1.0 - 2.0*(y+1)/ny);
  lammax = 90.0 - Stomp::Rad2Deg()*acos(1.0 - 2.0*y/ny);
  etamin = Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x+0.0)/nx + Stomp::EtaOffSet();
  if (etamin >= 180.0) etamin = etamin - 360.0;
  etamax = Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x+1.0)/nx + Stomp::EtaOffSet();
  if (etamax >= 180.0) etamax = etamax - 360.0;
}

void StompPixel::CohortPix(int input_resolution, unsigned long input_pixnum,
                           unsigned long& co_pixnum1,
                           unsigned long& co_pixnum2,
                           unsigned long& co_pixnum3) {
  unsigned long tmp_pixnum, pixnum1, pixnum2, pixnum3, pixnum4;

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

void StompPixel::CohortPix(StompPixel& pix_a,
			   StompPixel& pix_b,
			   StompPixel& pix_c) {
  unsigned long super_x, super_y, x1, x2, x3, x4, y1, y2, y3, y4;

  pix_a.SetResolution(resolution_);
  pix_b.SetResolution(resolution_);
  pix_c.SetResolution(resolution_);

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

void StompPixel::XYBounds(double theta, unsigned long& x_min,
			  unsigned long& x_max, unsigned long& y_min,
			  unsigned long& y_max, bool add_buffer) {
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

  unsigned long nx = Stomp::Nx0()*resolution_;
  unsigned long ny = Stomp::Ny0()*resolution_;

  double etamin = Eta() - theta*sphere_correction;
  etamin -= Stomp::EtaOffSet();
  etamin *= Stomp::Deg2Rad();

  if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi();

  etamin /= 2.0*Stomp::Pi();
  x_min = static_cast<unsigned long>(nx*etamin);

  lammax = (90.0 - lammax)*Stomp::Deg2Rad();

  if (lammax >= Stomp::Pi()) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<unsigned long>(ny*((1.0 - cos(lammax))/2.0));
  }

  double etamax = Eta() + theta*sphere_correction;
  etamax -= Stomp::EtaOffSet();
  etamax *= Stomp::Deg2Rad();

  if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi();

  etamax /= 2.0*Stomp::Pi();
  x_max = static_cast<unsigned long>(nx*etamax);

  lammin = (90.0 - lammin)*Stomp::Deg2Rad();

  if (lammin >= Stomp::Pi()) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<unsigned long>(ny*((1.0 - cos(lammin))/2.0));
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

void StompPixel::XYBounds(double theta, std::vector<unsigned long>& x_min,
			  std::vector<unsigned long>& x_max, unsigned long& y_min,
			  unsigned long& y_max, bool add_buffer) {
  if (x_min.empty() == false) x_min.clear();
  if (x_max.empty() == false) x_max.clear();

  double lammin = Lambda() - theta;
  if (lammin < -90.0) lammin = -90.0;
  double lammax = Lambda() + theta;
  if (lammax > 90.0) lammax = 90.0;

  unsigned long ny = Stomp::Ny0()*resolution_;

  lammax = (90.0 - lammax)*Stomp::Deg2Rad();

  if (lammax >= Stomp::Pi()) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<unsigned long>(ny*((1.0 - cos(lammax))/2.0));
  }

  lammin = (90.0 - lammin)*Stomp::Deg2Rad();

  if (lammin >= Stomp::Pi()) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<unsigned long>(ny*((1.0 - cos(lammin))/2.0));
  }

  if (add_buffer) {
    if (y_max < ny - 1) y_max++;
    if (y_min > 0) y_min--;
  }

  if (x_min.empty() == false) x_min.clear();
  if (x_max.empty() == false) x_max.clear();

  x_min.reserve(y_max-y_min+1);
  x_max.reserve(y_max-y_min+1);

  unsigned long nx = Stomp::Nx0()*resolution_;

  for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
    double lam = 90.0 -
      Stomp::Rad2Deg()*acos(1.0 - 2.0*(y+0.5)/(Stomp::Ny0()*resolution_));

    double sphere_correction = 1.0 + 0.000192312*lam*lam -
      1.82764e-08*lam*lam*lam*lam + 1.28162e-11*lam*lam*lam*lam*lam*lam;

    double etamin = Eta() - theta*sphere_correction;
    etamin -= Stomp::EtaOffSet();
    etamin *= Stomp::Deg2Rad();

    if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi();

    etamin /= 2.0*Stomp::Pi();
    x_min.push_back(static_cast<unsigned long>(nx*etamin));

    double etamax = Eta() + theta*sphere_correction;
    etamax -= Stomp::EtaOffSet();
    etamax *= Stomp::Deg2Rad();

    if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi();

    etamax /= 2.0*Stomp::Pi();
    x_max.push_back(static_cast<unsigned long>(nx*etamax));

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

void StompPixel::WithinRadius(double theta, StompVector& pix,
                              bool check_full_pixel) {
  if (pix.empty() == false) pix.clear();

  unsigned long y_min, y_max;
  std::vector<unsigned long> x_min, x_max;

  XYBounds(theta,x_min,x_max,y_min,y_max,true);

  double sinthetamax = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());

  unsigned long nx = Stomp::Nx0()*resolution_;
  unsigned long nx_pix;

  if (check_full_pixel) {
    double costheta;
    for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
      if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
        nx_pix = nx - x_min[n] + x_max[n] + 1;
      } else {
        nx_pix = x_max[n] - x_min[n] + 1;
      }
      for (unsigned long m=0,x=x_min[n];m<nx_pix;m++,x++) {
        if (x == nx) x = 0;
        StompPixel tmp_pix(x,y,resolution_,1.0);
        bool within_bounds = true;
        costheta = UnitSphereX()*tmp_pix.UnitSphereX_UL() +
            UnitSphereY()*tmp_pix.UnitSphereY_UL() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_UL();
        if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
        costheta = UnitSphereX()*tmp_pix.UnitSphereX_UR() +
            UnitSphereY()*tmp_pix.UnitSphereY_UR() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_UR();
        if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
        costheta = UnitSphereX()*tmp_pix.UnitSphereX_LL() +
            UnitSphereY()*tmp_pix.UnitSphereY_LL() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_LL();
        if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
        costheta = UnitSphereX()*tmp_pix.UnitSphereX_LR() +
            UnitSphereY()*tmp_pix.UnitSphereY_LR() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_LR();
        if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
        if (within_bounds) pix.push_back(tmp_pix);
      }
    }
  } else {
    double costheta;
    for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
      if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
        nx_pix = nx - x_min[n] + x_max[n] + 1;
      } else {
        nx_pix = x_max[n] - x_min[n] + 1;
      }
      for (unsigned long m=0,x=x_min[n];m<nx_pix;m++,x++) {
        if (x == nx) x = 0;
        StompPixel tmp_pix(x,y,resolution_,1.0);
        costheta = UnitSphereX()*tmp_pix.UnitSphereX() +
            UnitSphereY()*tmp_pix.UnitSphereY() +
            UnitSphereZ()*tmp_pix.UnitSphereZ();
        if (1.0 - costheta*costheta < sinthetamax) pix.push_back(tmp_pix);
      }
    }
  }
}

void StompPixel::WithinAnnulus(double theta_min, double theta_max,
			       StompVector& pix, bool check_full_pixel) {
  if (pix.empty() == false) pix.clear();

  unsigned long y_min, y_max;
  std::vector<unsigned long> x_min, x_max;

  XYBounds(theta_max,x_min,x_max,y_min,y_max,true);

  double sinthetamax =
    sin(theta_max*Stomp::Deg2Rad())*sin(theta_max*Stomp::Deg2Rad());
  double sinthetamin =
    sin(theta_min*Stomp::Deg2Rad())*sin(theta_min*Stomp::Deg2Rad());
  if (theta_min < 1.0e-10) sinthetamin = -1.0;

  unsigned long nx = Stomp::Nx0()*resolution_;
  unsigned long nx_pix;

  if (check_full_pixel) {
    double costheta;
    for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
      if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
        nx_pix = nx - x_min[n] + x_max[n] + 1;
      } else {
        nx_pix = x_max[n] - x_min[n] + 1;
      }
      for (unsigned long m=0,x=x_min[n];m<nx_pix;m++,x++) {
        if (x == nx) x = 0;
        StompPixel tmp_pix(x,y,resolution_,1.0);
        bool within_bounds = true;
        costheta =
            UnitSphereX()*tmp_pix.UnitSphereX_UL() +
            UnitSphereY()*tmp_pix.UnitSphereY_UL() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_UL();
        if ((1.0 - costheta*costheta > sinthetamax) ||
            (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
        costheta =
            UnitSphereX()*tmp_pix.UnitSphereX_UR() +
            UnitSphereY()*tmp_pix.UnitSphereY_UR() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_UR();
        if ((1.0 - costheta*costheta > sinthetamax) ||
            (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
        costheta =
            UnitSphereX()*tmp_pix.UnitSphereX_LL() +
            UnitSphereY()*tmp_pix.UnitSphereY_LL() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_LL();
        if ((1.0 - costheta*costheta > sinthetamax) ||
            (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
        costheta =
            UnitSphereX()*tmp_pix.UnitSphereX_LR() +
            UnitSphereY()*tmp_pix.UnitSphereY_LR() +
            UnitSphereZ()*tmp_pix.UnitSphereZ_LR();
        if ((1.0 - costheta*costheta > sinthetamax) ||
            (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
        if (within_bounds) pix.push_back(tmp_pix);
      }
    }
  } else {
    double costheta;
    for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
      if ((x_max[n] < x_min[n]) && (x_min[n] > nx/2)) {
        nx_pix = nx - x_min[n] + x_max[n] + 1;
      } else {
        nx_pix = x_max[n] - x_min[n] + 1;
      }
      for (unsigned long m=0,x=x_min[n];m<nx_pix;m++,x++) {
        if (x == nx) x = 0;
        StompPixel tmp_pix(x,y,resolution_,1.0);
        costheta = UnitSphereX()*tmp_pix.UnitSphereX() +
            UnitSphereY()*tmp_pix.UnitSphereY() +
            UnitSphereZ()*tmp_pix.UnitSphereZ();
        if ((1.0 - costheta*costheta < sinthetamax) &&
            (1.0 - costheta*costheta > sinthetamin)) pix.push_back(tmp_pix);
      }
    }
  }
}

bool StompPixel::IsWithinRadius(AngularCoordinate& ang, double theta,
                                bool check_full_pixel) {

  double sinthetamax = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());

  bool within_bounds = true;
  if (check_full_pixel) {
    double costheta =
        ang.UnitSphereX()*UnitSphereX_UL() +
        ang.UnitSphereY()*UnitSphereY_UL() +
        ang.UnitSphereZ()*UnitSphereZ_UL();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_UR() +
        ang.UnitSphereY()*UnitSphereY_UR() +
        ang.UnitSphereZ()*UnitSphereZ_UR();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_LL() +
        ang.UnitSphereY()*UnitSphereY_LL() +
        ang.UnitSphereZ()*UnitSphereZ_LL();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_LR() +
        ang.UnitSphereY()*UnitSphereY_LR() +
        ang.UnitSphereZ()*UnitSphereZ_LR();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
  } else {
    double costheta =
        ang.UnitSphereX()*UnitSphereX() +
        ang.UnitSphereY()*UnitSphereY() +
        ang.UnitSphereZ()*UnitSphereZ();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
  }

  return within_bounds;
}

bool StompPixel::IsWithinRadius(StompPixel& pix, double theta,
                                bool check_full_pixel) {

  double sinthetamax = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());

  bool within_bounds = true;
  if (check_full_pixel) {
    double costheta =
        pix.UnitSphereX()*UnitSphereX_UL() +
        pix.UnitSphereY()*UnitSphereY_UL() +
        pix.UnitSphereZ()*UnitSphereZ_UL();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_UR() +
        pix.UnitSphereY()*UnitSphereY_UR() +
        pix.UnitSphereZ()*UnitSphereZ_UR();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_LL() +
        pix.UnitSphereY()*UnitSphereY_LL() +
        pix.UnitSphereZ()*UnitSphereZ_LL();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_LR() +
        pix.UnitSphereY()*UnitSphereY_LR() +
        pix.UnitSphereZ()*UnitSphereZ_LR();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
  } else {
    double costheta =
        pix.UnitSphereX()*UnitSphereX() +
        pix.UnitSphereY()*UnitSphereY() +
        pix.UnitSphereZ()*UnitSphereZ();
    if (1.0 - costheta*costheta > sinthetamax) within_bounds = false;
  }

  return within_bounds;
}

bool StompPixel::IsWithinAnnulus(AngularCoordinate& ang, double theta_min,
                                 double theta_max, bool check_full_pixel) {

  double sinthetamax =
    sin(theta_max*Stomp::Deg2Rad())*sin(theta_max*Stomp::Deg2Rad());
  double sinthetamin =
    sin(theta_min*Stomp::Deg2Rad())*sin(theta_min*Stomp::Deg2Rad());
  if (theta_min < 1.0e-10) sinthetamin = -1.0;

  bool within_bounds = true;
  if (check_full_pixel) {
    double costheta;
    costheta =
        ang.UnitSphereX()*UnitSphereX_UL() +
        ang.UnitSphereY()*UnitSphereY_UL() +
        ang.UnitSphereZ()*UnitSphereZ_UL();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_UR() +
        ang.UnitSphereY()*UnitSphereY_UR() +
        ang.UnitSphereZ()*UnitSphereZ_UR();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_LL() +
        ang.UnitSphereY()*UnitSphereY_LL() +
        ang.UnitSphereZ()*UnitSphereZ_LL();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        ang.UnitSphereX()*UnitSphereX_LR() +
        ang.UnitSphereY()*UnitSphereY_LR() +
        ang.UnitSphereZ()*UnitSphereZ_LR();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
  } else {
    double costheta;
    costheta =
        ang.UnitSphereX()*UnitSphereX() +
        ang.UnitSphereY()*UnitSphereY() +
        ang.UnitSphereZ()*UnitSphereZ();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
  }

  return within_bounds;
}

bool StompPixel::IsWithinAnnulus(StompPixel& pix, double theta_min,
                                 double theta_max, bool check_full_pixel) {

  double sinthetamax =
    sin(theta_max*Stomp::Deg2Rad())*sin(theta_max*Stomp::Deg2Rad());
  double sinthetamin =
    sin(theta_min*Stomp::Deg2Rad())*sin(theta_min*Stomp::Deg2Rad());
  if (theta_min < 1.0e-10) sinthetamin = -1.0;

  bool within_bounds = true;
  if (check_full_pixel) {
    double costheta;
    costheta =
        pix.UnitSphereX()*UnitSphereX_UL() +
        pix.UnitSphereY()*UnitSphereY_UL() +
        pix.UnitSphereZ()*UnitSphereZ_UL();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_UR() +
        pix.UnitSphereY()*UnitSphereY_UR() +
        pix.UnitSphereZ()*UnitSphereZ_UR();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_LL() +
        pix.UnitSphereY()*UnitSphereY_LL() +
        pix.UnitSphereZ()*UnitSphereZ_LL();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
    costheta =
        pix.UnitSphereX()*UnitSphereX_LR() +
        pix.UnitSphereY()*UnitSphereY_LR() +
        pix.UnitSphereZ()*UnitSphereZ_LR();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
  } else {
    double costheta;
    costheta =
        pix.UnitSphereX()*UnitSphereX() +
        pix.UnitSphereY()*UnitSphereY() +
        pix.UnitSphereZ()*UnitSphereZ();
    if ((1.0 - costheta*costheta > sinthetamax) ||
        (1.0 - costheta*costheta < sinthetamin)) within_bounds = false;
  }

  return within_bounds;
}

double StompPixel::RA() {
  double ra, dec;

  AngularCoordinate::SurveyToEquatorial(Lambda(),Eta(),ra,dec);

  return ra;
}

double StompPixel::DEC() {
  double ra, dec;

  AngularCoordinate::SurveyToEquatorial(Lambda(),Eta(),ra,dec);

  return dec;
}

double StompPixel::GalLon() {
  double gal_lon, gal_lat;

  AngularCoordinate::SurveyToGalactic(Lambda(),Eta(),gal_lat,gal_lon);

  return gal_lon;
}

double StompPixel::GalLat() {
  double gal_lon, gal_lat;

  AngularCoordinate::SurveyToGalactic(Lambda(),Eta(),gal_lat,gal_lon);

  return gal_lat;
}

std::ostream& operator<<(std::ostream& output, AngularCoordinate& ang) {
	output << "(" <<  ang.RA() << ", " << ang.DEC() <<")";
	return output;  
}


bool StompPixel::Contains(AngularCoordinate& ang) {

  double eta = (ang.Eta() - Stomp::EtaOffSet())*Stomp::Deg2Rad();

  if (eta <= 0.0) eta += 2.0*Stomp::Pi();

  eta /= 2.0*Stomp::Pi();

  if (x_ == static_cast<unsigned long>(Stomp::Nx0()*resolution_*eta)) {
    double lambda = (90.0 - ang.Lambda())*Stomp::Deg2Rad();

    if (lambda >= Stomp::Pi()) {
      if (y_ == Stomp::Ny0()*resolution_ - 1) {
        return true;
      } else {
        return false;
      }
    } else {
      if (y_ == static_cast<unsigned long>(Stomp::Ny0()*resolution_*
                                           ((1.0 - cos(lambda))/2.0))) {
        return true;
      } else {
        return false;
      }
    }
  } else {
    return false;
  }
}

void StompPixel::AreaIndex(int input_resolution, double lammin, double lammax,
                           double etamin, double etamax, unsigned long& x_min,
                           unsigned long& x_max, unsigned long& y_min,
                           unsigned long& y_max) {
  unsigned long nx = Stomp::Nx0()*input_resolution;

  etamin -= Stomp::EtaOffSet();
  etamin *= Stomp::Deg2Rad();

  if (etamin <= 0.0) etamin = etamin + 2.0*Stomp::Pi();

  etamin /= 2.0*Stomp::Pi();
  x_min = static_cast<unsigned long>(nx*etamin);


  etamax -= Stomp::EtaOffSet();
  etamax *= Stomp::Deg2Rad();

  if (etamax <= 0.0) etamax = etamax + 2.0*Stomp::Pi();

  etamax /= 2.0*Stomp::Pi();
  x_max = static_cast<unsigned long>(nx*etamax);


  unsigned long ny = Stomp::Ny0()*input_resolution;

  lammax = (90.0 - lammax)*Stomp::Deg2Rad();

  if (lammax >= Stomp::Pi()) {
    y_min = ny - 1;
  } else {
    y_min = static_cast<unsigned long>(ny*((1.0 - cos(lammax))/2.0));
  }


  lammin = (90.0 - lammin)*Stomp::Deg2Rad();

  if (lammin >= Stomp::Pi()) {
    y_max = ny - 1;
  } else {
    y_max = static_cast<unsigned long>(ny*((1.0 - cos(lammin))/2.0));
  }
}

int StompPixel::Pix2EtaStep(int input_resolution, unsigned long input_pixnum,
                            double theta) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  unsigned long y = input_pixnum/nx;

  double lam = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.5)/ny);

  double deta = 2.5/(input_resolution/4);

  double eta_step = theta*(1.0 + 0.000192312*lam*lam -
			   1.82764e-08*lam*lam*lam*lam +
			   1.28162e-11*lam*lam*lam*lam*lam*lam);

  int etastep = 1;

  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

int StompPixel::EtaStep(double theta) {
  double lam = Lambda();

  double deta = 2.5/(resolution_/4);

  double eta_step = theta*(1.0 + 0.000192312*lam*lam -
			   1.82764e-08*lam*lam*lam*lam +
			   1.28162e-11*lam*lam*lam*lam*lam*lam);

  int etastep = 1;

  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

int StompPixel::Stripe(int input_resolution) {
  if ((input_resolution%2 != 0) || (input_resolution < 4)) {
    std::cout << "Illegal resolution in Stripe() call!\nExiting...\n";
    exit(1);
  }

  double stripe_width = 360.0/(Stomp::Nx0()*input_resolution);

  int stripe = static_cast<int>((Eta() + 32.5)/stripe_width) + 10;

  double etamin = stripe_width*(stripe - 10) - 32.5 -
      stripe_width/2.0 + 0.0000001;
  double etamax = stripe_width*(stripe - 10) - 32.5 +
      stripe_width/2.0 - 0.0000001;

  if (Eta() < etamin) stripe++;
  if (Eta() > etamax) stripe++;

  if (stripe < 0) stripe += Stomp::Nx0()*input_resolution;

  return stripe;
}

bool StompPixel::LocalOrder(StompPixel pix_a, StompPixel pix_b) {
  if (pix_a.Resolution() == pix_b.Resolution()) {
    if (pix_a.PixelY() == pix_b.PixelY()) {
      if (pix_a.PixelX() < pix_b.PixelX()) {
        return true;
      } else {
        return false;
      }
    } else {
      if (pix_a.PixelY() < pix_b.PixelY()) {
        return true;
      } else {
        return false;
      }
    }
  } else {
    if (pix_a.Resolution() < pix_b.Resolution()) {
      return true;
    } else {
      return false;
    }
  }
}

bool StompPixel::LocalOrderByReference(StompPixel& pix_a, StompPixel& pix_b) {
  if (pix_a.Resolution() == pix_b.Resolution()) {
    if (pix_a.PixelY() == pix_b.PixelY()) {
      if (pix_a.PixelX() < pix_b.PixelX()) {
        return true;
      } else {
        return false;
      }
    } else {
      if (pix_a.PixelY() < pix_b.PixelY()) {
        return true;
      } else {
        return false;
      }
    }
  } else {
    if (pix_a.Resolution() < pix_b.Resolution()) {
      return true;
    } else {
      return false;
    }
  }
}

bool StompPixel::SuperPixelBasedOrder(StompPixel pix_a, StompPixel pix_b) {

  if (pix_a.Superpixnum() == pix_b.Superpixnum()) {
    if (pix_a.Resolution() == pix_b.Resolution()) {
      if (pix_a.HPixnum() < pix_b.HPixnum()) {
	return true;
      } else {
	return false;
      }
    } else {
      if (pix_a.Resolution() < pix_b.Resolution()) {
	return true;
      } else {
	return false;
      }
    }
  } else {
    if (pix_a.Superpixnum() < pix_b.Superpixnum()) {
      return true;
    } else {
      return false;
    }
  }
}

bool StompPixel::SuperPixelOrder(StompPixel pix_a, StompPixel pix_b) {
  if (pix_a.Superpixnum() < pix_b.Superpixnum()) {
    return true;
  } else {
    return false;
  }
}

void StompPixel::ResolvePixel(StompVector& pix, bool ignore_weight) {
  sort(pix.begin(),pix.end(),StompPixel::SuperPixelBasedOrder);

  StompVector tmp_pix;
  StompVector final_pix;
  unsigned long superpixnum = pix[0].Superpixnum();

  for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
    if (superpixnum == iter->Superpixnum()) {
      tmp_pix.push_back(*iter);
    } else {
      ResolveSuperPixel(tmp_pix,ignore_weight);

      for (unsigned long i=0;i<tmp_pix.size();i++)
        final_pix.push_back(tmp_pix[i]);

      tmp_pix.clear();
      superpixnum = iter->Superpixnum();
      tmp_pix.push_back(*iter);
    }
  }

  ResolveSuperPixel(tmp_pix,ignore_weight);

  for (unsigned long i=0;i<tmp_pix.size();i++)
    final_pix.push_back(tmp_pix[i]);

  tmp_pix.clear();

  pix.clear();
  pix.reserve(final_pix.size());

  for (unsigned long i=0;i<final_pix.size();i++) pix.push_back(final_pix[i]);

  final_pix.clear();
}

void StompPixel::ResolveSuperPixel(StompVector& pix, bool ignore_weight) {

  sort(pix.begin(),pix.end(),StompPixel::SuperPixelBasedOrder);

  if (ignore_weight)
    for (unsigned long i=0;i<pix.size();i++) pix[i].SetWeight(1.0);

  StompVector unique_pix;
  StompIterator search_end = pix.begin();

  unique_pix.push_back(pix[0]);
  ++search_end;
  for (unsigned long i=1;i<pix.size();i++) {

    bool keep_pixel = true;

    if (StompPixel::WeightedPixelMatch(pix[i],pix[i-1])) keep_pixel = false;

    if ((keep_pixel) && (pix[i].Resolution() > Stomp::HPixResolution())) {
      StompPixel tmp_pix = pix[i];

      while (tmp_pix.Resolution() > Stomp::HPixResolution()) {
        tmp_pix.SetToSuperPix(tmp_pix.Resolution()/2);

        if (binary_search(pix.begin(),search_end,tmp_pix,
                          StompPixel::SuperPixelBasedOrder))
          keep_pixel = false;
      }
    }

    if (keep_pixel) unique_pix.push_back(pix[i]);
    ++search_end;
  }

  unsigned long n_start = pix.size();

  pix.clear();
  pix.reserve(unique_pix.size());

  for (unsigned long i=0;i<unique_pix.size();i++) pix.push_back(unique_pix[i]);

  sort(pix.begin(),pix.end(),StompPixel::SuperPixelBasedOrder);

  unique_pix.clear();

  unsigned long n_finish = unique_pix.size();

  while (n_start != n_finish) {
    n_start = pix.size();

    unique_pix.reserve(pix.size());
    for (unsigned long i=0;i<pix.size();i++) {
      if (pix[i].Resolution() > Stomp::HPixResolution()) {
        bool found_a = false, found_b = false, found_c = false;
        StompPixel pix_a = pix[i];
        StompPixel pix_b = pix[i];
        StompPixel pix_c = pix[i];

	pix[i].CohortPix(pix_a,pix_b,pix_c);

        if (StompPixel::SuperPixelBasedOrder(pix[i],pix_a) &&
            StompPixel::SuperPixelBasedOrder(pix[i],pix_b) &&
            StompPixel::SuperPixelBasedOrder(pix[i],pix_c)) {

          StompPair iter;

          iter = equal_range(pix.begin(),pix.end(),pix_a,
                             StompPixel::SuperPixelBasedOrder);
          if ((iter.first != iter.second) &&
              (StompPixel::WeightMatch(pix[i],*iter.first))) {
	    found_a = true;
	  } else {
	    found_a = false;
	  }

          iter = equal_range(pix.begin(),pix.end(),pix_b,
                             StompPixel::SuperPixelBasedOrder);
          if ((iter.first != iter.second) &&
              (StompPixel::WeightMatch(pix[i],*iter.first))) {
	    found_b = true;
	  } else {
	    found_b = false;
	  }

          iter = equal_range(pix.begin(),pix.end(),pix_c,
                             StompPixel::SuperPixelBasedOrder);
          if ((iter.first != iter.second) &&
              (StompPixel::WeightMatch(pix[i],*iter.first))) {
	    found_c = true;
	  } else {
	    found_c = false;
	  }

          if (found_a && found_b && found_c) {
	    pix_a = pix[i];
	    pix_a.SetToSuperPix(pix_a.Resolution()/2);
	    unique_pix.push_back(pix_a);
	  } else {
            unique_pix.push_back(pix[i]);
          }
        } else {
          unique_pix.push_back(pix[i]);
        }
      } else {
	unique_pix.push_back(pix[i]);
      }
    }

    if (unique_pix.size() != pix.size()) {
      std::cout <<
	"Something has gone wrong searching for superpixels. Exiting.\n";
      exit(1);
    }

    sort(unique_pix.begin(),unique_pix.end(),StompPixel::SuperPixelBasedOrder);

    for (unsigned long i=0;i<unique_pix.size();i++) pix[i] = unique_pix[i];

    unique_pix.clear();

    unique_pix.push_back(pix[0]);
    search_end = pix.begin();
    ++search_end;
    for (unsigned long i=1;i<pix.size();i++) {

      bool keep_pixel = true;

      if (StompPixel::PixelMatch(pix[i],pix[i-1])) keep_pixel = false;

      if ((keep_pixel) && (pix[i].Resolution() > Stomp::HPixResolution())) {
        StompPixel tmp_pix = pix[i];

        while (tmp_pix.Resolution() > Stomp::HPixResolution()) {
          tmp_pix.SetToSuperPix(tmp_pix.Resolution()/2);

          if (binary_search(pix.begin(),search_end,tmp_pix,
                            StompPixel::SuperPixelBasedOrder))
            keep_pixel = false;
        }
      }

      if (keep_pixel) unique_pix.push_back(pix[i]);
      ++search_end;
    }

    n_finish = unique_pix.size();

    pix.clear();

    pix.reserve(unique_pix.size());

    for (unsigned long i=0;i<unique_pix.size();i++)
      pix.push_back(unique_pix[i]);

    unique_pix.clear();
  }
}

void StompPixel::Ang2HPix(int input_resolution, AngularCoordinate& ang,
                          unsigned long& output_hpixnum,
                          unsigned long& output_superpixnum) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  int hnx = input_resolution/Stomp::HPixResolution();

  double eta = (ang.Eta() - Stomp::EtaOffSet())*Stomp::Deg2Rad();

  if (eta <= 0.0) eta += 2.0*Stomp::Pi();

  eta /= 2.0*Stomp::Pi();
  unsigned long x = static_cast<unsigned long>(nx*eta);

  double lambda = (90.0 - ang.Lambda())*Stomp::Deg2Rad();

  unsigned long y;
  if (lambda >= Stomp::Pi()) {
    y = ny - 1;
  } else {
    y = static_cast<unsigned long>(ny*((1.0 - cos(lambda))/2.0));
  }

  unsigned long x0 = x/hnx;
  unsigned long y0 = y/hnx;

  x -= x0*hnx;
  y -= y0*hnx;

  output_hpixnum = nx*y + x;
  output_superpixnum = Stomp::Nx0()*Stomp::HPixResolution()*y0 + x0;
}

void StompPixel::HPix2Ang(int input_resolution,
                          unsigned long input_hpixnum,
                          unsigned long input_superpixnum,
                          AngularCoordinate& ang) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long y0 = input_superpixnum/(Stomp::Nx0()*Stomp::HPixResolution());
  unsigned long x0 =
    input_superpixnum - y0*Stomp::Nx0()*Stomp::HPixResolution();

  y0 *= hnx;
  x0 *= hnx;

  unsigned long y = input_hpixnum/hnx;
  unsigned long x = input_hpixnum - hnx*y;

  ang.SetSurveyCoordinates(90.0-Stomp::Rad2Deg()*acos(1.0-2.0*(y+y0+0.5)/ny),
			   Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+x0+0.5))/nx +
			   Stomp::EtaOffSet());
}

void StompPixel::XY2HPix(int input_resolution, unsigned long x,
                         unsigned long y, unsigned long& output_hpixnum,
                         unsigned long& output_superpixnum) {
  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long x0 = x/hnx;
  unsigned long y0 = y/hnx;

  x -= x0;
  y -= y0;

  output_hpixnum = hnx*y + x;
  output_superpixnum = Stomp::Nx0()*Stomp::HPixResolution()*y0 + x0;
}

void StompPixel::HPix2XY(int input_resolution, unsigned long input_hpixnum,
                         unsigned long input_superpixnum,
                         unsigned long& x, unsigned long& y) {
  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long y0 =
    input_superpixnum/(Stomp::Nx0()*Stomp::HPixResolution());
  unsigned long x0 =
    input_superpixnum - y0*Stomp::Nx0()*Stomp::HPixResolution();

  unsigned long tmp_y = input_hpixnum/hnx;
  unsigned long tmp_x = input_hpixnum - hnx*tmp_y;

  x = tmp_x + x0*hnx;
  y = tmp_y + y0*hnx;
}

void StompPixel::SuperHPix(int hi_resolution, unsigned long hi_hpixnum,
                           int lo_resolution, unsigned long& lo_hpixnum) {
  if (hi_resolution < lo_resolution) {
    std::cout << "Can't go from low resolution to higher resolution.\n ";
    exit(1);
  } else {
    unsigned long nx_hi = hi_resolution/Stomp::HPixResolution();
    unsigned long nx_lo = lo_resolution/Stomp::HPixResolution();

    int ratio = hi_resolution/lo_resolution;

    unsigned long y = hi_hpixnum/nx_hi;
    unsigned long x = hi_hpixnum - nx_hi*y;

    x /= ratio;
    y /= ratio;

    lo_hpixnum = nx_lo*y + x;
  }
}

void StompPixel::NextSubHPix(int input_resolution, unsigned long input_hpixnum,
                             unsigned long& sub_hpixnum1,
                             unsigned long& sub_hpixnum2,
                             unsigned long& sub_hpixnum3,
                             unsigned long& sub_hpixnum4) {
  unsigned long nx_hi = 2*input_resolution/Stomp::HPixResolution();
  unsigned long nx_lo = input_resolution/Stomp::HPixResolution();

  unsigned long y = input_hpixnum/nx_lo;
  unsigned long x = input_hpixnum - nx_lo*y;

  sub_hpixnum1 = nx_hi*(2*y) + 2*x;
  sub_hpixnum2 = nx_hi*(2*y) + 2*x + 1;
  sub_hpixnum3 = nx_hi*(2*y + 1) + 2*x;
  sub_hpixnum4 = nx_hi*(2*y + 1) + 2*x + 1;
}

void StompPixel::SubHPix(int lo_resolution, unsigned long lo_hpixnum,
                         unsigned long lo_superpixnum, int hi_resolution,
                         unsigned long& x_min, unsigned long& x_max,
                         unsigned long& y_min, unsigned long& y_max) {
  unsigned long tmp_x, tmp_y;

  if (lo_resolution == hi_resolution) {
    HPix2XY(lo_resolution,lo_hpixnum,lo_superpixnum,tmp_x,tmp_y);

    y_min = tmp_y;
    y_max = tmp_y;
    x_min = tmp_x;
    x_max = tmp_x;
  } else {
    unsigned long tmp_hpixnum, hpixnum1, hpixnum2, hpixnum3, hpixnum4;
    int tmp_res;

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

void StompPixel::HPixelBound(int input_resolution,
                             unsigned long input_hpixnum,
                             unsigned long input_superpixnum,
                             double& lammin, double& lammax,
                             double& etamin, double& etamax) {
  unsigned long nx = Stomp::Nx0()*input_resolution;
  unsigned long ny = Stomp::Ny0()*input_resolution;

  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long y0 = input_superpixnum/(Stomp::Nx0()*Stomp::HPixResolution());
  unsigned long x0 =
    input_superpixnum - y0*Stomp::Nx0()*Stomp::HPixResolution();

  y0 *= hnx;
  x0 *= hnx;

  unsigned long y = input_hpixnum/hnx;
  unsigned long x = input_hpixnum - hnx*y;

  lammin = 90.0 - Stomp::Rad2Deg()*acos(1.0 - 2.0*(y+y0+1)/ny);
  lammax = 90.0 - Stomp::Rad2Deg()*acos(1.0 - 2.0*(y+y0)/ny);
  etamin =
    Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x+x0+0.0)/nx + Stomp::EtaOffSet();
  if (etamin >= 180.0) etamin = etamin - 360.0;
  etamax =
    Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x+x0+1.0)/nx + Stomp::EtaOffSet();
  if (etamax >= 180.0) etamax = etamax - 360.0;
}

void StompPixel::CohortHPix(int input_resolution, unsigned long input_hpixnum,
                            unsigned long& co_hpixnum1,
                            unsigned long& co_hpixnum2,
                            unsigned long& co_hpixnum3) {
  unsigned long tmp_hpixnum, hpixnum1, hpixnum2, hpixnum3, hpixnum4;

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

int StompPixel::HPix2EtaStep(int input_resolution,
                             unsigned long input_hpixnum,
                             unsigned long input_superpixnum,
                             double theta) {

  unsigned long ny = Stomp::Ny0()*input_resolution;

  int hnx = input_resolution/Stomp::HPixResolution();

  unsigned long y0 = input_superpixnum/(Stomp::Nx0()*Stomp::HPixResolution());
  unsigned long x0 =
    input_superpixnum - y0*Stomp::Nx0()*Stomp::HPixResolution();

  y0 *= hnx;
  x0 *= hnx;

  unsigned long y = input_hpixnum/hnx;

  double lam = 90.0-Stomp::Rad2Deg()*acos(1.0-2.0*(y+y0+0.5)/ny);

  double deta = 2.5/(input_resolution/4);

  double eta_step = theta*(1.0 + 0.000192312*lam*lam -
			   1.82764e-08*lam*lam*lam*lam +
			   1.28162e-11*lam*lam*lam*lam*lam*lam);
  int etastep = 1;

  while (eta_step > etastep*deta) etastep++;

  return etastep;
}

StompDensityPixel::StompDensityPixel() {
  density_ = 0.0;
}

StompDensityPixel::StompDensityPixel(const int input_resolution,
				     const unsigned long input_pixnum,
				     const double input_weight,
				     const double input_density) {
  SetResolution(input_resolution);

  unsigned long tmp_y = input_pixnum/(Stomp::Nx0()*Resolution());
  unsigned long tmp_x = input_pixnum - Stomp::Nx0()*Resolution()*tmp_y;

  SetPixnumFromXY(tmp_x,tmp_y);
  SetWeight(input_weight);
  density_ = input_density;
}

StompDensityPixel::StompDensityPixel(const int input_resolution,
				     const unsigned long input_hpixnum,
				     const unsigned long input_superpixnum,
				     const double input_weight,
				     const double input_density) {

  SetResolution(input_resolution);

  unsigned long tmp_x, tmp_y;
  StompPixel::HPix2XY(input_resolution,input_hpixnum,input_superpixnum,
		      tmp_x,tmp_y);

  SetPixnumFromXY(tmp_x,tmp_y);
  SetWeight(input_weight);
  density_ = input_density;
}

StompDensityPixel:: StompDensityPixel(const unsigned long input_x,
				      const unsigned long input_y,
				      const int input_resolution,
				      const double input_weight,
				      const double input_density) {

  SetResolution(input_resolution);
  SetPixnumFromXY(input_x,input_y);
  SetWeight(input_weight);
  density_ = input_density;
}

StompDensityPixel::StompDensityPixel(AngularCoordinate& ang,
                                     const int input_resolution,
                                     const double input_weight,
                                     const double input_density) {

  SetResolution(input_resolution);
  SetPixnumFromAng(ang);
  SetWeight(input_weight);
  density_ = input_density;
}

StompDensityPixel::~StompDensityPixel() {
  density_ = 0.0;
}

StompPointPixel::StompPointPixel() {
}

StompPointPixel::StompPointPixel(const int input_resolution,
				 const unsigned long input_pixnum,
				 const double input_weight) {
  SetResolution(input_resolution);

  unsigned long tmp_y = input_pixnum/(Stomp::Nx0()*Resolution());
  unsigned long tmp_x = input_pixnum - Stomp::Nx0()*Resolution()*tmp_y;

  SetPixnumFromXY(tmp_x,tmp_y);
  SetWeight(input_weight);
}

StompPointPixel::StompPointPixel(const int input_resolution,
				 const unsigned long input_hpixnum,
				 const unsigned long input_superpixnum,
				 const double input_weight) {

  SetResolution(input_resolution);

  unsigned long tmp_x, tmp_y;
  StompPixel::HPix2XY(input_resolution,input_hpixnum,input_superpixnum,
		      tmp_x,tmp_y);

  SetPixnumFromXY(tmp_x,tmp_y);
  SetWeight(input_weight);
}

StompPointPixel:: StompPointPixel(const unsigned long input_x,
				  const unsigned long input_y,
				  const int input_resolution,
				  const double input_weight) {

  SetResolution(input_resolution);
  SetPixnumFromXY(input_x,input_y);
  SetWeight(input_weight);
}

StompPointPixel::StompPointPixel(AngularCoordinate& ang,
				 const int input_resolution,
				 const double input_weight) {

  SetResolution(input_resolution);
  SetPixnumFromAng(ang);
  SetWeight(input_weight);
}

StompPointPixel::~StompPointPixel() {
  ang_.clear();
}

StompSubMap::StompSubMap(unsigned long superpixnum) {
  superpixnum_ = superpixnum;
  area_ = 0.0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  StompPixel::PixelBound(Stomp::HPixResolution(), superpixnum, lambda_min_,
			 lambda_max_, eta_min_, eta_max_);
  z_min_ = sin(lambda_min_*Stomp::Deg2Rad());
  z_max_ = sin(lambda_max_*Stomp::Deg2Rad());
  initialized_ = false;
  modified_ = false;
}

StompSubMap::~StompSubMap() {
  if (pix_.empty() == false) pix_.clear();
  superpixnum_ = Stomp::MaxSuperpixnum();
  initialized_ = false;
}

void StompSubMap::AddPixel(StompPixel& pix) {
  pix_.push_back(pix);
  modified_ = true;
}

void StompSubMap::Resolve() {
  if (pix_.size() != size_) modified_ = true;

  if (modified_) {
    StompPixel::ResolveSuperPixel(pix_);

    area_ = 0.0;
    min_resolution_ = Stomp::MaxPixelResolution();
    max_resolution_ = Stomp::HPixResolution();
    min_weight_ = 1.0e30;
    max_weight_ = -1.0e30;

    for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      area_ += iter->Area();
      if (iter->Resolution() < min_resolution_)
        min_resolution_ = iter->Resolution();
      if (iter->Resolution() > max_resolution_)
        max_resolution_ = iter->Resolution();
      if (iter->Weight() < min_weight_) min_weight_ = iter->Weight();
      if (iter->Weight() > max_weight_) max_weight_ = iter->Weight();
    }

    initialized_ = true;
    modified_ = false;
    size_ = pix_.size();
  }
}

bool StompSubMap::FindLocation(AngularCoordinate& ang, double& weight) {
  bool keep = false;

  for (int resolution=min_resolution_;
       resolution<=max_resolution_;resolution*=2) {
    StompPixel tmp_pix(ang,resolution);

    StompPair iter = equal_range(pix_.begin(),pix_.end(),tmp_pix,
                                 StompPixel::SuperPixelBasedOrder);

    if (iter.first != iter.second) {
      keep = true;
      weight = iter.first->Weight();
    }
    if (keep) resolution = max_resolution_*2;
  }

  return keep;
}

double StompSubMap::FindUnmaskedFraction(StompPixel& pix) {
  StompIterator iter;

  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    StompPixel tmp_pix(pix.Resolution()*2,0,pix.Superpixnum(),1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),
                       tmp_pix,StompPixel::SuperPixelBasedOrder);
  }


  int resolution = min_resolution_;
  double unmasked_fraction = 0.0;
  bool found_pixel = false;

  while ((resolution <= pix.Resolution()) && (found_pixel == false)) {
    StompPixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);

    StompPair super_iter = equal_range(pix_.begin(),iter,tmp_pix,
                                       StompPixel::SuperPixelBasedOrder);

    if (super_iter.first != super_iter.second) {
      found_pixel = true;
      unmasked_fraction = 1.0;
    }
    resolution *= 2;
  }

  while ((iter != pix_.end()) && (found_pixel == false)) {
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

double StompSubMap::FindAverageWeight(StompPixel& pix) {
  StompIterator iter;

  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    StompPixel tmp_pix(pix.Resolution()*2,0,pix.Superpixnum(),1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),tmp_pix,
                       StompPixel::SuperPixelBasedOrder);
  }

  double unmasked_fraction = 0.0, weighted_average = 0.0;
  bool found_pixel = false;

  int resolution = min_resolution_;
  while ((resolution <= pix.Resolution()) && (found_pixel == false)) {
    StompPixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);

    StompPair super_iter = equal_range(pix_.begin(),iter,tmp_pix,
                                       StompPixel::SuperPixelBasedOrder);

    if (super_iter.first != super_iter.second) {
      found_pixel = true;
      weighted_average = super_iter.first->Weight();
      unmasked_fraction = 1.0;
    }
    resolution *= 2;
  }

  while ((iter != pix_.end()) && (found_pixel == false)) {
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

void StompSubMap::FindMatchingPixels(StompPixel& pix,
                                     StompVector& match_pix,
                                     bool use_local_weights) {
  if (match_pix.empty() == false) match_pix.clear();

  bool found_pixel = false;

  StompIterator iter, find_iter;

  if (pix.Resolution() == max_resolution_) {
    iter = pix_.end();
  } else {
    StompPixel tmp_pix(pix.Resolution()*2,0,pix.Superpixnum(),1.0);
    iter = lower_bound(pix_.begin(),pix_.end(),tmp_pix,
                       StompPixel::SuperPixelBasedOrder);
  }

  int resolution = min_resolution_;
  while ((resolution <= pix.Resolution()) && (found_pixel == false)) {
    StompPixel tmp_pix = pix;
    tmp_pix.SetToSuperPix(resolution);

    find_iter = lower_bound(pix_.begin(),iter,tmp_pix,
                            StompPixel::SuperPixelBasedOrder);
    if (StompPixel::PixelMatch(*find_iter,tmp_pix)) {
      found_pixel = true;
      tmp_pix = pix;
      if (use_local_weights) tmp_pix.SetWeight(find_iter->Weight());
      match_pix.push_back(tmp_pix);
    }
    resolution *= 2;
  }

  while ((iter != pix_.end()) && (found_pixel == false)) {
    if (pix.Contains(*iter)) {
      StompPixel tmp_pix = *iter;
      tmp_pix = *iter;
      if (use_local_weights == false) tmp_pix.SetWeight(pix.Weight());
      match_pix.push_back(tmp_pix);
    }

    ++iter;
  }
}

double StompSubMap::AverageWeight() {
  double unmasked_fraction = 0.0, weighted_average = 0.0;

  if (initialized_) {
    for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      weighted_average += iter->Weight()*iter->Area();
      unmasked_fraction += iter->Area();
    }

    weighted_average /= unmasked_fraction;
  }

  return weighted_average;
}

bool StompSubMap::Add(StompVector& input_pix, int max_resolution,
                      bool drop_single) {

  std::vector<double> tmp_weight;
  std::vector<short> tmp_found;

  if (max_resolution < max_resolution_)
    max_resolution = max_resolution_;

  unsigned long x_min, x_max, y_min, y_max;

  StompPixel::SubPix(Stomp::HPixResolution(),superpixnum_,
                     max_resolution,x_min,x_max,y_min,y_max);

  unsigned long n_sub_pixel =
      max_resolution*max_resolution/
      (Stomp::HPixResolution()*Stomp::HPixResolution());

  tmp_weight.reserve(n_sub_pixel);
  tmp_found.reserve(n_sub_pixel);

  for (unsigned long i=0;i<n_sub_pixel;i++) {
    tmp_weight.push_back(0.0);
    tmp_found.push_back(0);
  }

  unsigned long hnx = max_resolution/Stomp::HPixResolution();
  unsigned long hny = max_resolution/Stomp::HPixResolution();

  unsigned long x,y,hpixnum,x0,y0;
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_weight[hpixnum] += iter->Weight();
        tmp_found[hpixnum] += 1;
      }
    }
  }

  for (StompIterator iter=input_pix.begin();iter!=input_pix.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_weight[hpixnum] += iter->Weight();
        tmp_found[hpixnum] += 1;
      }
    }
  }

  Clear();

  if (drop_single) {
    for (unsigned long hpixnum=0;hpixnum<n_sub_pixel;hpixnum++) {
      if (tmp_found[hpixnum] == 2) {
        StompPixel tmp_pix(max_resolution,hpixnum,superpixnum_,
                           tmp_weight[hpixnum]);
        AddPixel(tmp_pix);
      }
    }
  } else {
    for (unsigned long hpixnum=0;hpixnum<n_sub_pixel;hpixnum++) {
      if (tmp_found[hpixnum] > 0) {
        StompPixel tmp_pix(max_resolution,hpixnum,superpixnum_,
                           tmp_weight[hpixnum]);
        AddPixel(tmp_pix);
      }
    }
  }

  if (modified_) Resolve();

  return true;
}

bool StompSubMap::Multiply(StompVector& input_pix, int max_resolution,
                           bool drop_single) {

  std::vector<double> tmp_weight;
  std::vector<short> tmp_found;

  if (max_resolution < max_resolution_)
    max_resolution = max_resolution_;

  unsigned long x_min, x_max, y_min, y_max;

  StompPixel::SubPix(Stomp::HPixResolution(),superpixnum_,
                     max_resolution,x_min,x_max,y_min,y_max);

  unsigned long n_sub_pixel =
      max_resolution*max_resolution/
      (Stomp::HPixResolution()*Stomp::HPixResolution());

  tmp_weight.reserve(n_sub_pixel);
  tmp_found.reserve(n_sub_pixel);

  for (unsigned long i=0;i<n_sub_pixel;i++) {
    tmp_weight.push_back(1.0);
    tmp_found.push_back(0);
  }

  unsigned long hnx = max_resolution/Stomp::HPixResolution();
  unsigned long hny = max_resolution/Stomp::HPixResolution();

  unsigned long x,y,hpixnum,x0,y0;
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_weight[hpixnum] *= iter->Weight();
        tmp_found[hpixnum] += 1;
      }
    }
  }

  for (StompIterator iter=input_pix.begin();iter!=input_pix.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_weight[hpixnum] *= iter->Weight();
        tmp_found[hpixnum] += 1;
      }
    }
  }

  Clear();

  if (drop_single) {
    for (unsigned long hpixnum=0;hpixnum<n_sub_pixel;hpixnum++) {
      if (tmp_found[hpixnum] == 2) {
        StompPixel tmp_pix(max_resolution,hpixnum,superpixnum_,
                           tmp_weight[hpixnum]);
        AddPixel(tmp_pix);
      }
    }
  } else {
    for (unsigned long hpixnum=0;hpixnum<n_sub_pixel;hpixnum++) {
      if (tmp_found[hpixnum] > 0) {
        StompPixel tmp_pix(max_resolution,hpixnum,superpixnum_,
                           tmp_weight[hpixnum]);
        AddPixel(tmp_pix);
      }
    }
  }

  if (modified_) Resolve();

  return true;
}

bool StompSubMap::Exclude(StompVector& input_pix, int max_resolution) {

  std::vector<double> tmp_weight;
  std::vector<short> tmp_found;

  if (max_resolution < max_resolution_)
    max_resolution = max_resolution_;

  unsigned long x_min, x_max, y_min, y_max;

  StompPixel::SubPix(Stomp::HPixResolution(),superpixnum_,
                     max_resolution,x_min,x_max,y_min,y_max);

  unsigned long n_sub_pixel =
      max_resolution*max_resolution/
      (Stomp::HPixResolution()*Stomp::HPixResolution());

  tmp_weight.reserve(n_sub_pixel);
  tmp_found.reserve(n_sub_pixel);

  for (unsigned long i=0;i<n_sub_pixel;i++) {
    tmp_weight.push_back(0.0);
    tmp_found.push_back(0);
  }

  unsigned long hnx = max_resolution/Stomp::HPixResolution();
  unsigned long hny = max_resolution/Stomp::HPixResolution();

  unsigned long x,y,hpixnum,x0,y0;
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_weight[hpixnum] = iter->Weight();
        tmp_found[hpixnum] = 1;
      }
    }
  }

  for (StompIterator iter=input_pix.begin();iter!=input_pix.end();++iter) {
    iter->SubPix(max_resolution,x_min,x_max,y_min,y_max);
    for (y=y_min;y<=y_max;y++) {
      y0 = y/hny;
      for (x=x_min;x<=x_max;x++) {
        x0 = x/hnx;
        hpixnum = hnx*(y - y0*hny) + (x - x0*hnx);
        tmp_found[hpixnum] = 0;
      }
    }
  }

  Clear();

  for (unsigned long hpixnum=0;hpixnum<n_sub_pixel;hpixnum++) {
    if (tmp_found[hpixnum] == 1) {
      StompPixel tmp_pix(max_resolution,hpixnum,superpixnum_,
                         tmp_weight[hpixnum]);
      AddPixel(tmp_pix);
    }
  }

  if (modified_) Resolve();

  return true;
}

void StompSubMap::ScaleWeight(const double weight_scale) {
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter)
    iter->SetWeight(iter->Weight()*weight_scale);
}

void StompSubMap::AddConstantWeight(const double add_weight) {
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter)
    iter->SetWeight(iter->Weight()+add_weight);
}

void StompSubMap::InvertWeight() {
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    if ((iter->Weight() > 1.0e-15) || (iter->Weight() < -1.0e-15)) {
      iter->SetWeight(1.0/iter->Weight());
    } else {
      iter->SetWeight(0.0);
    }
  }
}

void StompSubMap::Pixels(StompVector& pix) {
  if (pix.empty() == false) pix.clear();
  for (StompIterator iter=pix_.begin();iter!=pix_.end();++iter)
    pix.push_back(*iter);
}

void StompSubMap::Clear() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  if (pix_.empty() == false) pix_.clear();
  initialized_ = false;
  modified_ = false;
}

StompMap::StompMap() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;

  sub_map_.reserve(Stomp::MaxSuperpixnum());

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    StompSubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }
}

StompMap::StompMap(StompVector& pix) {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;

  sub_map_.reserve(Stomp::MaxSuperpixnum());

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    StompSubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
    unsigned long k = iter->Superpixnum();

    sub_map_[k].AddPixel(*iter);
  }

  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Modified()) {
      iter->Resolve();

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
    }
  }
}

StompMap::StompMap(std::string& InputFile, bool hpixel_format,
		   bool weighted_map) {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;

  sub_map_.reserve(Stomp::MaxSuperpixnum());

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    StompSubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  std::ifstream input_file(InputFile.c_str());

  unsigned long hpixnum, superpixnum, pixnum;
  int resolution;
  double weight;

  while (!input_file.eof()) {
    if (hpixel_format) {
      if (weighted_map) {
        input_file >> hpixnum >> superpixnum >> resolution >> weight;
      } else {
        input_file >> hpixnum >> superpixnum >> resolution;
        weight = 1.0;
      }
    } else {
      if (weighted_map) {
        input_file >> pixnum >> resolution >> weight;
      } else {
        input_file >> pixnum >> resolution;
        weight = 1.0;
      }
    }

    if (!input_file.eof()) {
      if (!hpixel_format)
        StompPixel::Pix2HPix(resolution,pixnum,hpixnum,superpixnum);

      StompPixel tmp_pix(resolution,hpixnum,superpixnum,weight);

      sub_map_[superpixnum].AddPixel(tmp_pix);
    }
  }

  input_file.close();

  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Modified()) {
      iter->Resolve();

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
    }
  }
}

StompMap::~StompMap() {
  min_resolution_ = max_resolution_ = -1;
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;
  Clear();
}

bool StompMap::Initialize() {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;

  bool found_valid_superpixel = false;

  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Initialized()) {
      found_valid_superpixel = true;

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
    }
  }

  return found_valid_superpixel;
}

bool StompMap::Initialize(StompVector& pix) {
  area_ = 0.0;
  size_ = 0;
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();
  min_weight_ = 1.0e30;
  max_weight_ = -1.0e30;

  if (sub_map_.empty() == false) {
    for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter)
      iter->Clear();
    sub_map_.clear();
  }

  sub_map_.reserve(Stomp::MaxSuperpixnum());

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    StompSubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
    unsigned long k = iter->Superpixnum();

    sub_map_[k].AddPixel(*iter);
  }

  bool found_valid_superpixel = false;

  for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter) {
    if (iter->Modified()) {
      found_valid_superpixel = true;

      iter->Resolve();

      area_ += iter->Area();
      size_ += iter->Size();
      if (min_resolution_ > iter->MinResolution())
        min_resolution_ = iter->MinResolution();
      if (max_resolution_ < iter->MaxResolution())
        max_resolution_ = iter->MaxResolution();
      if (iter->MinWeight() < min_weight_) min_weight_ = iter->MinWeight();
      if (iter->MaxWeight() > max_weight_) max_weight_ = iter->MaxWeight();
    }
  }

  return found_valid_superpixel;
}

void StompMap::Coverage(StompVector& superpix) {
  if (superpix.empty() == false) superpix.clear();

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    if (sub_map_[k].Initialized() == true) {
      StompPixel tmp_pix(Stomp::HPixResolution(),k,1.0);
      superpix.push_back(tmp_pix);
    }
  }
}

bool StompMap::FindLocation(AngularCoordinate& ang, double& weight) {
  bool keep = false;

  unsigned long k;
  StompPixel::Ang2Pix(Stomp::HPixResolution(),ang,k);

  if (sub_map_[k].Initialized()) keep = sub_map_[k].FindLocation(ang,weight);

  return keep;
}

double StompMap::FindUnmaskedFraction(StompPixel& pix) {
  double unmasked_fraction = 0.0;

  unsigned long k = pix.Superpixnum();

  if (sub_map_[k].Initialized())
    unmasked_fraction = sub_map_[k].FindUnmaskedFraction(pix);

  return unmasked_fraction;
}

void StompMap::FindUnmaskedFraction(StompVector& pix,
                                    std::vector<double>& unmasked_fraction) {

  if (unmasked_fraction.empty() == false) unmasked_fraction.clear();

  for (unsigned long i=0;i<pix.size();i++){
    double pixel_unmasked_fraction = 0.0;
    unsigned long k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_unmasked_fraction = sub_map_[k].FindUnmaskedFraction(pix[i]);

    unmasked_fraction.push_back(pixel_unmasked_fraction);
  }
}

double StompMap::FindAverageWeight(StompPixel& pix) {
  double weighted_average = 0.0;
  unsigned long k = pix.Superpixnum();

  if (sub_map_[k].Initialized())
    weighted_average = sub_map_[k].FindAverageWeight(pix);

  return weighted_average;
}

void StompMap::FindAverageWeight(StompVector& pix,
                                 std::vector<double>& weighted_average) {
  if (weighted_average.empty() == false) weighted_average.clear();

  for (unsigned long i=0;i<pix.size();i++) {
    double pixel_weighted_average = 0.0;
    unsigned long k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized())
      pixel_weighted_average = sub_map_[k].FindAverageWeight(pix[i]);

    weighted_average.push_back(pixel_weighted_average);
  }
}

void StompMap::FindMatchingPixels(StompPixel& pix,
				  StompVector& match_pix,
				  bool use_local_weights) {
  if (match_pix.empty() == false) match_pix.clear();

  unsigned long k = pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    StompVector tmp_pix;

    sub_map_[k].FindMatchingPixels(pix,tmp_pix,use_local_weights);

    if (tmp_pix.empty() == false)
      for (StompIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
        match_pix.push_back(*iter);
  }
}

void StompMap::FindMatchingPixels(StompVector& pix,
				  StompVector& match_pix,
				  bool use_local_weights) {
  if (match_pix.empty() == false) match_pix.clear();

  for (unsigned long i=0;i<pix.size();i++) {

    unsigned long k = pix[i].Superpixnum();

    if (sub_map_[k].Initialized()) {
      StompVector tmp_pix;

      sub_map_[k].FindMatchingPixels(pix[i],tmp_pix,use_local_weights);

      if (tmp_pix.empty() == false)
        for (StompIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
          match_pix.push_back(*iter);
    }
  }
}

void StompMap::GenerateRandomPoints(AngularVector& ang,
				    unsigned long n_point,
                                    bool use_weighted_sampling) {
  if (ang.empty() == false) ang.clear();
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

  StompVector superpix;

  Coverage(superpix);

  MTRand mtrand;

  mtrand.seed();

  for (unsigned long m=0;m<n_point;m++) {

    bool keep = false;
    double lambda, eta, z, weight, probability_limit;
    AngularCoordinate tmp_ang(0.0,0.0);
    unsigned long n,k;

    while (keep == false) {
      n = mtrand.randInt(superpix.size()-1);
      k = superpix[n].Superpixnum();

      z = sub_map_[k].ZMin() + mtrand.rand(sub_map_[k].ZMax() -
					   sub_map_[k].ZMin());
      lambda = asin(z)*Stomp::Rad2Deg();
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

bool StompMap::Write(std::string& OutputFile, bool hpixel_format,
                     bool weighted_map) {

  std::ofstream output_file(OutputFile.c_str());

  if (output_file.is_open()) {
    for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
      if (sub_map_[k].Initialized()) {
        StompVector pix;

        Pixels(pix,k);

        for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
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

double StompMap::AverageWeight() {
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

bool StompMap::IngestMap(StompVector& pix, bool destroy_copy) {

  for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
    unsigned long k = iter->Superpixnum();
    sub_map_[k].AddPixel(*iter);
  }

  if (destroy_copy) pix.clear();

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    if (sub_map_[k].Modified()) sub_map_[k].Resolve();

  return Initialize();
}

bool StompMap::IngestMap(StompMap& stomp_map, bool destroy_copy) {

  StompVector super_pix;

  stomp_map.Coverage(super_pix);

  for (StompIterator iter=super_pix.begin();iter!=super_pix.end();++iter) {
    StompVector tmp_pix;

    stomp_map.Pixels(tmp_pix,iter->Superpixnum());

    for (unsigned long i=0;i<tmp_pix.size();i++) {
      sub_map_[iter->Superpixnum()].AddPixel(tmp_pix[i]);
    }
  }

  if (destroy_copy) stomp_map.Clear();

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    if (sub_map_[k].Modified()) sub_map_[k].Resolve();

  return Initialize();
}

bool StompMap::IntersectMap(StompVector& pix) {
  StompMap stomp_map;

  stomp_map.Initialize(pix);

  return IntersectMap(stomp_map);
}

bool StompMap::IntersectMap(StompMap& stomp_map) {
  bool found_overlapping_area = false;

  unsigned long k = 0;
  while (k<Stomp::MaxSuperpixnum() && !found_overlapping_area) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      if (Area(k) < stomp_map.Area(k)) {
	StompVector tmp_pix;
        StompVector match_pix;

        sub_map_[k].Pixels(tmp_pix);

	stomp_map.FindMatchingPixels(tmp_pix,match_pix,false);

	if (match_pix.empty() == false) found_overlapping_area = true;
      } else {
	StompVector tmp_pix;
        StompVector match_pix;

        stomp_map.Pixels(tmp_pix,k);

	FindMatchingPixels(tmp_pix,match_pix,true);

	if (match_pix.empty() == false) found_overlapping_area = true;
      }
    }
    k++;
  }

  if (found_overlapping_area) {
    for (k=0;k<Stomp::MaxSuperpixnum();k++) {
      if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
        if (Area(k) < stomp_map.Area(k)) {
          StompVector tmp_pix;
          StompVector match_pix;

          sub_map_[k].Pixels(tmp_pix);

          stomp_map.FindMatchingPixels(tmp_pix,match_pix,false);

          sub_map_[k].Clear();

          if (match_pix.empty() == false) {
            found_overlapping_area = true;

            for (StompIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[k].AddPixel(*match_iter);
            }
            sub_map_[k].Resolve();

            match_pix.clear();
          }
        } else {
          StompVector tmp_pix;
          StompVector match_pix;

          stomp_map.Pixels(tmp_pix,k);

          FindMatchingPixels(tmp_pix,match_pix,true);

          sub_map_[k].Clear();

          if (match_pix.empty() == false) {
            found_overlapping_area = true;

            for (StompIterator match_iter=match_pix.begin();
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

bool StompMap::AddMap(StompVector& pix, bool drop_single) {

  StompMap stomp_map;
  stomp_map.Initialize(pix);

  return AddMap(stomp_map,drop_single);
}

bool StompMap::AddMap(StompMap& stomp_map, bool drop_single) {
  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      // Ok, we've got 2 maps in this superpixel, so we have to break
      // both down and calculate the overlap.

      StompVector added_pix;

      stomp_map.Pixels(added_pix,k);

      sub_map_[k].Add(added_pix,stomp_map.MaxResolution(k),drop_single);
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
          StompVector added_pix;

          stomp_map.Pixels(added_pix,k);

          for (StompIterator iter=added_pix.begin();
               iter!=added_pix.end();++iter) sub_map_[k].AddPixel(*iter);

          sub_map_[k].Resolve();
        }
      }
    }
  }

  return Initialize();
}

bool StompMap::MultiplyMap(StompVector& pix, bool drop_single) {

  StompMap stomp_map;
  stomp_map.Initialize(pix);

  return MultiplyMap(stomp_map,drop_single);
}

bool StompMap::MultiplyMap(StompMap& stomp_map, bool drop_single) {
  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      // Ok, we've got 2 maps in this superpixel, so we have to break
      // both down and calculate the overlap.

      StompVector multi_pix;

      stomp_map.Pixels(multi_pix,k);

      sub_map_[k].Multiply(multi_pix,stomp_map.MaxResolution(k),drop_single);
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
          StompVector multi_pix;

          stomp_map.Pixels(multi_pix,k);

          for (StompIterator iter=multi_pix.begin();
               iter!=multi_pix.end();++iter) sub_map_[k].AddPixel(*iter);

          sub_map_[k].Resolve();
        }
      }
    }
  }

  return Initialize();
}

bool StompMap::ExcludeMap(StompVector& pix, bool destroy_copy) {

  StompMap stomp_map;
  stomp_map.Initialize(pix);

  if (destroy_copy) pix.clear();

  return ExcludeMap(stomp_map,destroy_copy);
}

bool StompMap::ExcludeMap(StompMap& stomp_map, bool destroy_copy) {

  StompVector super_pix;

  stomp_map.Coverage(super_pix);

  for (StompIterator iter=super_pix.begin();iter!=super_pix.end();++iter) {
    unsigned long k = iter->Superpixnum();
    if (sub_map_[k].Initialized()) {
      StompVector exclude_pix;
      stomp_map.Pixels(exclude_pix,k);

      sub_map_[k].Exclude(exclude_pix,stomp_map.MaxResolution());
    }
  }

  if (destroy_copy) stomp_map.Clear();

  return Initialize();
}

bool StompMap::ImprintMap(StompVector& pix) {
  StompMap stomp_map;

  stomp_map.Initialize(pix);

  return ImprintMap(stomp_map);
}

bool StompMap::ImprintMap(StompMap& stomp_map) {
  bool found_overlapping_area = false;

  unsigned long k = 0;
  while (k<Stomp::MaxSuperpixnum() && !found_overlapping_area) {
    if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
      if (Area(k) < stomp_map.Area(k)) {
	StompVector tmp_pix;
        StompVector match_pix;

        sub_map_[k].Pixels(tmp_pix);

	stomp_map.FindMatchingPixels(tmp_pix,match_pix,false);

	if (match_pix.empty() == false) found_overlapping_area = true;
      } else {
	StompVector tmp_pix;
        StompVector match_pix;

        stomp_map.Pixels(tmp_pix,k);

	FindMatchingPixels(tmp_pix,match_pix,true);

	if (match_pix.empty() == false) found_overlapping_area = true;
      }
    }
    k++;
  }

  if (found_overlapping_area) {
    for (k=0;k<Stomp::MaxSuperpixnum();k++) {
      if (sub_map_[k].Initialized() && stomp_map.ContainsSuperpixel(k)) {
        if (Area(k) < stomp_map.Area(k)) {
          StompVector tmp_pix;
          StompVector match_pix;

          sub_map_[k].Pixels(tmp_pix);

          stomp_map.FindMatchingPixels(tmp_pix,match_pix,true);

          sub_map_[k].Clear();

          if (match_pix.empty() == false) {
            found_overlapping_area = true;

            for (StompIterator match_iter=match_pix.begin();
                 match_iter!=match_pix.end();++match_iter) {
              sub_map_[k].AddPixel(*match_iter);
            }
            sub_map_[k].Resolve();

            match_pix.clear();
          }
        } else {
          StompVector tmp_pix;
          StompVector match_pix;

          stomp_map.Pixels(tmp_pix,k);

          FindMatchingPixels(tmp_pix,match_pix,false);

          sub_map_[k].Clear();

          if (match_pix.empty() == false) {
            found_overlapping_area = true;

            for (StompIterator match_iter=match_pix.begin();
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

void StompMap::ScaleWeight(const double weight_scale) {
  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    if (sub_map_[k].Initialized()) sub_map_[k].ScaleWeight(weight_scale);
}

void StompMap::AddConstantWeight(const double add_weight) {
  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    if (sub_map_[k].Initialized()) sub_map_[k].AddConstantWeight(add_weight);
}

void StompMap::InvertWeight() {
  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    if (sub_map_[k].Initialized()) sub_map_[k].InvertWeight();
}

void StompMap::Pixels(StompVector& pix, unsigned long superpixnum) {
  if (pix.empty() == false) pix.clear();

  if (superpixnum < Stomp::MaxSuperpixnum()) {
    sub_map_[superpixnum].Pixels(pix);
  } else {
    for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
      if (sub_map_[k].Initialized()) {
        StompVector tmp_pix;

        sub_map_[k].Pixels(tmp_pix);

        for (StompIterator iter=tmp_pix.begin();iter!=tmp_pix.end();++iter)
          pix.push_back(*iter);
      }
    }
  }
}

void StompMap::Clear() {
  if (sub_map_.empty() == false) {
    for (SubMapIterator iter=sub_map_.begin();iter!=sub_map_.end();++iter)
      iter->Clear();
    sub_map_.clear();
  }
  area_ = 0.0;
  size_ = 0;
}

StompDensitySubMap::StompDensitySubMap(unsigned long superpixnum) {
  superpixnum_ = superpixnum;
  initialized_ = false;
  area_ = 0.0;
  total_density_ = 0.0;
}

StompDensitySubMap::~StompDensitySubMap() {
  superpixnum_ = Stomp::MaxSuperpixnum();
  initialized_ = false;
  area_ = 0.0;
  total_density_ = 0.0;
}

StompSection::StompSection() {
  stripe_min_ = stripe_max_ = -1;
}

StompSection::~StompSection() {
  stripe_min_ = stripe_max_ = -1;
}

StompDensityMap::StompDensityMap(StompMap& stomp_map, int input_resolution,
				 double min_unmasked_fraction,
                                 int region_resolution,
                                 bool use_map_weight_as_density) {
  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  region_resolution_ = region_resolution;

  StompVector superpix;
  stomp_map.Coverage(superpix);

  for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    StompVector sub_pix;
    iter->SubPix(resolution_,sub_pix);

    for (StompIterator sub_iter=sub_pix.begin();
         sub_iter!=sub_pix.end();++sub_iter) {
      double unmasked_fraction = stomp_map.FindUnmaskedFraction(*sub_iter);
      double initial_density = 0.0;
      if (unmasked_fraction > unmasked_fraction_minimum_) {
        if (use_map_weight_as_density)
          initial_density = stomp_map.FindAverageWeight(*sub_iter);
	StompDensityPixel tmp_pix(sub_iter->PixelX(),
				  sub_iter->PixelY(),
				  sub_iter->Resolution(),
				  unmasked_fraction,initial_density);
	pix_.push_back(tmp_pix);
      }
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(),pix_.end(),StompPixel::SuperPixelBasedOrder);
  mean_density_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_density_ = false;

  area_ = 0.0;
  total_density_ = 0.0;
  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter)
    area_ += iter->Area()*iter->Weight();

  InitializeSubMap();

  initialized_sub_map_ = true;
  initialized_region_map_ = false;
}

StompDensityMap::StompDensityMap(StompDensityMap& density_map,
				 int input_resolution,
				 double min_unmasked_fraction,
                                 int region_resolution,
                                 bool use_weighted_average_resampling) {
  if (input_resolution > density_map.Resolution()) {
    std::cout << "Cannot make higher resolution density map " <<
      "by resampling. Exiting.\n";
    exit(1);
  }

  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  region_resolution_ = region_resolution;

  if (density_map.Resolution() == resolution_) {
    pix_.reserve(density_map.Size());
    for (StompDensityIterator iter=density_map.Begin();
	 iter!=density_map.End();++iter) pix_.push_back(*iter);
  } else {
    StompVector superpix;
    density_map.Coverage(superpix);

    unsigned long x_min, x_max, y_min, y_max;
    StompDensityPixel tmp_pix;
    tmp_pix.SetResolution(resolution_);

    for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
      iter->SubPix(resolution_,x_min,x_max,y_min,y_max);
      for (unsigned long y=y_min;y<=y_max;y++) {
	for (unsigned long x=x_min;x<=x_max;x++) {
	  tmp_pix.SetPixnumFromXY(x,y);
	  density_map.Resample(tmp_pix,use_weighted_average_resampling);
	  if (tmp_pix.Weight() > unmasked_fraction_minimum_)
	    pix_.push_back(tmp_pix);
        }
      }
    }

    std::cout << "Resampled from " << density_map.Size() << " pixels to " <<
        pix_.size() << "\n";
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(),pix_.end(),StompPixel::SuperPixelBasedOrder);
  mean_density_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_density_ = false;

  area_ = 0.0;
  total_density_ = 0.0;
  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    area_ += iter->Area()*iter->Weight();
    total_density_ += iter->Density();
  }

  InitializeSubMap();

  initialized_region_map_ = false;
}

StompDensityMap::StompDensityMap(StompMap& stomp_map,
                                 AngularCoordinate& center, double theta_max,
                                 int input_resolution,
				 double min_unmasked_fraction,
				 double theta_min,
                                 int region_resolution) {

  resolution_ = input_resolution;
  unmasked_fraction_minimum_ = min_unmasked_fraction;
  region_resolution_ = region_resolution;

  StompDensityPixel tmp_pix(center,resolution_,0.0,0.0);

  StompVector pix;
  tmp_pix.WithinAnnulus(theta_min,theta_max,pix);

  for (StompIterator iter=pix.begin();iter!=pix.end();++iter) {
    double unmasked_fraction = stomp_map.FindUnmaskedFraction(*iter);
    if (unmasked_fraction > unmasked_fraction_minimum_) {
      tmp_pix.SetPixnumFromXY(iter->PixelX(), iter->PixelY());
      tmp_pix.SetWeight(unmasked_fraction);
      pix_.push_back(tmp_pix);
    }
  }

  pix_.resize(pix_.size());

  sort(pix_.begin(),pix_.end(),StompPixel::SuperPixelBasedOrder);
  mean_density_ = 0.0;
  converted_to_overdensity_ = false;
  calculated_mean_density_ = false;

  area_ = 0.0;
  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter)
    area_ += iter->Area()*iter->Weight();

  initialized_region_map_ = false;

  InitializeSubMap();
}

StompDensityMap::~StompDensityMap() {
  area_ = 0.0;
  mean_density_ = 0.0;
  total_density_ = 0.0;
  if (pix_.empty() == false) pix_.clear();
  if (sub_map_.empty() == false) sub_map_.clear();
  if (region_map_.empty() == false) region_map_.clear();
}

void StompDensityMap::InitializeSubMap() {
  if (sub_map_.empty() == false) sub_map_.clear();

  sub_map_.reserve(Stomp::MaxSuperpixnum());

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    StompDensitySubMap tmp_sub_map(k);
    sub_map_.push_back(tmp_sub_map);
  }

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++)
    sub_map_[k].SetNull(pix_.end());

  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    unsigned long k = iter->Superpixnum();
    sub_map_[k].AddToArea(iter->Resolution(),iter->Weight());
    sub_map_[k].AddToDensity(iter->Density());
    if (sub_map_[k].Initialized() == false) {
      sub_map_[k].SetBegin(iter);
    } else {
      sub_map_[k].SetEnd(iter);
    }
  }
}

void StompDensityMap::InitializeRegions(int n_region) {
  if (region_resolution_ > resolution_) {
    std::cout << "Attempted to make region map at higher " <<
      "resolution than density map.\nThis is not possible.\n";
    exit(1);
  }

  if (region_resolution_ > 256) {
    std::cout <<
      "WARNING: Attempting to generate region map with resolution " <<
      "above 256!\n";
    std::cout << "This may end badly.\n";
    if (region_resolution_ > 2048) {
      std::cout <<
	"Ok, the resolution is above 2048.  Can't do this.  Try again\n";
      exit(1);
    }
  }

  std::cout << "Generating regionation map at " << region_resolution_ << "\n";

  StompVector region_pix;

  if (region_resolution_ == resolution_) {
    region_pix.reserve(pix_.size());
    for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      StompPixel tmp_pix(iter->PixelX(),iter->PixelY(),iter->Resolution(),
                     iter->Weight());
      region_pix.push_back(tmp_pix);
    }
  } else {
    double resampled_unmasked_fraction_minimum = unmasked_fraction_minimum_;
    resampled_unmasked_fraction_minimum *=
        region_resolution_*region_resolution_/(resolution_*resolution_);

    StompVector superpix;
    Coverage(superpix);

    for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
      StompVector sub_pix;
      iter->SubPix(region_resolution_,sub_pix);

      for (StompIterator sub_iter=sub_pix.begin();
	   sub_iter!=sub_pix.end();++sub_iter) {
        double unmasked_fraction = FindUnmaskedFraction(*sub_iter);
        if (unmasked_fraction > resampled_unmasked_fraction_minimum) {
          StompPixel tmp_pix(sub_iter->PixelX(),
                             sub_iter->PixelY(),
                             sub_iter->Resolution(),
                             unmasked_fraction);
          region_pix.push_back(tmp_pix);
        }
      }
    }
  }

  sort(region_pix.begin(),region_pix.end(),StompPixel::SuperPixelBasedOrder);

  if (static_cast<unsigned int>(n_region) > region_pix.size()) {
    std::cout << "WARNING: Exceeded maximum possible regions.  Setting to " <<
      region_pix.size() << " regions.\n";
    n_region_ = region_pix.size();
  } else {
    n_region_ = n_region;
  }

  if (static_cast<unsigned int>(n_region_) == region_pix.size()) {
    unsigned long i=0;
    for (StompIterator iter=region_pix.begin();
	 iter!=region_pix.end();++iter) {
      region_map_[iter->Pixnum()] = i;
      i++;
    }
    std::cout << "\tNumber of regions matches number of regionation pixels.\n";
    std::cout << "\tThis will be dead easy, " <<
      "but won't guarantee an equal area solution...\n";
  } else {
    std::cout << "\tBreaking up " << area_ << " square degrees into " <<
      n_region_ << " equal-area pieces.\n";
    std::cout << "\tWARNING: May not work well for areas " <<
      "less than 100 square degrees.\n";

    std::vector<int> tmp_stripe;
    tmp_stripe.reserve(region_pix.size());
    for (StompIterator iter=region_pix.begin();
	 iter!=region_pix.end();++iter) {
      tmp_stripe.push_back(iter->Stripe(region_resolution_));
    }

    sort(tmp_stripe.begin(),tmp_stripe.end());
    std::vector<int> stripe;
    stripe.push_back(tmp_stripe[0]);
    for (unsigned long i=1;i<tmp_stripe.size();i++)
      if (tmp_stripe[i] != tmp_stripe[i-1]) stripe.push_back(tmp_stripe[i]);

    tmp_stripe.clear();

    sort(stripe.begin(),stripe.end());

    std::cout << "Stripes:\n";
    for (unsigned long i=0;i<stripe.size();i++)
      std::cout << stripe[i] << " ";
    std::cout << "\n";

    std::vector<StompSection> super_section;

    StompSection tmp_section;
    tmp_section.SetMinStripe(stripe[0]);
    tmp_section.SetMaxStripe(stripe[0]);

    super_section.push_back(tmp_section);

    for (unsigned int i=1,j=0;i<stripe.size();i++) {
      if (stripe[i] == stripe[i-1] + 1) {
        super_section[j].SetMaxStripe(stripe[i]);
      } else {
        tmp_section.SetMinStripe(stripe[i]);
        tmp_section.SetMaxStripe(stripe[i]);
        super_section.push_back(tmp_section);
        j++;
      }
    }

    double region_length = sqrt(area_/n_region_);
    int region_width =
        static_cast<int>(region_length*Stomp::Nx0()*region_resolution_/360.0);
    if (region_width == 0) region_width = 1;

    std::vector<StompSection> section;

    int j = -1;
    for (std::vector<StompSection>::iterator iter=super_section.begin();
         iter!=super_section.end();++iter) {

      for (int stripe_iter=iter->MinStripe(),section_iter=region_width;
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
    double unit_area = StompPixel::PixelArea(region_resolution_);
    unsigned long n_pixel = 0;
    int region_iter = 0;
    double mean_area = area_/region_pix.size();
    double area_break = area_/n_region_;

    for (unsigned int i=0;i<section.size();i++)
      std::cout << "Section " << i << ": " << section[i].MinStripe() <<
	" - " << section[i].MaxStripe() << "\n";

    std::cout << "Assigning areas...\n\n";
    std::cout << "Sample  Pixels  Unmasked Area  Masked Area\n";
    std::cout << "------  ------  -------------  -----------\n";

    for (std::vector<StompSection>::iterator section_iter=section.begin();
         section_iter!=section.end();++section_iter) {

      for (StompIterator iter=region_pix.begin();
	   iter!=region_pix.end();++iter) {
        if ((iter->Stripe(region_resolution_) >=
	     section_iter->MinStripe()) &&
            (iter->Stripe(region_resolution_) <=
	     section_iter->MaxStripe())) {
          if ((region_area + 0.75*mean_area < area_break*(region_iter+1)) ||
	      (region_iter == n_region_-1)) {
            region_area += iter->Weight()*unit_area;
            region_map_[iter->Pixnum()] = region_iter;
            running_area += iter->Weight()*unit_area;
            n_pixel++;
          } else {
	    std::cout << region_iter << "\t" << n_pixel << "\t" <<
                n_pixel*unit_area << "\t\t" << running_area << "\n";
            region_iter++;
            region_area += iter->Weight()*unit_area;
            region_map_[iter->Pixnum()] = region_iter;
            running_area = iter->Weight()*unit_area;
            n_pixel = 1;
          }
        }
      }
    }
    std::cout << region_iter << "\t" << n_pixel << "\t" <<
      n_pixel*unit_area << "\t\t" << running_area << "\n";
  }

  std::vector<unsigned long> region_count_check;

  std::cout << "Final pixel count check:\n";

  for (int i=0;i<n_region_;i++) region_count_check.push_back(0);

  for (RegionMap::iterator iter=region_map_.begin();
       iter!=region_map_.end();++iter) {
    if (iter->second < n_region_) {
      region_count_check[iter->second]++;
    } else {
      std::cout << "Encountered illegal region index: " <<
	iter->second << "\nBailing...\n";
    }
  }

  for (int i=0;i<n_region_;i++)
    std::cout << "\t" << i << ": " << region_count_check[i] << " pixels\n";

  initialized_region_map_ = true;
}

bool StompDensityMap::AddToMap(AngularCoordinate& ang, double object_weight) {
  StompDensityPixel tmp_pix(ang,resolution_,object_weight);

  unsigned long k = tmp_pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    StompDensityPair iter;
    iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
		       StompPixel::SuperPixelBasedOrder);
    if (iter.first != iter.second) {
      iter.first->AddToDensity(object_weight);
      sub_map_[k].AddToDensity(object_weight);
      total_density_ += object_weight;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool StompDensityMap::AddToMap(WeightedAngularCoordinate& ang) {
  StompDensityPixel tmp_pix(ang,resolution_,ang.Weight());

  unsigned long k = tmp_pix.Superpixnum();

  if (sub_map_[k].Initialized()) {
    StompDensityPair iter;
    iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
		       StompPixel::SuperPixelBasedOrder);
    if (iter.first != iter.second) {
      iter.first->AddToDensity(ang.Weight());
      sub_map_[k].AddToDensity(ang.Weight());
      total_density_ += ang.Weight();
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void StompDensityMap::Coverage(StompVector& superpix) {
  if (superpix.empty() == false) superpix.clear();

  for (unsigned long k=0;k<Stomp::MaxSuperpixnum();k++) {
    if (sub_map_[k].Initialized() == true) {
      StompPixel tmp_pix(Stomp::HPixResolution(),k,1.0);
      superpix.push_back(tmp_pix);
    }
  }
}

double StompDensityMap::FindLocalDensity(AngularCoordinate& ang,
                                         double theta_max, double theta_min) {
  StompPixel center_pix(ang,resolution_,0.0);

  unsigned long y_min, y_max;
  std::vector<unsigned long> x_min, x_max;

  center_pix.XYBounds(theta_max,x_min,x_max,y_min,y_max,true);

  double sinthetamax =
    sin(theta_max*Stomp::Deg2Rad())*sin(theta_max*Stomp::Deg2Rad());
  double sinthetamin =
    sin(theta_min*Stomp::Deg2Rad())*sin(theta_min*Stomp::Deg2Rad());
  if (theta_min < 1.0e-10) sinthetamin = -1.0;

  double costheta = 0.0;
  StompDensityIterator iter;
  double total_area = 0.0, total_density = 0.0;

  for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
    StompPixel tmp_left(x_min[n],y,resolution_);
    StompPixel tmp_right(x_max[n],y,resolution_);
    unsigned long k = tmp_left.Superpixnum();

    if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

      // First, we iterate our way through the superpixels that don't contain
      // the end pixel.  We can't use strict ordering here to bound our
      // our iteration since the ordering breaks at the superpixel boundaries.
      // Likewise, we can't assume that tmp_left < tmp_right since they may
      // be across the x = nx-1,0 boundary.  So, things are a little kludgy.

      while (k != tmp_right.Superpixnum()) {
	if (sub_map_[k].Initialized()) {
	  if (sub_map_[k].Begin()->PixelY() <= y) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
			       StompPixel::SuperPixelBasedOrder);
	    while (iter->PixelY() < y) ++iter;
	    while ((iter->PixelY() == y) && (iter != sub_map_[k].End())) {
	      costheta =
                ang.UnitSphereX()*iter->UnitSphereX() +
                ang.UnitSphereY()*iter->UnitSphereY() +
                ang.UnitSphereZ()*iter->UnitSphereZ();
	      if ((1.0 - costheta*costheta < sinthetamax) &&
		  (1.0 - costheta*costheta > sinthetamin)) {
		total_area += iter->Weight();
		total_density += iter->Density();
	      }
	      ++iter;
	    }
	  }
	  tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	  tmp_left.Iterate();
	  k = tmp_left.Superpixnum();
	} else {
          while ((sub_map_[k].Initialized() == false) &&
                 (k != tmp_right.Superpixnum())) {
	    tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
            tmp_left.Iterate();
            k = tmp_left.Superpixnum();
          }
        }
      }
    }

    // Ok, now we know that tmp_left and tmp_right are in the same superpixel
    // so we can use strict order to make sure that we don't go past tmp_right.

    if (sub_map_[k].Initialized()) {
      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
                         StompPixel::SuperPixelBasedOrder);
      while (StompPixel::SuperPixelBasedOrder(*iter,tmp_left)) ++iter;

      while (iter != sub_map_[k].End()) {
	if (StompPixel::SuperPixelBasedOrder(*iter,tmp_right)) {
	  costheta =
	    ang.UnitSphereX()*iter->UnitSphereX() +
	    ang.UnitSphereY()*iter->UnitSphereY() +
	    ang.UnitSphereZ()*iter->UnitSphereZ();
	  if ((1.0 - costheta*costheta < sinthetamax) &&
	      (1.0 - costheta*costheta > sinthetamin)) {
	    total_area += iter->Weight();
	    total_density += iter->Density();
	  }
	  ++iter;
	} else {
	  iter = sub_map_[k].End();
	}
      }
    }
  }

  if (converted_to_overdensity_) {
    total_density = total_density*mean_density_ + mean_density_;
  }

  if (total_area > 0.0000001) {
    return total_density/(total_area*center_pix.Area());
  } else {
    return 0.0;
  }
}

double StompDensityMap::FindLocalArea(AngularCoordinate& ang,
                                      double theta_max, double theta_min) {
  StompPixel center_pix(ang,resolution_);

  unsigned long y_min, y_max;
  std::vector<unsigned long> x_min, x_max;

  center_pix.XYBounds(theta_max,x_min,x_max,y_min,y_max,true);

  double sinthetamax =
    sin(theta_max*Stomp::Deg2Rad())*sin(theta_max*Stomp::Deg2Rad());
  double sinthetamin =
    sin(theta_min*Stomp::Deg2Rad())*sin(theta_min*Stomp::Deg2Rad());
  if (theta_min < 1.0e-10) sinthetamin = -1.0;

  double costheta = 0.0;
  StompDensityIterator iter;
  double total_area = 0.0;

  for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
    StompPixel tmp_left(x_min[n],y,resolution_);
    StompPixel tmp_right(x_max[n],y,resolution_);
    unsigned long k = tmp_left.Superpixnum();

    if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {
      // First, we iterate our way through the superpixels that don't contain
      // the end pixel.  We can't use strict ordering here to bound our
      // our iteration since the ordering breaks at the superpixel boundaries.
      // Likewise, we can't assume that tmp_left < tmp_right since they may
      // be across the x = nx-1,0 boundary.  So, things are a little kludgy.

      while (k != tmp_right.Superpixnum()) {
	if (sub_map_[k].Initialized()) {
	  if (sub_map_[k].Begin()->PixelY() <= y) {
	    iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
			       StompPixel::SuperPixelBasedOrder);
	    while (iter->PixelY() < y) ++iter;
	    while ((iter->PixelY() == y) && (iter != sub_map_[k].End())) {
	      costheta =
                ang.UnitSphereX()*iter->UnitSphereX() +
                ang.UnitSphereY()*iter->UnitSphereY() +
                ang.UnitSphereZ()*iter->UnitSphereZ();
	      if ((1.0 - costheta*costheta < sinthetamax) &&
		  (1.0 - costheta*costheta > sinthetamin))
		total_area += iter->Weight();
	      ++iter;
	    }
	  }
	  tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
	  tmp_left.Iterate();
	  k = tmp_left.Superpixnum();
	} else {
          while ((sub_map_[k].Initialized() == false) &&
                 (k != tmp_right.Superpixnum())) {
	    tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
            tmp_left.Iterate();
            k = tmp_left.Superpixnum();
          }
        }
      }
    }

    // Ok, now we know that tmp_left and tmp_right are in the same superpixel
    // so we can use strict order to make sure that we don't go past tmp_right.

    if (sub_map_[k].Initialized()) {
      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
                         StompPixel::SuperPixelBasedOrder);
      while (iter->PixelY() < y) ++iter;

      while (iter != sub_map_[k].End()) {
	if (StompPixel::SuperPixelBasedOrder(*iter,tmp_right)) {
	  costheta =
	    ang.UnitSphereX()*iter->UnitSphereX() +
	    ang.UnitSphereY()*iter->UnitSphereY() +
	    ang.UnitSphereZ()*iter->UnitSphereZ();
	  if ((1.0 - costheta*costheta < sinthetamax) &&
	      (1.0 - costheta*costheta > sinthetamin))
	    total_area += iter->Weight();
	  ++iter;
	} else {
	  iter = sub_map_[k].End();
	}
      }
    }
  }

  return total_area*center_pix.Area();
}

double StompDensityMap::FindUnmaskedFraction(StompPixel& pix) {

  double unmasked_fraction = 0.0;

  if (pix.Resolution() > resolution_) {
    unmasked_fraction = -1.0;
  } else {
    unsigned long k = pix.Superpixnum();

    if (sub_map_[k].Initialized()) {


      if (pix.Resolution() == resolution_) {
        StompDensityPixel tmp_pix(pix.PixelX(),pix.PixelY(),pix.Resolution());

        StompDensityPair iter = equal_range(sub_map_[k].Begin(),
					    sub_map_[k].End(),tmp_pix,
					    StompPixel::SuperPixelBasedOrder);
        if (iter.first != iter.second)
          unmasked_fraction = iter.first->Weight();
      } else {
        unsigned long y_min, y_max, x_min, x_max;
        double pixel_fraction =
            1.0*pix.Resolution()*pix.Resolution()/(resolution_*resolution_);

        pix.SubPix(resolution_,x_min,x_max,y_min,y_max);

        for (unsigned long y=y_min;y<=y_max;y++) {
          StompPixel tmp_left(x_min,y,resolution_);
          StompPixel tmp_right(x_max,y,resolution_);

          // Ok, we know that tmp_left and tmp_right are in the
          // same superpixel so we can use strict order to make sure
          // that we don't go past tmp_right.

          if (tmp_right.HPixnum() >= sub_map_[k].Begin()->HPixnum()) {
            while (tmp_left.HPixnum() < sub_map_[k].Begin()->HPixnum())
              tmp_left.Iterate();

	    StompDensityIterator iter =
	      lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
			  StompPixel::SuperPixelBasedOrder);
            if (iter != sub_map_[k].End()) {
              while (iter->PixelY() < y) ++iter;

              while (iter != sub_map_[k].End()) {
                if (iter->HPixnum() <= tmp_right.HPixnum()) {
                  unmasked_fraction += pixel_fraction*iter->Weight();
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

  return unmasked_fraction;
}

double StompDensityMap::FindDensity(StompPixel& pix) {

  double unmasked_fraction = 0.0, total_density = 0.0;

  if (pix.Resolution() > resolution_) {
    unmasked_fraction = -1.0;
    total_density = -1.0;
  } else {
    unsigned long k = pix.Superpixnum();

    if (sub_map_[k].Initialized()) {

      if (pix.Resolution() == resolution_) {
        StompDensityPixel tmp_pix(pix.PixelX(),pix.PixelY(),pix.Resolution());

        StompDensityPair iter = equal_range(sub_map_[k].Begin(),
					    sub_map_[k].End(),tmp_pix,
					    StompPixel::SuperPixelBasedOrder);
        if (iter.first != iter.second) {
          unmasked_fraction = iter.first->Weight();
	  total_density = iter.first->Density();
        }
      } else {
        unsigned long y_min, y_max, x_min, x_max;
        double pixel_fraction =
            1.0*pix.Resolution()*pix.Resolution()/(resolution_*resolution_);

        pix.SubPix(resolution_,x_min,x_max,y_min,y_max);

        for (unsigned long y=y_min;y<=y_max;y++) {
          StompPixel tmp_left(x_min,y,resolution_);
          StompPixel tmp_right(x_max,y,resolution_);

          // Ok, we know that tmp_left and tmp_right are in the
          // same superpixel so we can use strict order to make sure
          // that we don't go past tmp_right.

          if (tmp_right.HPixnum() >= sub_map_[k].Begin()->HPixnum()) {
            while (tmp_left.HPixnum() < sub_map_[k].Begin()->HPixnum())
              tmp_left.Iterate();

            StompDensityIterator iter =
	      lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),tmp_left,
			  StompPixel::SuperPixelBasedOrder);
            if (iter != sub_map_[k].End()) {
              while (iter->PixelY() < y) ++iter;

              while (iter != sub_map_[k].End()) {
                if (iter->HPixnum() <= tmp_right.HPixnum()) {
                  unmasked_fraction += pixel_fraction*iter->Weight();
		  total_density += iter->Density();
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

  if (converted_to_overdensity_) {
    total_density = total_density*mean_density_ + mean_density_;
  }

  if (unmasked_fraction > 0.0000001) {
    return total_density/(unmasked_fraction*pix.Area());
  } else {
    if (total_density > -0.00000001) {
      return 0.0;
    } else {
      return -1.0;
    }
  }
}

void StompDensityMap::Resample(StompDensityPixel& pix,
                               bool use_weighted_average) {

  double unmasked_fraction = 0.0, total_density = 0.0, weighted_density = 0.0;

  if (pix.Resolution() > resolution_) {
    unmasked_fraction = -1.0;
    total_density = -1.0;
  } else {
    unsigned long k = pix.Superpixnum();

    if (sub_map_[k].Initialized()) {

      if (pix.Resolution() == resolution_) {
        StompDensityPixel tmp_pix(pix.PixelX(),pix.PixelY(),pix.Resolution());
        StompDensityPair iter = equal_range(sub_map_[k].Begin(),
					    sub_map_[k].End(),tmp_pix,
					    StompPixel::SuperPixelBasedOrder);
        if (iter.first != iter.second) {
          unmasked_fraction = iter.first->Weight();
	  total_density = iter.first->Density();
        }
      } else {
        unsigned long y_min, y_max, x_min, x_max;
        double pixel_fraction =
            1.0*pix.Resolution()*pix.Resolution()/(resolution_*resolution_);
        double super_density = 0.0;
        StompPixel tmp_pix;
        tmp_pix.SetResolution(resolution_);
        StompDensityPair iter;

        pix.SubPix(resolution_,x_min,x_max,y_min,y_max);

        for (unsigned long y=y_min;y<=y_max;y++) {
          for (unsigned long x=x_min;x<=x_max;x++) {
            tmp_pix.SetPixnumFromXY(x,y);
            iter = equal_range(sub_map_[k].Begin(),sub_map_[k].End(),tmp_pix,
                               StompPixel::SuperPixelBasedOrder);
            if (iter.first != iter.second) {
              unmasked_fraction += pixel_fraction*iter.first->Weight();
              super_density = iter.first->Density();
              if (converted_to_overdensity_)
                super_density =
                    mean_density_*iter.first->Area()*
                    iter.first->Weight()*iter.first->Density() +
                    mean_density_*iter.first->Area()*iter.first->Weight();
              total_density += super_density;
              weighted_density +=
                  super_density*pixel_fraction*iter.first->Weight();
            }
          }
        }
      }
    }
  }

  if (use_weighted_average) {
    if (unmasked_fraction > 0.000000001) {
      weighted_density /= unmasked_fraction;
    } else {
      weighted_density = 0.0;
    }
  }

  pix.SetWeight(unmasked_fraction);
  if (use_weighted_average) {
    pix.SetDensity(weighted_density);
  } else {
    pix.SetDensity(total_density);
  }
}

void StompDensityMap::CalculateMeanDensity() {
  double sum_pixel = 0.0;
  mean_density_ = 0.0;

  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter) {
    mean_density_ += iter->Density()/(iter->Area()*iter->Weight());
    sum_pixel += 1.0;
  }

  mean_density_ /= sum_pixel;
  calculated_mean_density_ = true;
}

void StompDensityMap::ConvertToOverDensity() {
  if (calculated_mean_density_ == false) CalculateMeanDensity();

  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter)
    iter->ConvertToOverDensity(mean_density_);

  converted_to_overdensity_ = true;
}

bool StompDensityMap::ImprintMap(StompMap& stomp_map) {
  StompVector pixVec;

  pixVec.reserve(pix_.size());

  for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter)
    pixVec.push_back(StompPixel(iter->PixelX(),iter->PixelY(),
                                iter->Resolution(),iter->Density()));

  return stomp_map.ImprintMap(pixVec);
}

bool StompDensityMap::Write(std::string& OutputFile, bool hpixel_format,
                            bool weighted_map) {

  std::ofstream output_file(OutputFile.c_str());

  if (output_file.is_open()) {
    for (StompDensityIterator iter=pix_.begin();iter!=pix_.end();++iter) {
      if (hpixel_format) {
        if (weighted_map) {
          output_file <<
	    iter->HPixnum() << " " <<
	    iter->Superpixnum() << " " <<
	    iter->Resolution() << " " <<
	    iter->Density() << " " <<
	    iter->Weight() << "\n";
        } else {
          output_file <<
	    iter->HPixnum() << " " <<
	    iter->Superpixnum() << " " <<
	    iter->Resolution() << " " <<
	    iter->Density() << "\n";
        }
      } else {
        if (weighted_map) {
          output_file <<
	    iter->Pixnum() << " " <<
	    iter->Resolution() << " " <<
	    iter->Density() << " " <<
	    iter->Weight() << "\n";
        } else {
          output_file <<
	    iter->Pixnum() << " " <<
	    iter->Resolution() << " " <<
	    iter->Density() << "\n";
        }
      }
    }

    output_file.close();

    return true;
  } else {
    return false;
  }
}

void StompDensityMap::AutoCorrelate(AngularCorrelation& wtheta) {
  double mean_delta = 0.0, mean_weighted_delta = 0.0, sum_weight = 0.0;
  double min_weight = 2.0, max_weight = 0.0;

  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    std::cout << "\tFound " << theta_end - theta_begin <<
      " angular bins for this density map...\n";
    if (calculated_mean_density_ == false) {
      std::cout << "\tFinding mean density... " << MeanDensity() << "\n";
    } else {
      std::cout << "\tUsing a mean density of " <<
          mean_density_ << " objects/sq. degree...\n";
    }
    if (converted_to_overdensity_ == false) {
      std::cout << "\tConverting densities to fractional over-densities...\n";
      ConvertToOverDensity();
    }

    std::cout << "\t\tBeginning auto-correlation with " << pix_.size() <<
      " pixels...\n";

    double theta = wtheta.ThetaMax(resolution_);
    double sinthetamin = wtheta.SinThetaMin(resolution_);
    double sinthetamax = wtheta.SinThetaMax(resolution_);

    unsigned long y_min, y_max, k;
    std::vector<unsigned long> x_min, x_max;
    double costheta = 0.0, sintheta = 0.0;
    StompDensityIterator iter;
    ThetaIterator theta_iter;
    StompDensityPixel tmp_left, tmp_right;
    tmp_left.SetResolution(resolution_);
    tmp_right.SetResolution(resolution_);

    for (StompDensityIterator map_iter=pix_.begin();
	 map_iter!=pix_.end();++map_iter) {

      mean_delta += map_iter->Density();
      mean_weighted_delta += map_iter->Density()*map_iter->Weight();
      sum_weight += map_iter->Weight();
      if (map_iter->Weight() < min_weight) min_weight = map_iter->Weight();
      if (map_iter->Weight() > max_weight) max_weight = map_iter->Weight();

      map_iter->XYBounds(theta,x_min,x_max,y_min,y_max,true);

      for (unsigned long y=y_min,n=0;y<=y_max;y++,n++) {
	tmp_left.SetPixnumFromXY(x_min[n],y);
	tmp_right.SetPixnumFromXY(x_max[n],y);
	k = tmp_left.Superpixnum();

	if (StompPixel::SuperPixelBasedOrder(*map_iter,tmp_right)) {
	  if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

	    // This is the same schema for iterating through the bounding
	    // pixels as in FindLocalArea and FindLocalDensity.

	    while (k != tmp_right.Superpixnum()) {
	      if (sub_map_[k].Initialized()) {
		if (sub_map_[k].Begin()->PixelY() <= y) {
		  iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
				     tmp_left,
                                     StompPixel::SuperPixelBasedOrder);
		  while ((iter->PixelY() == y) &&
			 (iter != sub_map_[k].End())) {
		    if (StompPixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		      costheta =
			map_iter->UnitSphereX()*iter->UnitSphereX() +
			map_iter->UnitSphereY()*iter->UnitSphereY() +
			map_iter->UnitSphereZ()*iter->UnitSphereZ();
		      sintheta = 1.0 - costheta*costheta;
		      if ((sintheta < sinthetamax) &&
			  (sintheta > sinthetamin)) {
			theta_iter = wtheta.Find(theta_begin,theta_end,
						 sintheta);
			theta_iter->AddToWtheta(map_iter->Density()*
						map_iter->Weight()*
						iter->Density()*
						iter->Weight());
			theta_iter->AddToCounter(map_iter->Weight()*
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
		while ((sub_map_[k].Initialized() == false) &&
		       (k != tmp_right.Superpixnum())) {
		  tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
		  tmp_left.Iterate();
		  k = tmp_left.Superpixnum();
		}
	      }
	    }
	  }

	  if (sub_map_[k].Initialized()) {
	    if (StompPixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
						 tmp_left) ||
		StompPixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
                                 tmp_left,
				 StompPixel::SuperPixelBasedOrder);
	      while (iter != sub_map_[k].End()) {
		if (StompPixel::SuperPixelBasedOrder(*iter,tmp_right)) {
		  if (StompPixel::SuperPixelBasedOrder(*map_iter,*iter)) {
		    costheta =
		      map_iter->UnitSphereX()*iter->UnitSphereX() +
		      map_iter->UnitSphereY()*iter->UnitSphereY() +
		      map_iter->UnitSphereZ()*iter->UnitSphereZ();
		    sintheta = 1.0 - costheta*costheta;
		    if ((sintheta < sinthetamax) &&
			(sintheta > sinthetamin)) {
		      theta_iter = wtheta.Find(theta_begin,theta_end,
					       sintheta);
		      theta_iter->AddToWtheta(map_iter->Density()*
					      map_iter->Weight()*
					      iter->Density()*iter->Weight());
		      theta_iter->AddToCounter(map_iter->Weight()*
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
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }

  std::cout << "\t\t\tDone. Average over-density: " <<
    mean_delta/pix_.size() <<
    " (" << mean_weighted_delta/sum_weight << ") for " <<
    sum_weight*pix_[0].Area() << " sq. degrees...\n";
}

void StompDensityMap::CrossCorrelate(StompDensityMap& density_map,
                                     AngularCorrelation& wtheta) {
  double mean_delta = 0.0, mean_weighted_delta = 0.0, sum_weight = 0.0;
  double min_weight = 2.0, max_weight = 0.0;

  if (resolution_ != density_map.Resolution()) {
    std::cout << "Map resolutions must match!  Exiting...\n";
    exit(1);
  }

  ThetaIterator theta_begin = wtheta.Begin(resolution_);
  ThetaIterator theta_end = wtheta.End(resolution_);

  if (theta_begin != theta_end) {
    std::cout << "\tFound " << theta_end - theta_begin <<
      " angular bins for this density map...\n";
    if (calculated_mean_density_ == false) {
      std::cout << "\tFinding mean density...\n";
      CalculateMeanDensity();
    } else {
      std::cout << "\tUsing a mean density of " <<
          mean_density_ << " objects/sq. degree...\n";
    }
    if (converted_to_overdensity_ == false) {
      std::cout << "\tConverting densities to fractional over-densities...\n";
      ConvertToOverDensity();
    }
    if (density_map.IsOverDensityMap() == false) {
      std::cout << "\tConverting cross-correlation map" <<
          " to fractional over-densities...\n";
      density_map.ConvertToOverDensity();
    }

    std::cout << "\t\tBeginning cross-correlation with " <<
        density_map.Size() << " x " << pix_.size() << " pixels...\n";

    double theta = wtheta.ThetaMax(resolution_);
    double sinthetamin = wtheta.SinThetaMin(resolution_);
    double sinthetamax = wtheta.SinThetaMax(resolution_);

    unsigned long y_min, y_max, x_min, x_max, k;
    double costheta = 0.0, sintheta = 0.0;
    StompDensityIterator iter;
    ThetaIterator theta_iter;
    StompDensityPixel tmp_left, tmp_right;
    tmp_left.SetResolution(resolution_);
    tmp_right.SetResolution(resolution_);

    for (StompDensityIterator map_iter=density_map.Begin();
	 map_iter!=density_map.End();++map_iter) {

      mean_delta += map_iter->Density();
      mean_weighted_delta += map_iter->Density()*map_iter->Weight();
      sum_weight += map_iter->Weight();
      if (map_iter->Weight() < min_weight) min_weight = map_iter->Weight();
      if (map_iter->Weight() > max_weight) max_weight = map_iter->Weight();

      map_iter->XYBounds(theta,x_min,x_max,y_min,y_max,true);

      for (unsigned long y=y_min;y<=y_max;y++) {
	tmp_left.SetPixnumFromXY(x_min,y);
	tmp_right.SetPixnumFromXY(x_max,y);
	k = tmp_left.Superpixnum();

        if (tmp_left.Superpixnum() != tmp_right.Superpixnum()) {

          // This is the same schema for iterating through the bounding
          // pixels as in FindLocalArea and FindLocalDensity.

          while (k != tmp_right.Superpixnum()) {
            if (sub_map_[k].Initialized()) {
              if (sub_map_[k].Begin()->PixelY() <= y) {
                iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
                                   tmp_left,
                                   StompPixel::SuperPixelBasedOrder);
                while ((iter->PixelY() == y) &&
                       (iter != sub_map_[k].End())) {
                  costheta =
                      map_iter->UnitSphereX()*iter->UnitSphereX() +
                      map_iter->UnitSphereY()*iter->UnitSphereY() +
                      map_iter->UnitSphereZ()*iter->UnitSphereZ();
                  sintheta = 1.0 - costheta*costheta;
                  if ((sintheta < sinthetamax) &&
                      (sintheta > sinthetamin)) {
                    theta_iter = wtheta.Find(theta_begin,theta_end,
                                             sintheta);
                    theta_iter->AddToWtheta(map_iter->Density()*
                                            map_iter->Weight()*
                                            iter->Density()*
                                            iter->Weight());
                    theta_iter->AddToCounter(map_iter->Weight()*
                                             iter->Weight());
                  }
                  ++iter;
                }
              }
              tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
              tmp_left.Iterate();
              k = tmp_left.Superpixnum();
            } else {
              while ((sub_map_[k].Initialized() == false) &&
                     (k != tmp_right.Superpixnum())) {
                tmp_left.SetPixnumFromXY(tmp_left.PixelX1()-1,y);
                tmp_left.Iterate();
                k = tmp_left.Superpixnum();
              }
            }
	  }

	  if (sub_map_[k].Initialized()) {
	    if (StompPixel::SuperPixelBasedOrder(*sub_map_[k].Begin(),
						 tmp_left) ||
		StompPixel::PixelMatch(*sub_map_[k].Begin(),tmp_left)) {
	      iter = lower_bound(sub_map_[k].Begin(),sub_map_[k].End(),
                                 tmp_left,
				 StompPixel::SuperPixelBasedOrder);
	      while (iter != sub_map_[k].End()) {
		if (StompPixel::SuperPixelBasedOrder(*iter,tmp_right)) {
                  costheta =
		      map_iter->UnitSphereX()*iter->UnitSphereX() +
		      map_iter->UnitSphereY()*iter->UnitSphereY() +
		      map_iter->UnitSphereZ()*iter->UnitSphereZ();
                  sintheta = 1.0 - costheta*costheta;
                  if ((sintheta < sinthetamax) &&
                      (sintheta > sinthetamin)) {
                    theta_iter = wtheta.Find(theta_begin,theta_end,
                                             sintheta);
                    theta_iter->AddToWtheta(map_iter->Density()*
                                            map_iter->Weight()*
                                            iter->Density()*iter->Weight());
                    theta_iter->AddToCounter(map_iter->Weight()*
                                             iter->Weight());
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
  } else {
    std::cout << "No angular bins have resolution " << resolution_ << "...\n";
  }

  std::cout << std::scientific << "\t\t\tDone. Average over-density: " <<
      mean_delta/pix_.size() << " for " <<
      sum_weight*pix_[0].Area() << " sq. degrees...\n";
}

AngularCorrelation::AngularCorrelation(double theta_min, double theta_max,
				       double bins_per_decade,
				       bool assign_resolutions) {
  double unit_double = floor(log10(theta_min))*bins_per_decade;
  double theta = pow(10.0,unit_double/bins_per_decade);

  while (theta < theta_max) {
    if ((theta > theta_min) && (theta < theta_max)) {
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
  sintheta_min_ = thetabin_[0].SinThetaMin();
  theta_max_ = thetabin_[thetabin_.size()-1].ThetaMax();
  sintheta_max_ = thetabin_[thetabin_.size()-1].SinThetaMax();

  if (assign_resolutions) AssignBinResolutions();
}

AngularCorrelation::AngularCorrelation(double theta_min, double theta_max,
				       unsigned long n_bins,
				       bool assign_resolutions) {
  double dtheta = (theta_max - theta_min)/n_bins;

  for (unsigned long i=0;i<n_bins;i++) {
    AngularBin thetabin;
    thetabin.SetThetaMin(theta_min + i*dtheta);
    thetabin.SetThetaMin(theta_min + (i+1)*dtheta);
    thetabin.SetTheta(0.5*(thetabin.ThetaMin()+thetabin.ThetaMax()));
    thetabin_.push_back(thetabin);
  }

  theta_min_ = thetabin_[0].ThetaMin();
  sintheta_min_ = thetabin_[0].SinThetaMin();
  theta_max_ = thetabin_[n_bins-1].ThetaMax();
  sintheta_max_ = thetabin_[n_bins-1].SinThetaMax();

  if (assign_resolutions) AssignBinResolutions();
}

void AngularCorrelation::AssignBinResolutions(double lammin, double lammax,
					      int min_resolution) {
  min_resolution_ = Stomp::MaxPixelResolution();
  max_resolution_ = Stomp::HPixResolution();

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
    int pixel_resolution = Stomp::HPixResolution()/2;

    unsigned long ny_req = 1000000000, ny_min, ny_max;
    unsigned long small_good = 0, eta_good = 0;
    StompPixel tmp_pix, tmp2_pix;

    while (((small_good < ny_req) || (eta_good < ny_req)) &&
	   (pixel_resolution <= min_resolution/2)) {

      small_good = eta_good = 0;
      pixel_resolution *= 2;

      tmp_pix.SetResolution(pixel_resolution);
      tmp2_pix.SetResolution(pixel_resolution);

      tmp_pix.SetPixnumFromAng(min_ang);
      ny_max = tmp_pix.PixelY();

      tmp_pix.SetPixnumFromAng(max_ang);
      ny_min = tmp_pix.PixelY();

      ny_req = ny_max - ny_min;

      tmp2_pix = tmp_pix;
      for (unsigned long y=ny_min+1,x=tmp2_pix.PixelX();y<=ny_max;y++) {
	tmp_pix.SetPixnumFromXY(x+1,y);
	double costheta =
	  tmp_pix.UnitSphereX()*tmp2_pix.UnitSphereX() +
	  tmp_pix.UnitSphereY()*tmp2_pix.UnitSphereY() +
	  tmp_pix.UnitSphereZ()*tmp2_pix.UnitSphereZ();
	if (1.0 - costheta*costheta < iter->SinThetaMax()) eta_good++;

	tmp_pix.SetPixnumFromXY(x,y);
	costheta =
	  tmp_pix.UnitSphereX()*tmp2_pix.UnitSphereX() +
	  tmp_pix.UnitSphereY()*tmp2_pix.UnitSphereY() +
	  tmp_pix.UnitSphereZ()*tmp2_pix.UnitSphereZ();
	if (1.0 - costheta*costheta < iter->SinThetaMax()) small_good++;

	tmp2_pix = tmp_pix;
      }
    }

    iter->SetResolution(pixel_resolution);

    if (pixel_resolution < min_resolution_) min_resolution_ = pixel_resolution;
    if (pixel_resolution > max_resolution_) max_resolution_ = pixel_resolution;
  }
}

double AngularCorrelation::ThetaMin(int resolution) {
  double theta_min = -1.0;
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    theta_min = theta_min_;
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      theta_min = iter.first->ThetaMin();
    }
  }

  return theta_min;
}

double AngularCorrelation::ThetaMax(int resolution) {
  double theta_max = -1.0;
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    theta_max = theta_max_;
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      --iter.second;
      theta_max = iter.second->ThetaMax();
    }
  }

  return theta_max;
}

double AngularCorrelation::SinThetaMin(int resolution) {
  double sintheta_min = -1.0;
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    sintheta_min = sintheta_min_;
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      sintheta_min = iter.first->SinThetaMin();
    }
  }

  return sintheta_min;
}

double AngularCorrelation::SinThetaMax(int resolution) {
  double sintheta_max = -1.0;
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    sintheta_max = sintheta_max_;
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    if (iter.first != iter.second) {
      --iter.second;
      sintheta_max = iter.second->SinThetaMax();
    }
  }

  return sintheta_max;
}

ThetaIterator AngularCorrelation::Begin(int resolution) {
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    return thetabin_.begin();
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    return iter.first;
  }
}

ThetaIterator AngularCorrelation::End(int resolution) {
  if ((resolution < Stomp::HPixResolution()) ||
      (resolution > Stomp::MaxPixelResolution()) ||
      (resolution % 2 != 0)) {
    return thetabin_.end();
  } else {
    AngularBin theta;
    theta.SetResolution(resolution);
    ThetaPair iter = equal_range(thetabin_.begin(),thetabin_.end(),
				 theta,AngularBin::ReverseResolutionOrder);
    return iter.second;
  }
}

ThetaIterator AngularCorrelation::Find(ThetaIterator begin,
				       ThetaIterator end,
				       double sintheta) {
  ThetaIterator top = --end;
  ThetaIterator bottom = begin;
  ThetaIterator iter;

  if ((sintheta < bottom->SinThetaMin()) ||
      (sintheta > top->SinThetaMax())) {
    iter = ++end;
  } else {
    ++top;
    --bottom;
    while (top-bottom > 1) {
      iter = bottom + (top - bottom)/2;
      if (sintheta < iter->SinThetaMin()) {
        top = iter;
      } else {
        bottom = iter;
      }
    }
    iter = bottom;
  }

  return iter;
}

AngularCoordinate::AngularCoordinate(double theta, double phi, Sphere sphere) {
  sphere_ = sphere;
  set_xyz_ = false;

  if (sphere_ == Survey) {
    if ((theta >= 90.0) || (theta <= -90.0)) {
      std::cout << "Illegal lambda value!\n";
      exit(1);
    }
    if (phi > 180.0) phi -= 360.0;
    if (phi < -180.0) phi += 360.0;

    theta_ = phi;
    phi_ = theta;
  }

  if (sphere_ == Equatorial) {
    if ((phi >= 90.0) || (phi <= -90.0)) {
      std::cout << "Illegal DEC value!\n";
      exit(1);
    }
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0) theta += 360.0;

    theta_ = theta;
    phi_ = phi;
  }

  if (sphere_ == Galactic) {
    if ((phi >= 90.0) || (phi <= -90.0)) {
      std::cout << "Illegal Galactic latitude value!\n";
      exit(1);
    }
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0) theta += 360.0;

    theta_ = theta;
    phi_ = phi;
  }
}

AngularCoordinate::~AngularCoordinate() {
  sphere_ = Survey;
  set_xyz_ = false;
  theta_ = 0.0;
  phi_ = 0.0;
}

void AngularCoordinate::SetSurveyCoordinates(double lambda, double eta) {
  sphere_ = Survey;

  if ((lambda >= 90.0) || (lambda <= -90.0)) {
    std::cout << "Illegal lambda value!\n";
    exit(1);
  }
  if (eta > 180.0) eta -= 360.0;
  if (eta < -180.0) eta += 360.0;

  theta_ = eta;
  phi_ = lambda;
  set_xyz_ = false;
}

void AngularCoordinate::SetEquatorialCoordinates(double ra, double dec) {
  sphere_ = Equatorial;

  if ((dec >= 90.0) || (dec <= -90.0)) {
    std::cout << "Illegal DEC value!\n";
    exit(1);
  }
  if (ra > 360.0) ra -= 360.0;
  if (ra < 0.0) ra += 360.0;

  theta_ = ra;
  phi_ = dec;
  set_xyz_ = false;
}

void AngularCoordinate::SetGalacticCoordinates(double gal_lat,
                                               double gal_lon) {
  sphere_ = Galactic;

  if ((gal_lat >= 90.0) || (gal_lat <= -90.0)) {
    std::cout << "Illegal Galactic latitude value!\n";
    exit(1);
  }
  if (gal_lon > 360.0) gal_lon -= 360.0;
  if (gal_lon < 0.0) gal_lon += 360.0;

  theta_ = gal_lon;
  phi_ = gal_lat;
  set_xyz_ = false;
}

void AngularCoordinate::InitializeUnitSphere() {
  if (set_xyz_ == false) {
    if (sphere_ == Survey) {
      us_x_ = -1.0*sin(phi_*Stomp::Deg2Rad());
      us_y_ = cos(phi_*Stomp::Deg2Rad())*
          cos(theta_*Stomp::Deg2Rad()+Stomp::EtaPole());
      us_z_ = cos(phi_*Stomp::Deg2Rad())*
          sin(theta_*Stomp::Deg2Rad()+Stomp::EtaPole());
      set_xyz_ = true;
    }

    if (sphere_ == Equatorial) {
      us_x_ = cos(Stomp::Deg2Rad()*theta_-Stomp::Node())*
          cos(Stomp::Deg2Rad()*phi_);
      us_y_ = sin(Stomp::Deg2Rad()*theta_-Stomp::Node())*
          cos(Stomp::Deg2Rad()*phi_);
      us_z_ = sin(Stomp::Deg2Rad()*phi_);
      set_xyz_ = true;
    }

    if (sphere_ == Galactic) {
      ConvertToEquatorial();

      us_x_ = cos(Stomp::Deg2Rad()*theta_-Stomp::Node())*
        cos(Stomp::Deg2Rad()*phi_);
      us_y_ = sin(Stomp::Deg2Rad()*theta_-Stomp::Node())*
        cos(Stomp::Deg2Rad()*phi_);
      us_z_ = sin(Stomp::Deg2Rad()*phi_);
      set_xyz_ = true;
    }
  }
}

void AngularCoordinate::ConvertToEquatorial() {

  if (sphere_ == Survey) {
    if (set_xyz_ == false) {
      us_x_ = -1.0*sin(phi_*Stomp::Deg2Rad());
      us_y_ = cos(phi_*Stomp::Deg2Rad())*
          cos(theta_*Stomp::Deg2Rad()+Stomp::EtaPole());
      us_z_ = cos(phi_*Stomp::Deg2Rad())*
          sin(theta_*Stomp::Deg2Rad()+Stomp::EtaPole());
      set_xyz_ = true;
    }

    theta_ = (atan2(us_y_,us_x_) + Stomp::Node())/Stomp::Deg2Rad();
    if (theta_ > 360.0) theta_ -= 360.0;
    if (theta_ < 0.0) theta_ += 360.0;
    phi_ = asin(us_z_)/Stomp::Deg2Rad();
    sphere_ = Equatorial;
  }

  if (sphere_ == Galactic) {
    double a, b, sb, cb, cbsa, ao, bo;
    double g_psi = 4.9368292465;
    double stheta = -0.88998808748;
    double ctheta = 0.45598377618;
    double g_phi = 0.57477043300;

    a = theta_*Stomp::Deg2Rad() - g_phi;
    b = phi_*Stomp::Deg2Rad();

    sb = sin(b);
    cb = cos(b);
    cbsa = cb*sin(a);

    b = -1.0*stheta*cbsa + ctheta*sb;
    if (b > 1.0) b = 1.0;

    bo = asin(b)/Stomp::Deg2Rad();

    a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

    ao = (a+g_psi+4.0*Stomp::Pi())/Stomp::Deg2Rad();
    if (ao > 360.0) ao -= 360.0;

    theta_ = ao;
    phi_ = bo;
    sphere_ = Equatorial;
  }
}

void AngularCoordinate::ConvertToSurvey() {

  if (sphere_ == Equatorial) {

    if (set_xyz_ == false) {
      us_x_ = cos(Stomp::Deg2Rad()*theta_-Stomp::Node())*
          cos(Stomp::Deg2Rad()*phi_);
      us_y_ = sin(Stomp::Deg2Rad()*theta_-Stomp::Node())*
          cos(Stomp::Deg2Rad()*phi_);
      us_z_ = sin(Stomp::Deg2Rad()*phi_);
      set_xyz_ = true;
    }

    phi_ = -1.0*asin(us_x_)/Stomp::Deg2Rad();
    theta_ = (atan2(us_z_,us_y_) - Stomp::EtaPole())/Stomp::Deg2Rad();
    if (theta_ < -180.0) theta_ += 360.0;
    if (theta_ > 180.0) theta_ -= 360.0;
    sphere_ = Survey;
  }

  if (sphere_ == Galactic) {
    ConvertToEquatorial();

    if (set_xyz_ == false) {
      us_x_ = cos(Stomp::Deg2Rad()*theta_-Stomp::Node())*
	cos(Stomp::Deg2Rad()*phi_);
      us_y_ = sin(Stomp::Deg2Rad()*theta_-Stomp::Node())*
	cos(Stomp::Deg2Rad()*phi_);
      us_z_ = sin(Stomp::Deg2Rad()*phi_);
      set_xyz_ = true;
    }

    phi_ = -1.0*asin(us_x_)/Stomp::Deg2Rad();
    theta_ = (atan2(us_z_,us_y_) - Stomp::EtaPole())/Stomp::Deg2Rad();
    if (theta_ < -180.0) theta_ += 360.0;
    if (theta_ > 180.0) theta_ -= 360.0;
    sphere_ = Survey;
  }
}

void AngularCoordinate::ConvertToGalactic() {
  if (sphere_ == Survey) {
    ConvertToEquatorial();

    double a, b, sb, cb, cbsa, ao, bo;
    double g_psi = 0.57477043300;
    double stheta = 0.88998808748;
    double ctheta = 0.45598377618;
    double g_phi = 4.9368292465;

    a = theta_*Stomp::Deg2Rad() - g_phi;
    b = phi_*Stomp::Deg2Rad();

    sb = sin(b);
    cb = cos(b);
    cbsa = cb*sin(a);

    b = -1.0*stheta*cbsa + ctheta*sb;
    if (b > 1.0) b = 1.0;

    bo = asin(b)/Stomp::Deg2Rad();

    a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

    ao = (a+g_psi+4.0*Stomp::Pi())/Stomp::Deg2Rad();

    while (ao > 360.0) ao -= 360.0;

    theta_ = ao;
    phi_ = bo;
    sphere_ = Galactic;
  }

  if (sphere_ == Equatorial) {
    double a, b, sb, cb, cbsa, ao, bo;
    double g_psi = 0.57477043300;
    double stheta = 0.88998808748;
    double ctheta = 0.45598377618;
    double g_phi = 4.9368292465;

    a = theta_*Stomp::Deg2Rad() - g_phi;
    b = phi_*Stomp::Deg2Rad();

    sb = sin(b);
    cb = cos(b);
    cbsa = cb*sin(a);

    b = -1.0*stheta*cbsa + ctheta*sb;
    if (b > 1.0) b = 1.0;

    bo = asin(b)/Stomp::Deg2Rad();

    a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

    ao = (a+g_psi+4.0*Stomp::Pi())/Stomp::Deg2Rad();

    while (ao > 360.0) ao -= 360.0;

    theta_ = ao;
    phi_ = bo;
    sphere_ = Galactic;
  }
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
}

void AngularCoordinate::SurveyToEquatorial(double lambda, double eta,
                                           double& ra, double& dec) {
  double x, y, z;

  x = -1.0*sin(lambda*Stomp::Deg2Rad());
  y = cos(lambda*Stomp::Deg2Rad())*cos(eta*Stomp::Deg2Rad()+Stomp::EtaPole());
  z = cos(lambda*Stomp::Deg2Rad())*sin(eta*Stomp::Deg2Rad()+Stomp::EtaPole());

  ra = (atan2(y,x) + Stomp::Node())/Stomp::Deg2Rad();
  dec = asin(z)/Stomp::Deg2Rad();
}

void AngularCoordinate::SurveyToXYZ(double lambda, double eta,
                                    double& x, double& y, double& z) {
  x = -1.0*sin(lambda*Stomp::Deg2Rad());
  y = cos(lambda*Stomp::Deg2Rad())*cos(eta*Stomp::Deg2Rad()+Stomp::EtaPole());
  z = cos(lambda*Stomp::Deg2Rad())*sin(eta*Stomp::Deg2Rad()+Stomp::EtaPole());
}

void AngularCoordinate::EquatorialToSurvey(double ra, double dec,
                                           double& lambda, double& eta) {
  double x, y, z;

  x = cos(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  y = sin(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  z = sin(Stomp::Deg2Rad()*dec);

  lambda = -1.0*asin(x)/Stomp::Deg2Rad();
  eta = (atan2(z,y) - Stomp::EtaPole())/Stomp::Deg2Rad();
  if (eta < -180.0) eta += 360.0;
  if (eta > 180.0) eta -= 360.0;
}

void AngularCoordinate::EquatorialToXYZ(double ra, double dec,
                                        double& x, double& y, double& z) {
  x = cos(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  y = sin(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  z = sin(Stomp::Deg2Rad()*dec);
}

void AngularCoordinate::EquatorialToGalactic(double ra, double dec,
                                             double& gal_lat,double& gal_lon) {
  double a, b, sb, cb, cbsa, ao, bo;
  double g_psi = 0.57477043300;
  double stheta = 0.88998808748;
  double ctheta = 0.45598377618;
  double g_phi = 4.9368292465;

  a = ra*Stomp::Deg2Rad() - g_phi;
  b = dec*Stomp::Deg2Rad();

  sb = sin(b);
  cb = cos(b);
  cbsa = cb*sin(a);

  b = -1.0*stheta*cbsa + ctheta*sb;
  if (b > 1.0) b = 1.0;

  bo = asin(b)/Stomp::Deg2Rad();

  a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

  ao = (a+g_psi+4.0*Stomp::Pi())/Stomp::Deg2Rad();

  while (ao > 360.0) ao -= 360.0;

  gal_lon = ao;
  gal_lat = bo;
}

void AngularCoordinate::GalacticToEquatorial(double gal_lat, double gal_lon,
                                             double& ra, double& dec) {
  double a, b, sb, cb, cbsa, ao, bo;
  double g_psi = 4.9368292465;
  double stheta = -0.88998808748;
  double ctheta = 0.45598377618;
  double g_phi = 0.57477043300;

  a = gal_lon*Stomp::Deg2Rad() - g_phi;
  b = gal_lat*Stomp::Deg2Rad();

  sb = sin(b);
  cb = cos(b);
  cbsa = cb*sin(a);

  b = -1.0*stheta*cbsa + ctheta*sb;
  if (b > 1.0) b = 1.0;

  bo = asin(b)/Stomp::Deg2Rad();

  a = atan2(ctheta*cbsa + stheta*sb,cb*cos(a));

  ao = (a+g_psi+4.0*Stomp::Pi())/Stomp::Deg2Rad();
  while (ao > 360.0) ao -= 360.0;

  ra = ao;
  dec = bo;
}

void AngularCoordinate::GalacticToXYZ(double gal_lat, double gal_lon,
                                      double& x, double& y, double& z) {
  double ra, dec;
  GalacticToEquatorial(gal_lat,gal_lon,ra,dec);

  x = cos(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  y = sin(Stomp::Deg2Rad()*ra-Stomp::Node())*cos(Stomp::Deg2Rad()*dec);
  z = sin(Stomp::Deg2Rad()*dec);
}

WeightedAngularCoordinate::WeightedAngularCoordinate() {
  weight_ = 0.0;
}

WeightedAngularCoordinate::WeightedAngularCoordinate(double theta,
						     double phi,
						     double weight,
						     Sphere sphere) {
  if (sphere == Survey) SetSurveyCoordinates(theta,phi);

  if (sphere == Equatorial) SetEquatorialCoordinates(theta,phi);

  if (sphere == Galactic) SetGalacticCoordinates(theta,phi);

  weight_ = weight;
}

WeightedAngularCoordinate::~WeightedAngularCoordinate() {
  weight_ = 0.0;
}

FootprintBound::FootprintBound() {
  area_ = 0.0;
  pixel_area_ = 0.0;
  lammin_ = etamin_ = 200.0;
  lammax_ = etamax_ = -200.0;
  x_min_ = x_max_ = y_min_ = y_max_ = 0;
  max_resolution_ = Stomp::MaxPixelResolution();
}

FootprintBound::~FootprintBound() {
  if (pix_.empty() == false) pix_.clear();

  area_ = 0.0;
  pixel_area_ = 0.0;
  lammin_ = etamin_ = 200.0;
  lammax_ = etamax_ = -200.0;
  x_min_ = x_max_ = y_min_ = y_max_ = 0;
  max_resolution_ = -1;
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
  return Stomp::HPixArea()*Stomp::MaxSuperpixnum();
}

double FootprintBound::ScorePixel(StompPixel& pix) {

  unsigned long nx = Stomp::Nx0()*pix.Resolution();
  unsigned long ny = Stomp::Ny0()*pix.Resolution();
  unsigned long x = pix.PixelX(), y = pix.PixelY();

  double lammid = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.5)/ny);
  double lammin = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+1.0)/ny);
  double lammax = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.0)/ny);
  double lam_quart = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.75)/ny);
  double lam_three = 90.0 - Stomp::Rad2Deg()*acos(1.0-2.0*(y+0.25)/ny);

  double etamid = Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+0.5))/nx +
      Stomp::EtaOffSet();
  if (etamid >= 180.0) etamid -= 360.0;
  if (etamid <= -180.0) etamid += 360.0;

  double etamin = Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+0.0))/nx +
      Stomp::EtaOffSet();
  if (etamin >= 180.0) etamin -= 360.0;
  if (etamin <= -180.0) etamin += 360.0;

  double etamax = Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+1.0))/nx +
      Stomp::EtaOffSet();
  if (etamax >= 180.0) etamax -= 360.0;
  if (etamax <= -180.0) etamax += 360.0;

  double eta_quart = Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+0.25))/nx +
      Stomp::EtaOffSet();
  if (eta_quart >= 180.0) eta_quart -= 360.0;
  if (eta_quart <= -180.0) eta_quart += 360.0;

  double eta_three = Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x+0.75))/nx +
      Stomp::EtaOffSet();
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

bool FootprintBound::FindXYBounds(const int resolution) {

  unsigned long nx = Stomp::Nx0()*resolution, ny = Stomp::Ny0()*resolution;

  StompPixel::AreaIndex(resolution,lammin_,lammax_,etamin_,etamax_,
                        x_min_, x_max_, y_min_, y_max_);

  // Checking top border

  bool found_pixel = true;

  StompPixel tmp_pix;

  tmp_pix.SetResolution(resolution);

  while (found_pixel) {

    found_pixel = false;

    unsigned long j = y_max_, nx_pix;

    if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
      nx_pix = nx - x_min_ + x_max_ + 1;
    } else {
      nx_pix = x_max_ - x_min_ + 1;
    }

    for (unsigned long m=0,i=x_min_;m<nx_pix;m++,i++) {
      if (i == nx) i = 0;

      tmp_pix.SetPixnumFromXY(i,j);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        i = x_max_ + 1;
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
  }

  // Checking bottom border

  found_pixel = true;

  while (found_pixel) {

    found_pixel = false;

    unsigned long j = y_min_, nx_pix;

    if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
      nx_pix = nx - x_min_ + x_max_ + 1;
    } else {
      nx_pix = x_max_ - x_min_ + 1;
    }

    for (unsigned long m=0,i=x_min_;m<nx_pix;m++,i++) {
      if (i == nx) i = 0;

      tmp_pix.SetPixnumFromXY(i,j);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        i = x_max_ + 1;
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
  }

  // Checking left border

  found_pixel = true;

  while (found_pixel) {

    found_pixel = false;

    unsigned long i = x_min_;

    for (unsigned long j=y_min_;j<=y_max_;j++) {

      tmp_pix.SetPixnumFromXY(i,j);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        j = y_max_ + 1;
      }
    }

    if (found_pixel) {
      if (x_min_ == 0) {
        x_min_ = nx - 1;
      } else {
        x_min_--;
      }
    }
  }

  // Checking right border

  found_pixel = true;

  while (found_pixel) {

    found_pixel = false;

    unsigned long i = x_min_;

    for (unsigned long j=y_min_;j<=y_max_;j++) {

      tmp_pix.SetPixnumFromXY(i,j);

      // This if statement checks positions within the pixel against the
      // footprint bound.
      if (ScorePixel(tmp_pix) < -0.000001) {
        found_pixel = true;
        j = y_max_ + 1;
      }
    }

    if (found_pixel) {
      if (x_max_ == nx - 1) {
        x_max_ = 0;
      } else {
        x_max_++;
      }
    }
  }

  return true;
}

int FootprintBound::FindStartingResolution() {

  double min_area = StompPixel::PixelArea(max_resolution_);

  if (area_ < 10.0*min_area) {
    std::cout << "Footprint area too small to pixelize...\n";
    return -1;
  }

  int starting_resolution = Stomp::HPixResolution();

  // We want to start things off with the coarsest possible resolution to
  // save time, but we have to be careful that we're not so coarse that we
  // miss parts of the footprint.  This finds the resolution that has pixels
  // about 1/100th the area of the footprint.

  while (area_/StompPixel::PixelArea(starting_resolution) <= 100.0)
    starting_resolution *= 2;

  return starting_resolution;
}

bool FootprintBound::Pixelize() {

  if (pix_.empty() == false) pix_.clear();
  pixel_area_ = 0.0;

  int starting_resolution = FindStartingResolution();

  if ((starting_resolution < Stomp::HPixResolution()) ||
      (starting_resolution > max_resolution_)) {
    std::cout << "Error finding starting resolution...\n";
    return false;
  }

  // We need to be careful around the poles since the pixels there get
  // very distorted.
  if ((lammin_ > 85.0) || (lammax_ < -85.0)) starting_resolution = 512;

  if (FindXYBounds(starting_resolution)) {

    StompVector resolve_pix, previous_pix;

    for (int resolution=starting_resolution;
         resolution<=max_resolution_;resolution*=2) {

      unsigned n_keep = 0;
      unsigned long nx = Stomp::Nx0()*resolution;
      StompPixel tmp_pix;
      tmp_pix.SetResolution(resolution);

      double score;
      AngularCoordinate ang;

      if (resolution == starting_resolution) {

        resolve_pix.clear();

        unsigned long nx_pix;
        if ((x_max_ < x_min_) && (x_min_ > nx/2)) {
          nx_pix = nx - x_min_ + x_max_ + 1;
        } else {
          nx_pix = x_max_ - x_min_ + 1;
        }

        for (unsigned long y=y_min_;y<=y_max_;y++) {
          for (unsigned long m=0,x=x_min_;m<nx_pix;m++,x++) {
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
          for (StompIterator iter=previous_pix.begin();
               iter!=previous_pix.end();++iter) {
            StompVector sub_pix;
            iter->SubPix(resolution,sub_pix);
            for (StompIterator sub_iter=sub_pix.begin();
                 sub_iter!=sub_pix.end();++sub_iter)
              resolve_pix.push_back(*sub_iter);
          }
        }

        previous_pix.clear();

        for (StompIterator iter=resolve_pix.begin();
             iter!=resolve_pix.end();++iter) previous_pix.push_back(*iter);

        resolve_pix.clear();

        unsigned long x_min, x_max, y_min, y_max;

        for (StompIterator iter=previous_pix.begin();
             iter!=previous_pix.end();++iter) {

          iter->SubPix(resolution,x_min,x_max,y_min,y_max);

          for (unsigned long y=y_min;y<=y_max;y++) {
            for (unsigned long x=x_min;x<=x_max;x++) {
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

    if (area_ > pixel_area_) {
      sort(resolve_pix.begin(),resolve_pix.end(),StompPixel::WeightedOrder);

      unsigned long n=0;
      double ur_weight = resolve_pix[n].Weight();
      while ((n < resolve_pix.size()) &&
             ((area_ > pixel_area_) ||
              ((resolve_pix[n].Weight() < ur_weight + 0.1) &&
               (resolve_pix[n].Weight() > ur_weight - 0.1)))) {
        ur_weight = resolve_pix[n].Weight();
        resolve_pix[n].SetWeight(weight_);
        AddToPixelizedArea(max_resolution_);
        pix_.push_back(resolve_pix[n]);
        n++;
      }
    }

    StompPixel::ResolvePixel(pix_);

    return true;
  } else {
    std::cout << "Finding x-y bounds failed somehow.  Bailing...\n";
    return false;
  }
}

CircleBound::CircleBound(const AngularCoordinate& ang,
                         double radius, double weight) {
  SetWeight(weight);

  ang_ = ang;
  radius_ = radius;
  sin2radius_ = sin(radius*Stomp::Deg2Rad())*sin(radius*Stomp::Deg2Rad());

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
  if (lammin < -90.0) lammin = -90.0;

  double lammax = ang_.Lambda() + radius_;
  if (lammax > 90.0) lammax = 90.0;

  double etamin, etamax;

  if (AngularCoordinate::EtaMultiplier(lammax) >
      AngularCoordinate::EtaMultiplier(lammin)) {
    etamin = ang_.Eta() - AngularCoordinate::EtaMultiplier(lammax)*radius_;
    etamax = ang_.Eta() + AngularCoordinate::EtaMultiplier(lammax)*radius_;
  } else {
    etamin = ang_.Eta() - AngularCoordinate::EtaMultiplier(lammin)*radius_;
    etamax = ang_.Eta() + AngularCoordinate::EtaMultiplier(lammin)*radius_;
  }

  if (etamin < -180.0) etamin += 360.0;
  if (etamax > 180.0) etamax -= 360.0;

  SetAngularBounds(lammin,lammax,etamin,etamax);

  return true;
}

bool CircleBound::FindArea() {
  SetArea((1.0 -
           cos(radius_*Stomp::Deg2Rad()))*2.0*Stomp::Pi()*Stomp::Strad2Deg());
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

PolygonBound::PolygonBound(AngularVector& ang, double weight) {

  SetWeight(weight);

  for (AngularIterator iter=ang.begin();iter!=ang.end();++iter)
    ang_.push_back(*iter);

  n_vert_ = ang_.size();

  x_.reserve(n_vert_);
  y_.reserve(n_vert_);
  z_.reserve(n_vert_);
  dot_.reserve(n_vert_);

  for (unsigned long i=0;i<n_vert_;i++) {

    std::vector<double> tmp_x, tmp_y, tmp_z;

    for (unsigned long j=0;j<n_vert_;j++) {
      tmp_x.push_back(ang_[j].UnitSphereX());
      tmp_y.push_back(ang_[j].UnitSphereY());
      tmp_z.push_back(ang_[j].UnitSphereZ());
    }

    for (unsigned long j=0;j<n_vert_;j++) {
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

  for (unsigned long i=0;i<n_vert_;i++) {
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

  for (unsigned long j=0,k=1;j<n_vert_;j++,k++) {
    if (k == n_vert_) k = 0;

    double cm = (-x_[j]*x_[k] - y_[j]*y_[k] - z_[j]*z_[k]);

    sum += acos(cm);
  }

  double tmp_area = (sum - (n_vert_ - 2)*Stomp::Pi())*Stomp::Strad2Deg();

  if (tmp_area > 4.0*Stomp::Pi()*Stomp::Strad2Deg()) {
    std::cout << "Polygon area is over half the sphere.  This is bad.\n";
    return false;
  }

  SetArea(tmp_area);

  return true;
}

bool PolygonBound::CheckPoint(AngularCoordinate& ang) {
  bool in_polygon = true;

  unsigned long n=0;
  while ((n < n_vert_) && (in_polygon)) {

    in_polygon = false;
    double dot = 1.0 - x_[n]*ang.UnitSphereX() -
        y_[n]*ang.UnitSphereY() - z_[n]*ang.UnitSphereZ();
    if (DoubleLE(dot_[n],0.0)) {
      if (DoubleLE(fabs(dot_[n]),dot)) in_polygon = true;
    } else {
      if (DoubleGE(dot_[n],dot)) in_polygon = true;
    }
    n++;
  }

  return in_polygon;
}
