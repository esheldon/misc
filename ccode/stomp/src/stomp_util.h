#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <sstream>
#include "MersenneTwister.h"

class Stomp;
class StompPixel;
class StompDensityPixel;
class StompPointPixel;
class StompMap;
class StompSubMap;
class StompDensityMap;
class StompDensitySubMap;
class StompSection;
class FootprintBound;
class CircleBound;
class PolygonBound;
class AngularCoordinate;
class AngularBin;
class AngularCorrelation;

class Stomp {
  // This is our singleton class for storing all of the constants that define
  // our pixelization scheme.  At its most basic, the pixelization is an
  // equal-area rectangular scheme.  This makes calculations like total area
  // and density simple, but it does mean that you encounter significant
  // pixel distortion as you approach the poles.  You should never actually
  // instantiate this class, but just call its methods directly.

 public:
  static double Pi() {
    static double pi = 2.0*asin(1.0);
    return pi;
  };
  static double Deg2Rad() {
    static double deg2Rad = Pi()/180.0;
    return deg2Rad;
  };
  static double Rad2Deg() {
    static double rad2Deg = 180.0/Pi();
    return rad2Deg;
  };
  static double Strad2Deg() {
    static double strad2Deg = 180.0*180.0/(Pi()*Pi());
    return strad2Deg;
  };
  static unsigned long Nx0() {
    static unsigned long nx0 = 36;
    return nx0;
  };
  static unsigned long Ny0() {
    static unsigned long ny0 = 13;
    return ny0;
  };

  // For historical reasons, coordinate system is built around the SDSS
  // survey coordinates rather than traditional equatorial RA-DEC coordinates.
  // To switch to those coordinates, the next five functions would need to be
  // modified so that EtaOffSet, Node and EtaPole all return zero.
  static double EtaOffSet() {
    static double etaOffSet = 91.25;
    return etaOffSet;
  };
  static double SurveyCenterRA() {
    static double surveyCenterRA = 185.0;
    return surveyCenterRA;
  };
  static double SurveyCenterDEC() {
    static double surveyCenterDEC = 32.5;
    return surveyCenterDEC;
  };
  static double Node() {
    static double node = Deg2Rad()*(SurveyCenterRA()-90.0);
    return node;
  };
  static double EtaPole() {
    static double etaPole = Deg2Rad()*SurveyCenterDEC();
    return etaPole;
  };

  // For the purposes of rapid localization, we set a basic level of
  // pixelization that divides the sphere into 7488 superpixels (the value is
  // chosen such that the width of one pixel matches the fiducial width of a
  // stripe in the SDSS survey coordinates.
  //
  // Pixels are addressed hierarchically, so a given pixel is refined into 4
  // sub-pixels and is joined with 3 nearby pixels to form a superpixel.  The
  // level of refinement is encoded in the "resolution" level, with the
  // following two functions defining the limits on acceptable values (basically
  // limited by the number of pixels that can be addressed in a single
  // superpixel with 32-bit integers).  The current limits allow for about
  // half a terapixel on the sphere, which corresponds to roughly 2 arcsecond
  // resolution.  Valid resolution values are all powers of 2; refining the
  // pixel scale increases resolution by a factor of 2 at each level and
  // coarsening the pixel scale reduces the resolution by a factor of 2 at each
  // level.
  static int HPixResolution() {
    static int hpix_resolution = 4;
    return hpix_resolution;
  };
  static int MaxPixelResolution() {
    static int max_pix_resolution = 32768;
    return max_pix_resolution;
  };
  static double HPixArea() {
    static double hpix_area =
        4.0*Pi()*Strad2Deg()/(HPixResolution()*HPixResolution()*Nx0()*Ny0());
    return hpix_area;
  };
  static unsigned long MaxPixnum() {
    static unsigned long max_pixnum = Nx0()*Ny0()*2048*2048;
    return max_pixnum;
  };
  static unsigned long MaxSuperpixnum() {
    static unsigned long max_superpixnum =
        Nx0()*Ny0()*HPixResolution()*HPixResolution();
    return max_superpixnum;
  };
};


class AngularCoordinate {
  // Our generic class for handling angular positions.  The idea is that
  // locations on the celestial sphere should be abstract objects from which
  // you can draw whatever angular coordinate pair is necessary for a given
  // use case.  AngularCoordinate's can be instantiated with a particular
  // coordinate system in mind or that can be set later on.
  
 public:
  enum Sphere {Survey, Equatorial, Galactic};
  AngularCoordinate(double theta = 0.0, double phi = 0.0,
                    Sphere sphere = Survey);
  ~AngularCoordinate();

  // In addition to the angular coordinate, this class also allows you to
  // extract the X-Y-Z Cartesian coordinates of the angular position on a unit
  // sphere.  This method initializes that functionality, but probably
  // shouldn't ever need to be called explicitly since it's called whenever
  // necessary by the associated methods.
  void InitializeUnitSphere();

  // These three methods let you explicitly set the angular coordinate in one
  // of the supported angular coordinate systems.  Calling these methods resets
  // any other previous values that the AngularCoordinate instance may have
  // had.
  void SetSurveyCoordinates(double lambda, double eta);
  void SetEquatorialCoordinates(double ra, double dec);
  void SetGalacticCoordinates(double gal_lat, double gal_lon);

  // The internal representation of a given AngularCoordinate is stored in
  // whatever coordinate system was last requested.  In principle, you should
  // never need to call the explicit methods for converting between the
  // different systems, as they're called automatically if you request a
  // coordinate other than the currently stored representation, but these
  // methods are available for that if so desired.
  void ConvertToGalactic();
  void ConvertToEquatorial();
  void ConvertToSurvey();

  // The basic methods for extracting each of the angular coordinate values.
  double Lambda() {
    if (sphere_ != Survey) ConvertToSurvey();
    return phi_;
  };
  double Eta() {
    if (sphere_ != Survey) ConvertToSurvey();
    return theta_;
  };
  double RA() {
    if (sphere_ != Equatorial) ConvertToEquatorial();
    return theta_;
  };
  double DEC() {
    if (sphere_ != Equatorial) ConvertToEquatorial();
    return phi_;
  };
  double GalLon() {
    if (sphere_ != Galactic) ConvertToGalactic();
    return theta_;
  };
  double GalLat() {
    if (sphere_ != Galactic) ConvertToGalactic();
    return phi_;
  };


  // And the associated methods for doing the same with the unit sphere
  // Cartesian coordinates.
  double UnitSphereX() {
    if (set_xyz_ == false) InitializeUnitSphere();
    return us_x_;
  };
  double UnitSphereY() {
    if (set_xyz_ == false) InitializeUnitSphere();
    return us_y_;
  };
  double UnitSphereZ() {
    if (set_xyz_ == false) InitializeUnitSphere();
    return us_z_;
  };


  // Return a string representation of this coordinate
  // If no argument is given it defaults to Equatorial.
  // e.g.  cout<<"Position is: "<<ang.Repr()<<endl;
  std::string Repr(Sphere sphere=Equatorial) {
	  std::stringstream output;
	  switch (sphere) {
		  case Equatorial: 
			  output << "(" << RA() << "," << DEC() << ")";
			  break;
		  case Galactic: 
			  output << "(" << GalLon() << "," << GalLat() << ")";
			  break;
			  //case Survey:
		  default:
			  output << "(" << Lambda() << "," << Eta() << ")";
			  break;
	  }
	  return(output.str());
  }


  // Once you have a single angular position on the sphere, very quickly you
  // end up wanting to know the angular distance between your coordinate and
  // another.  This method returns that value in degrees.
  double AngularDistance(AngularCoordinate& ang) {
    if (set_xyz_ == false) InitializeUnitSphere();
    return acos(us_x_*ang.UnitSphereX() + us_y_*ang.UnitSphereY() +
                us_z_*ang.UnitSphereZ())/Stomp::Deg2Rad();
  };

  // And these two methods return the dot-product and cross-product between
  // the unit vector represented by your angular position and another.
  double DotProduct(AngularCoordinate& ang) {
    if (set_xyz_ == false) InitializeUnitSphere();
    return us_x_*ang.UnitSphereX() +
        us_y_*ang.UnitSphereY() + us_z_*ang.UnitSphereZ();
  };
  AngularCoordinate CrossProduct(AngularCoordinate& ang) {
    if (set_xyz_ == false) InitializeUnitSphere();
    double x_perp = us_y_*ang.UnitSphereZ() - us_z_*ang.UnitSphereY();
    double y_perp = us_x_*ang.UnitSphereZ() - us_z_*ang.UnitSphereX();
    double z_perp = us_x_*ang.UnitSphereY() - us_y_*ang.UnitSphereX();
    double r_perp = sqrt(x_perp*x_perp + y_perp*y_perp + z_perp*z_perp);

    return AngularCoordinate((atan2(y_perp/r_perp,x_perp/r_perp)+
                              Stomp::Node())/Stomp::Deg2Rad(),
                             asin(z_perp/r_perp)/Stomp::Deg2Rad(),Equatorial);
  };

  // Static methods for when users want to switch between coordinate systems
  // without instantiating the class.
  static void SurveyToGalactic(double lambda, double eta,
                               double& gal_lat, double& gal_lon);
  static void SurveyToEquatorial(double lambda, double eta,
                                 double& ra, double& dec);
  static void EquatorialToSurvey(double ra, double dec,
                                 double& lambda, double& eta);
  static void EquatorialToGalactic(double ra, double dec,
                                   double& gal_lat, double& gal_lon);
  static void GalacticToSurvey(double gal_lat, double gal_lon,
                               double& lambda, double& eta);
  static void GalacticToEquatorial(double gal_lat, double gal_lon,
                                   double& ra, double& dec);
  static void SurveyToXYZ(double lambda, double eta,
			  double& x, double& y, double& z);
  static void EquatorialToXYZ(double ra, double dec,
			      double& x, double& y, double& z);
  static void GalacticToXYZ(double gal_lat, double gal_lon,
			    double& x, double& y, double& z);

  // This is a bit more obscure.  The idea here is that, when you want to find
  // the pixel bounds that subtend a given angular scale about a point on the
  // sphere, finding those bounds in latitude is easier than in longitude.
  // Given a latitude in one of the coordinate systems, the multiplier tells
  // you how many more pixels you should check in the longitude direction
  // relative to the latitude direction.
  static double EtaMultiplier(double lambda) {
    return (1.0 + 0.000192312*lambda*lambda -
            1.82764e-08*lambda*lambda*lambda*lambda +
            1.28162e-11*lambda*lambda*lambda*lambda*lambda*lambda);
  };
  static double RAMultiplier(double dec) {
    return (1.0 + 0.000192312*dec*dec -
            1.82764e-08*dec*dec*dec*dec +
            1.28162e-11*dec*dec*dec*dec*dec*dec);
  };
  static double GalLonMultiplier(double gal_lat) {
    return (1.0 + 0.000192312*gal_lat*gal_lat -
            1.82764e-08*gal_lat*gal_lat*gal_lat*gal_lat +
            1.28162e-11*gal_lat*gal_lat*gal_lat*gal_lat*gal_lat*gal_lat);
  };


 private:
  double theta_, phi_, us_x_, us_y_, us_z_;
  Sphere sphere_;
  bool set_xyz_;
};

typedef std::vector<AngularCoordinate> AngularVector;
typedef AngularVector::iterator AngularIterator;

class WeightedAngularCoordinate : public AngularCoordinate {
  // Sub-class of AngularCoordinate where we attach a weight value to that
  // angular position.

 public:
  WeightedAngularCoordinate();
  WeightedAngularCoordinate(double theta, double phi,
			    double weight, Sphere sphere = Survey);
  ~WeightedAngularCoordinate();
  void SetWeight(double weight) {
    weight_ = weight;
  };
  double Weight() {
    return weight_;
  };

 private:
  double weight_;
};

typedef std::vector<WeightedAngularCoordinate> WAngularVector;
typedef WAngularVector::iterator WAngularIterator;

class AngularBin {
 public:
  AngularBin() {
    theta_min_ = theta_max_ = sintheta_min_ = sintheta_max_ = wtheta_ =
        counter_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    resolution_ = -1;
  };
  ~AngularBin() {
    theta_min_ = theta_max_ = sintheta_min_ = sintheta_max_ = wtheta_ =
        counter_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    resolution_ = -1;
  };
  void SetResolution(int resolution) {
    resolution_ = resolution;
  };
  void SetTheta(double theta) {
    theta_ = theta;
  };
  void SetThetaMin(double theta_min) {
    theta_min_ = theta_min;
    sintheta_min_ = sin(theta_min_*Stomp::Deg2Rad())*
        sin(theta_min_*Stomp::Deg2Rad());
  };
  void SetThetaMax(double theta_max) {
    theta_max_ = theta_max;
    sintheta_max_ = sin(theta_max_*Stomp::Deg2Rad())*
        sin(theta_max_*Stomp::Deg2Rad());
  };
  void AddToWtheta(double dwtheta) {
    wtheta_ += dwtheta;
  };
  void AddToCounter(double weight) {
    counter_ += weight;
  };
  void AddToGalGal(double weight) {
    gal_gal_ += weight;
  };
  void AddToGalRand(double weight) {
    gal_rand_ += weight;
  };
  void AddToRandGal(double weight) {
    rand_gal_ += weight;
  };
  void AddToRandRand(double weight) {
    rand_rand_ += weight;
  };
  int Resolution() {
    return resolution_;
  };
  double Theta() {
    return theta_;
  };
  double ThetaMin() {
    return theta_min_;
  };
  double ThetaMax() {
    return theta_max_;
  };
  double SinThetaMin() {
    return sintheta_min_;
  };
  double SinThetaMax() {
    return sintheta_max_;
  };
  double Wtheta() {
    return wtheta_;
  };
  double Counter() {
    return counter_;
  };
  double GalGal() {
    return gal_gal_;
  };
  double GalRand() {
    return gal_rand_;
  };
  double RandGal() {
    return rand_gal_;
  };
  double RandRand() {
    return rand_rand_;
  };
  static bool ThetaOrder(AngularBin theta_a, AngularBin theta_b) {
    return (theta_a.ThetaMin() < theta_b.ThetaMin() ? true : false);
  }
  static bool SinThetaOrder(AngularBin theta_a, AngularBin theta_b) {
    return (theta_a.SinThetaMin() < theta_b.SinThetaMin() ? true : false);
  }
  static bool ReverseResolutionOrder(AngularBin theta_a, AngularBin theta_b) {
    return (theta_b.Resolution() < theta_a.Resolution() ? true : false);
  }

 private:
  double theta_min_, theta_max_, sintheta_min_, sintheta_max_, theta_;
  double wtheta_, counter_, gal_gal_, gal_rand_, rand_gal_, rand_rand_;
  int resolution_;
};

typedef std::vector<AngularBin> ThetaVector;
typedef ThetaVector::iterator ThetaIterator;
typedef std::pair<ThetaIterator,ThetaIterator> ThetaPair;

class AngularCorrelation {
 public:
  AngularCorrelation(double theta_min, double theta_max,
		     double bins_per_decade, bool assign_resolutions = true);
  AngularCorrelation(double theta_min, double theta_max,
		     unsigned long n_bins, bool assign_resolutions = true);
  ~AngularCorrelation() {
    thetabin_.clear();
  };
  void AssignBinResolutions(double lammin = -70.0, double lammax = 70.0,
			    int min_resolution = Stomp::MaxPixelResolution());
  double ThetaMin(int resolution = -1);
  double ThetaMax(int resolution = -1);
  double SinThetaMin(int resolution = -1);
  double SinThetaMax(int resolution = -1);
  ThetaIterator Begin(int resolution = -1);
  ThetaIterator End(int resolution = -1);
  ThetaIterator Find(ThetaIterator begin, ThetaIterator end,
		     double sintheta);
  unsigned long NBins() {
    return thetabin_.size();
  };
  int MinResolution() {
    return min_resolution_;
  };
  int MaxResolution() {
    return max_resolution_;
  };

 private:
  ThetaVector thetabin_;
  double theta_min_, theta_max_, sintheta_min_, sintheta_max_;
  int min_resolution_, max_resolution_;
};

typedef std::vector<AngularCorrelation> WThetaVector;
typedef WThetaVector::iterator WThetaIterator;

class StompPixel {
  // The core class for this library.  An instance of this class represents
  // a single pixel covering a particular region of the sky, with a particular
  // weight represented by a float.  StompPixels can be instantiated with an
  // AngularCoordinate and resolution level or pixel indices or just
  // instantiated with no particular location.
 public:
  StompPixel();
  StompPixel(const int resolution, const unsigned long pixnum,
             const double weight = 0.0);
  StompPixel(const int resolution, const unsigned long hpixnum,
             const unsigned long superpixnum, const double weight = 0.0);
  StompPixel(AngularCoordinate& ang, const int resolution,
             const double weight = 0.0);
  StompPixel(const unsigned long x, const unsigned long y,
             const int resolution, const double weight = 0.0);
  ~StompPixel();
  void SetPixnumFromAng(AngularCoordinate& ang);
  inline void SetResolution(int resolution) {
    resolution_ = resolution;
    x_ = 0;
    y_ = 0;
  };
  inline void SetPixnumFromXY(unsigned long x, unsigned long y) {
    x_ = x;
    y_ = y;
  };
  inline int Resolution() {
    return resolution_;
  };
  inline double Weight() {
    return weight_;
  };
  inline void SetWeight(double weight) {
    weight_ = weight;
  };
  inline void ReverseWeight() {
    weight_ *= -1.0;
  };
  inline void InvertWeight() {
    weight_ = 1.0/weight_;
  };
  inline unsigned long PixelX() {
    return x_;
  };
  inline unsigned long PixelY() {
    return y_;
  };

  // These methods all relate the hierarchical nature of the pixelization
  // method.  SetToSuperPix degrades a high resolution pixel into the lower
  // resolution pixel that contains it.  SubPix returns either a vector of
  // higher resolution pixels contained by this pixel or the X-Y pixel index
  // bounds that will let one iterate through the sub-pixels without
  // instantiating them.
  bool SetToSuperPix(int lo_resolution);
  void SubPix(int hi_resolution, std::vector<StompPixel>& pix);
  void SubPix(int hi_resolution, unsigned long& x_min, unsigned long& x_max,
	      unsigned long& y_min, unsigned long& y_max);

  // CohortPix returns the pixels at the same resolution that would combine
  // with the current pixel to form the pixel at the next coarser resolution
  // level.
  void CohortPix(StompPixel& pix_a, StompPixel& pix_b, StompPixel& pix_c);

  inline double Area() {
    // Since the pixels are equal-area, we only need to know how many times
    // we've sub-divided to get from the HpixResolution to our current
    // resolution.
    return Stomp::HPixArea()*Stomp::HPixResolution()*Stomp::HPixResolution()/
        (resolution_*resolution_);
  };

  inline unsigned long SuperPix(int lo_resolution) {
    // This returns the index of the pixel that contains the current pixel at
    // a coarser resolution.
    return (resolution_ < lo_resolution ?
            Stomp::Nx0()*Stomp::Ny0()*lo_resolution*lo_resolution :
            Stomp::Nx0()*lo_resolution*
            static_cast<unsigned long>(y_*lo_resolution/resolution_) +
            static_cast<unsigned long>(x_*lo_resolution/resolution_));
  };
  inline unsigned long Superpixnum() {
    // The fundamental unit of the spherical pixelization.  There are 7488
    // superpixels covering the sky, which makes isolating any searches to
    // just the pixels within a single superpixel a very quick localization
    // strategy.

    return Stomp::Nx0()*Stomp::HPixResolution()*
        static_cast<unsigned long>(y_*Stomp::HPixResolution()/resolution_) +
        static_cast<unsigned long>(x_*Stomp::HPixResolution()/resolution_);
  };
  inline unsigned long HPixnum() {
    // Single index ordering within a superpixel.  The cast finds the x-y
    // position of the superpixel and then we scale that up to the
    // pseudo resolution within the superpixel (where HPixResolution() is
    // effectively 0 and we scale up from there.  This is a cheat that lets
    // us single index pixels up to a higher resolution without going to a
    // 64 bit index.

    return
        resolution_/Stomp::HPixResolution()*
        (y_ - resolution_/Stomp::HPixResolution()*
         static_cast<unsigned long>(y_*Stomp::HPixResolution()/resolution_)) +
        (x_ - resolution_/Stomp::HPixResolution()*
         static_cast<unsigned long>(x_*Stomp::HPixResolution()/resolution_));
  };
  unsigned long Pixnum() {
    // Single index ordering for the whole sphere.  Unforunately, the limits of
    // a 32 bit integer only take us up to about 17 arcsecond resolution.

    return Stomp::Nx0()*resolution_*y_ + x_;
  };

  // Given either the X-Y-resolution, StompPixel or AngularCoordinate, return
  // true or false based on whether the implied location is within the current
  // StompPixel.
  inline bool Contains(int pixel_resolution, unsigned long pixel_x,
                unsigned long pixel_y) {
    return ((pixel_resolution >= resolution_) &&
	    (pixel_x*resolution_/pixel_resolution == x_) &&
	    (pixel_y*resolution_/pixel_resolution == y_) ? true : false);
  };
  inline bool Contains(StompPixel& pix) {
    return ((pix.Resolution() >= resolution_) &&
	    (pix.PixelX()*resolution_/pix.Resolution() == x_) &&
	    (pix.PixelY()*resolution_/pix.Resolution() == y_) ? true : false);
  };
  bool Contains(AngularCoordinate& ang);


  // Given an angle in degrees (or upper and lower angular bounds in degrees),
  // return a list of pixels at the same resolution within those bounds.
  void WithinRadius(double theta, std::vector<StompPixel>& pix,
                    bool check_full_pixel=false);
  void WithinAnnulus(double theta_min, double theta_max,
		     std::vector<StompPixel>& pix,
                     bool check_full_pixel=false);

  // Similar to the previous methods, but the return values here are the
  // X-Y indices rather than the pixels themselves.  The second instance allows
  // the X index bounds to vary with Y index value to take into account the
  // effects of curvature on the sphere, while the first instance just uses
  // the outer limits.
  void XYBounds(double theta, unsigned long& x_min, unsigned long& x_max,
		unsigned long& y_min, unsigned long& y_max,
		bool add_buffer = false);
  void XYBounds(double theta, std::vector<unsigned long>& x_min,
		std::vector<unsigned long>& x_max,
		unsigned long& y_min, unsigned long& y_max,
		bool add_buffer = false);
  int EtaStep(double theta);


  // Finishing up the angle-checking methods, we have two more methods that
  // return true or false based on whether the current pixel is within a given
  // angular range of a point on the sphere (specified by either a raw angular
  // coordinate or the center point of another StompPixel).
  bool IsWithinRadius(AngularCoordinate& ang, double theta,
                      bool check_full_pixel=false);
  bool IsWithinRadius(StompPixel& pix, double theta,
                      bool check_full_pixel=false);
  bool IsWithinAnnulus(AngularCoordinate& ang, double theta_min,
                       double theta_max, bool check_full_pixel=false);
  bool IsWithinAnnulus(StompPixel& pix, double theta_min, double theta_max,
                       bool check_full_pixel=false);


  // A hold-over from the SDSS coordinate system, this converts the current
  // pixel index into an SDSS stripe number.  Although this is generally not
  // useful information in an of itself, stripe number is used as a proxy
  // for constructing roughly square subsections of StompMaps.
  int Stripe(int resolution = Stomp::HPixResolution());

  // Some methods for extracting the angular position of the pixel center...
  double RA();
  double DEC();
  double GalLon();
  double GalLat();
  inline void Ang(AngularCoordinate& ang) {
    ang.SetSurveyCoordinates(90.0 - Stomp::Rad2Deg()*
                             acos(1.0-2.0*(y_+0.5)/(Stomp::Ny0()*resolution_)),
                             Stomp::Rad2Deg()*(2.0*Stomp::Pi()*(x_+0.5))/
                             (Stomp::Nx0()*resolution_) + Stomp::EtaOffSet());
  };
  inline double Lambda() {
    return 90.0 -
        Stomp::Rad2Deg()*acos(1.0 - 2.0*(y_+0.5)/(Stomp::Ny0()*resolution_));
  }
  inline double Eta() {
    double eta =
        Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x_+0.5)/(Stomp::Nx0()*resolution_) +
        Stomp::EtaOffSet();
    return (eta >= 180.0 ? eta - 360.0 : eta);
  };

  // ... likewise for the Cartesian coordinates on the unit sphere.
  inline double UnitSphereX() {
    return -1.0*sin(Lambda()*Stomp::Deg2Rad());
  };
  inline double UnitSphereY() {
    return cos(Lambda()*Stomp::Deg2Rad())*
        cos(Eta()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereZ() {
    return cos(Lambda()*Stomp::Deg2Rad())*
        sin(Eta()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };


  // Since the pixels are rectangular in survey coordinates, we have meaningful
  // notions of the bounds in lambda-eta space.
  inline double LambdaMin() {
    return 90.0 -
        Stomp::Rad2Deg()*acos(1.0 - 2.0*(y_+1)/(Stomp::Ny0()*resolution_));
  };
  inline double LambdaMax() {
    return 90.0 -
        Stomp::Rad2Deg()*acos(1.0 - 2.0*y_/(Stomp::Ny0()*resolution_));
  };
  inline double EtaMin() {
    double etamin =
        Stomp::Rad2Deg()*2.0*Stomp::Pi()*x_/(Stomp::Nx0()*resolution_) +
        Stomp::EtaOffSet();

    return (etamin >= 180.0 ? etamin - 360.0 : etamin);
  };
  inline double EtaMax() {
    double etamax =
        Stomp::Rad2Deg()*2.0*Stomp::Pi()*(x_+1)/(Stomp::Nx0()*resolution_) +
        Stomp::EtaOffSet();
    return (etamax >= 180.0 ? etamax - 360.0 : etamax);
  };

  // And it can be useful to be able to quickly extract the x-y-z positions of
  // the pixel corners.
  inline double UnitSphereX_UL() {
    return -1.0*sin(LambdaMax()*Stomp::Deg2Rad());
  };
  inline double UnitSphereY_UL() {
    return cos(LambdaMax()*Stomp::Deg2Rad())*
        cos(EtaMin()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereZ_UL() {
    return cos(LambdaMax()*Stomp::Deg2Rad())*
        sin(EtaMin()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereX_UR() {
    return -1.0*sin(LambdaMax()*Stomp::Deg2Rad());
  };
  inline double UnitSphereY_UR() {
    return cos(LambdaMax()*Stomp::Deg2Rad())*
        cos(EtaMax()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereZ_UR() {
    return cos(LambdaMax()*Stomp::Deg2Rad())*
        sin(EtaMax()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereX_LL() {
    return -1.0*sin(LambdaMin()*Stomp::Deg2Rad());
  };
  inline double UnitSphereY_LL() {
    return cos(LambdaMin()*Stomp::Deg2Rad())*
        cos(EtaMin()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereZ_LL() {
    return cos(LambdaMin()*Stomp::Deg2Rad())*
        sin(EtaMin()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereX_LR() {
    return -1.0*sin(LambdaMin()*Stomp::Deg2Rad());
  };
  inline double UnitSphereY_LR() {
    return cos(LambdaMin()*Stomp::Deg2Rad())*
        cos(EtaMax()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };
  inline double UnitSphereZ_LR() {
    return cos(LambdaMin()*Stomp::Deg2Rad())*
        sin(EtaMax()*Stomp::Deg2Rad()+Stomp::EtaPole());
  };


  inline void Iterate(bool wrap_pixel = true) {
    if (x_ == Stomp::Nx0()*resolution_ - 1) {
      x_ = 0;
      if (wrap_pixel == false) y_++;
    } else {
      x_++;
    }
  };
  inline unsigned long PixelX0() {
    // Like PixelX, but this returns the first x value for the current pixel's
    // superpixel (useful for knowing the bounds for SuperPixelBasedOrder'd
    // lists.
    return static_cast<unsigned long>(x_*Stomp::HPixResolution()/resolution_)*
        resolution_/Stomp::HPixResolution();
  };
  inline unsigned long PixelY0() {
    // Same as PixelX0, but for the y index.
    return static_cast<unsigned long>(y_*Stomp::HPixResolution()/resolution_)*
        resolution_/Stomp::HPixResolution();
  };
  inline unsigned long PixelX1() {
    // This would be the x value just beyond the limit for the current pixel's
    // superpixel.  Hence, all the pixels with PixelX0 <= x < PixelX1 are in
    // the same column of superpixels.  For pixels in
    // superpixel = MaxSuperpixum, this is Nx0*_resolution, so it can be used
    // as an iteration bound for all superpixels.
    return PixelX0() + resolution_/Stomp::HPixResolution();
  };
  inline unsigned long PixelY1() {
    // Same as PixelX1, but for the y index.
    return PixelY0() + resolution_/Stomp::HPixResolution();
  };


  // This next block of code is there to provide backwards compatibility
  // to a straight C interface.  True, we're using references which aren't
  // in C, but these methods still allow users to access the basic interfaces
  // without instantiating a StompPixel, which can be handy in some cases.
  static void Ang2Pix(const int resolution, AngularCoordinate& ang,
		      unsigned long& pixnum);
  static void Pix2Ang(int resolution, unsigned long pixnum,
                      AngularCoordinate& ang);
  static void Pix2HPix(int input_resolution, unsigned long input_pixnum,
		       unsigned long& output_hpixnum,
		       unsigned long& output_superpixnum);
  static void HPix2Pix(int input_resolution, unsigned long input_hpixnum,
		       unsigned long input_superpixnum,
		       unsigned long& output_pixnum);
  static void SuperPix(int hi_resolution, unsigned long hi_pixnum,
                       int lo_resolution, unsigned long& lo_pixnum);
  static void SubPix(int lo_resolution, unsigned long hi_pixnum,
		     int hi_resolution, unsigned long& x_min,
		     unsigned long& x_max, unsigned long& y_min,
		     unsigned long& y_max);
  static void NextSubPix(int input_resolution, unsigned long input_pixnum,
			 unsigned long& sub_pixnum1,
			 unsigned long& sub_pixnum2,
			 unsigned long& sub_pixnum3,
			 unsigned long& sub_pixnum4);
  static void AreaIndex(int resolution, double lammin, double lammax,
			double etamin, double etamax, unsigned long& x_min,
			unsigned long& x_max, unsigned long& y_min,
			unsigned long& y_max);
  static void PixelBound(int resolution, unsigned long pixnum, double& lammin,
			 double& lammax, double& etamin, double& etamax);
  static void CohortPix(int resolution, unsigned long hpixnum,
			unsigned long& pixnum1, unsigned long& pixnum2,
			unsigned long& pixnum3);
  static double PixelArea(int resolution) {
    return Stomp::HPixArea()*Stomp::HPixResolution()*Stomp::HPixResolution()/
        (resolution*resolution);
  };
  static int Pix2EtaStep(int resolution, unsigned long pixnum, double theta);
  static void Ang2HPix(int resolution, AngularCoordinate& ang,
		       unsigned long& hpixnum, unsigned long& superpixnum);
  static void HPix2Ang(int resolution, unsigned long hpixnum,
                       unsigned long superpixnum, AngularCoordinate& ang);
  static void XY2HPix(int resolution, unsigned long x, unsigned long y,
                      unsigned long& hpixnum, unsigned long& superpixnum);
  static void HPix2XY(int resolution, unsigned long hpixnum,
		      unsigned long superpixnum, unsigned long& x,
		      unsigned long& y);
  static void SuperHPix(int hi_resolution, unsigned long hi_hpixnum,
                        int lo_resolution, unsigned long& lo_hpixnum);
  static void NextSubHPix(int resolution, unsigned long hpixnum,
			  unsigned long& hpixnum1, unsigned long& hpixnum2,
			  unsigned long& hpixnum3, unsigned long& hpixnum4);
  static void SubHPix(int lo_resolution, unsigned long hi_hpixnum,
		      unsigned long hi_superpixnum, int hi_resolution,
		      unsigned long& x_min, unsigned long& x_max,
		      unsigned long& y_min, unsigned long& y_max);
  static void HPixelBound(int resolution, unsigned long hpixnum,
			  unsigned long superpixnum, double& lammin,
			  double& lammax, double& etamin, double& etamax);
  static void CohortHPix(int resolution, unsigned long hpixnum,
			 unsigned long& hpixnum1, unsigned long& hpixnum2,
			 unsigned long& hpixnum3);
  static double HPixelArea(int resolution) {
    return Stomp::HPixArea()*Stomp::HPixResolution()*Stomp::HPixResolution()/
        (resolution*resolution);
  };
  static int HPix2EtaStep(int resolution, unsigned long hpixnum,
                          unsigned long superpixnum, double theta);
  static void XY2Pix(int resolution, unsigned long x, unsigned long y,
		     unsigned long& pixnum) {
    pixnum = Stomp::Nx0()*resolution*y + x;
  };
  static void Pix2XY(int resolution, unsigned long pixnum,
		     unsigned long& x, unsigned long& y) {
    y = pixnum/(Stomp::Nx0()*resolution);
    x = pixnum - Stomp::Nx0()*resolution*y;
  };

  // Now we've got the various methods to establish ordering on the pixels.
  // LocalOrder is the the simplest, just arranging all of the pixels in
  // vanilla row-column order.  That's useful for some operations where you
  // want to be able to access nearby pixels simply.  However, if you're
  // doing a search on a large region of the sky, it often helps to be able to
  // limit the search more drastically at the outset.  For that, we have
  // SuperPixelBasedOrder where pixels are grouped by their lowest resolution
  // superpixel and then locally sorted within that bound.  This is the
  // default sorting method for the StompMap class to make searching on those
  // maps more efficient.  Finally, we have some methods for checking
  // whether or not we're looking at equivalent pixels, one where the weights
  // associated with the pixels matter and one that's purely geometric.
  static bool LocalOrder(StompPixel pix_a, StompPixel pix_b);
  static bool LocalOrderByReference(StompPixel& pix_a, StompPixel& pix_b);
  static bool SuperPixelBasedOrder(StompPixel pix_a, StompPixel pix_b);
  static bool SuperPixelOrder(StompPixel pix_a, StompPixel pix_b);
  inline static bool WeightedOrder(StompPixel pix_a, StompPixel pix_b) {
    return (pix_a.Weight() < pix_b.Weight() ? true : false);
  };
  inline static bool WeightMatch(StompPixel pix_a, StompPixel pix_b) {
    return ((pix_b.Weight() < pix_a.Weight() + 0.000001) &&
            (pix_b.Weight() > pix_a.Weight() - 0.000001) ? true : false);
  };
  inline static bool WeightedPixelMatch(StompPixel& pix_a, StompPixel& pix_b) {
    return ((pix_a.Resolution() == pix_b.Resolution()) &&
            (pix_a.PixelX() == pix_b.PixelX()) &&
            (pix_a.PixelY() == pix_b.PixelY()) &&
            (pix_b.Weight() < pix_a.Weight() + 0.000001) &&
            (pix_b.Weight() > pix_a.Weight() - 0.000001) ? true : false);
  };
  inline static bool PixelMatch(StompPixel& pix_a, StompPixel& pix_b) {
    return ((pix_a.Resolution() == pix_b.Resolution()) &&
            (pix_a.PixelX() == pix_b.PixelX()) &&
            (pix_a.PixelY() == pix_b.PixelY()) ? true : false);
  };

  // Finally, these methods handle maps consisting of vectors of StompPixels.
  // One could make the argument that this should be in the StompMap class,
  // but one of the primary purposes of these methods is to take a list of
  // pixels where there may be duplication or cases where smaller pixels are
  // within larger pixels and generate a set of pixels that uniquely covers
  // a give region of the sky.  That extra applicability makes it appropriate
  // to put here.  The main method to call is ResolvePixel, which will call
  // ResolveSuperPixel individually for each fo the superpixels covered by
  // the vector of StompPixels.  The resulting vector will be sorted by
  // SuperPixelBasedOrder.
  static void ResolveSuperPixel(std::vector<StompPixel>& pix,
                                bool ignore_weight = false);
  static void ResolvePixel(std::vector<StompPixel>& pix,
                           bool ignore_weight = false);

 private:
  double weight_;
  unsigned long x_, y_;
  int resolution_;
};

typedef std::vector<StompPixel> StompVector;
typedef StompVector::iterator StompIterator;
typedef std::pair<StompIterator,StompIterator> StompPair;

class StompDensityPixel : public StompPixel {
  // In order to do correlation function calculations, we need two floats for
  // each pixel: one to store the fraction of the pixel that's within a
  // given region and another to store the local density in that pixel.  This
  // sub-classes StompPixel, but the usage is going to be somewhat different.
  // StompPixels are taken to be units of a StompMap where the geometry
  // of the map is the union of all of the StompPixels and the StompPixels are
  // not assumed to be at the same resolution level.  StompDensityPixels, OTOH,
  // for the bases for StompDensityMaps where the map is taken to be a regular
  // sampling of some field over a given area.  The total area for the map
  // can be calculated, but operations like determining whether or not a given
  // position is inside or outside the map is not generically available.
 public:
  StompDensityPixel();
  StompDensityPixel(const int resolution, const unsigned long pixnum,
                    const double weight = 0.0, const double density = 0.0);
  StompDensityPixel(const int resolution, const unsigned long hpixnum,
                    const unsigned long superpixnum, const double weight = 0.0,
                    const double density = 0.0);
  StompDensityPixel(AngularCoordinate& ang, const int resolution,
                    const double weight = 0.0, const double density = 0.0);
  StompDensityPixel(const unsigned long x, const unsigned long y,
                    const int resolution, const double weight = 0.0,
                    const double density = 0.0);
  ~StompDensityPixel();
  inline void SetDensity(double input_density) {
    density_ = input_density;
  };
  inline double Density() {
    return density_;
  };
  inline void AddToDensity(double added_density) {
    density_ += added_density;
  };
  inline void ScaleDensity(double scale_factor) {
    density_ *= scale_factor;
  };
  inline void ConvertToOverDensity(double expected_density) {
    density_ = (density_ - expected_density*Weight()*Area())/
        (expected_density*Weight()*Area());
  };

 private:
  double density_;
};

typedef std::vector<StompDensityPixel> StompDensityVector;
typedef StompDensityVector::iterator StompDensityIterator;
typedef std::pair<StompDensityIterator,StompDensityIterator> StompDensityPair;
typedef std::map<const unsigned long, int> RegionMap;


class StompPointPixel : public StompPixel {
  // Our second variation on the StompPixel.  Like StompDensityPixel, the idea
  // here is to use the StompPixel as a scaffold for sampling a field over an
  // area.  Instead of storing a density, however, StompPointPixel stores a
  // vector of WeightedAngularCoordinates.  This allows a vector of
  // StompPointPixels to act as a de facto tree structure for fast
  // pair-counting routines.
 public:
  StompPointPixel();
  StompPointPixel(const int resolution, const unsigned long pixnum,
                  const double weight = 0.0);
  StompPointPixel(const int resolution, const unsigned long hpixnum,
                  const unsigned long superpixnum,
                  const double weight = 0.0);
  StompPointPixel(AngularCoordinate& ang, const int resolution,
                  const double weight = 0.0);
  StompPointPixel(const unsigned long x, const unsigned long y,
                  const int resolution, const double weight = 0.0);
  ~StompPointPixel();
  inline bool AddPoint(AngularCoordinate& ang, double object_weight = 1.0) {
    WeightedAngularCoordinate w_ang(ang.Lambda(),ang.Eta(),
				    object_weight,
				    AngularCoordinate::Survey);
    return AddPoint(w_ang);
  };
  inline bool AddPoint(WeightedAngularCoordinate& ang) {
    if (Contains(ang)) {
      ang_.push_back(ang);
      return true;
    } else {
      return false;
    }
  };
  inline WAngularIterator Begin() {
    return ang_.begin();
  };
  inline WAngularIterator End() {
    return ang_.end();
  };
  inline unsigned long NPoints() {
    return ang_.size();
  };

 private:
  WAngularVector ang_;
};

typedef std::vector<StompPointPixel> StompPointVector;
typedef StompPointVector::iterator StompPointIterator;
typedef std::pair<StompPointIterator,StompPointIterator> StompPointPair;

class StompSubMap {
  // While the preferred interface for interacting with a StompMap is through
  // that class, the actual work is deferred to the StompSubMap class.  Each
  // instance contains all of the pixels for a corresponding superpixel as well
  // as some summary statistics.  All of the operations done on the StompMap
  // end up calling corresponding methods for each of the StompSubMap instances
  // contained in that StompMap.  See the comments around those methods in the
  // StompMap class declaration for an explaination of what each method does.

 public:
  StompSubMap(unsigned long superpixnum);
  ~StompSubMap();
  void AddPixel(StompPixel& pix);
  void Resolve();
  void Initialize();
  bool FindLocation(AngularCoordinate& ang, double& weight);
  double FindUnmaskedFraction(StompPixel& pix);
  double FindAverageWeight(StompPixel& pix);
  void FindMatchingPixels(StompPixel& pix,
			  StompVector& match_pix,
			  bool use_local_weights = false);
  double AverageWeight();
  bool Add(StompVector& pix, int max_resolution, bool drop_single);
  bool Multiply(StompVector& pix, int max_resolution, bool drop_single);
  bool Exclude(StompVector& pix, int max_resolution);
  void ScaleWeight(const double weight_scale);
  void AddConstantWeight(const double add_weight);
  void InvertWeight();
  void Pixels(StompVector& pix);
  void CheckResolution(int resolution);
  void Clear();
  inline unsigned long Superpixnum() {
    return superpixnum_;
  };
  inline StompIterator Begin() {
    return (initialized_ ? pix_.begin() : pix_.end());
  };
  inline StompIterator End() {
    return pix_.end();
  };
  inline double Area() {
    return area_;
  };
  inline bool Initialized() {
    return initialized_;
  };
  inline bool Modified() {
    return modified_;
  };
  inline int MinResolution() {
    return min_resolution_;
  };
  inline int MaxResolution() {
    return max_resolution_;
  };
  inline double MinWeight() {
    return min_weight_;
  };
  inline double MaxWeight() {
    return max_weight_;
  };
  inline double LambdaMin() {
    return lambda_min_;
  };
  inline double LambdaMax() {
    return lambda_max_;
  };
  inline double EtaMin() {
    return eta_min_;
  };
  inline double EtaMax() {
    return eta_max_;
  };
  inline double ZMin() {
    return z_min_;
  };
  inline double ZMax() {
    return z_max_;
  };
  inline unsigned long Size() {
    return size_;
  };

private:
  unsigned long superpixnum_, size_;
  StompVector pix_;
  double area_, lambda_min_, lambda_max_, eta_min_, eta_max_, z_min_, z_max_;
  double min_weight_, max_weight_;
  int min_resolution_, max_resolution_;
  bool initialized_, modified_;
};

typedef std::vector<StompSubMap> SubMapVector;
typedef SubMapVector::iterator SubMapIterator;
typedef std::pair<SubMapIterator,SubMapIterator> SubMapPair;

class StompMap {
  // A StompMap is intended to function as a region on the sky whose geometry
  // is given by a set of StompPixels of various resolutions which combine to
  // cover that area.  Since each StompPixel has an associated weight, the map
  // can also encode a scalar field (temperature, observing depth, local
  // seeing, etc.) over that region.  A StompMap can be combined with other
  // StompMaps with all of the logical operators you would expect (union,
  // intersection and exclusion as well as addition and multiplication of the
  // weights as a function of position).  Likewise, you can test angular
  // positions and pixels against a StompMap to see if they are within it or
  // query the angular extent and area of a StompMap on the Sky.

 public:
  // The preferred constructor for a StompMap takes a vector of StompPixels
  // as its argument.  However, it can be constructed from a properly formatted
  // ASCII text file as well.
  StompMap();
  StompMap(StompVector& pix);
  StompMap(std::string& InputFile,
           const bool hpixel_format = true,
           const bool weighted_map = true);
  ~StompMap();

  // Initialize is called to organize the StompMap internally.  Unless the
  // map is being reset with a new set of pixels, as in the second instance of
  // this method, Initialize should probably never be invoked.
  bool Initialize();
  bool Initialize(StompVector& pix);

  // Simple call to determine if a point is within the current StompMap
  // instance.  If it is, the method returns true and stores the weight of the
  // map at that location in the weight variable.  If not, false is returned
  // and the weight value is meaningless.
  bool FindLocation(AngularCoordinate& ang, double& weight);

  // Given a StompPixel, this returns the fraction of that pixel's area that is
  // contained within the current map (0 <= fraction <= 1).  Alternatively, a
  // vector of pixels can be processed in a single call, in which case a
  // vector of coverages is returned.
  double FindUnmaskedFraction(StompPixel& pix);
  void FindUnmaskedFraction(StompVector& pix,
                            std::vector<double>& unmasked_fraction);

  // Similar to FindUnmaskedFraction, this returns the area-averaged weight of
  // the map over the area covered by the input pixel (or pixels).
  // AverageWeight does the same task, but over the entire StompMap.
  double FindAverageWeight(StompPixel& pix);
  void FindAverageWeight(StompVector& pix,
                         std::vector<double>& average_weight);
  double AverageWeight();

  // This is part of the process for finding the intersection between two maps.
  // For a given pixel, we return the pixels in our map that are contained
  // within that test pixel.  If use_local_weights is set to true, then the
  // pixel weights are set to match the weights in the current map.  If not,
  // then the matching pixels are set to the weight from the test StompPixel.
  void FindMatchingPixels(StompPixel& pix,
			  StompVector& match_pix,
			  bool use_local_weights = false);
  void FindMatchingPixels(StompVector& pix,
			  StompVector& match_pix,
			  bool use_local_weights = false);

  // Return a vector of SuperPixels that cover the StompMap.  This serves two
  // purposes.  First, it acts as a rough proxy for the area of the current
  // map, which can occasionally be useful.  More importantly, all of the real
  // work in a StompMap is done on a superpixel-by-superpixel basis, so this
  // becomes an important thing to know when querying the map.
  void Coverage(StompVector& superpix);

  // Given a requested number of points, return a vector of Poisson random
  // angular positions within the current StompMap's area.
  //
  // If the use_weighted_sampling flag is set to true, then the local weight is
  // taken into account when generating random points.  In this case, a pixel
  // with the same area but twice the weight as another pixel should, in the
  // limit of infinite realizations, have twice as many points as the
  // lower-weighted one.
  void GenerateRandomPoints(AngularVector& ang,
                            unsigned long n_point = 1,
                            bool use_weighted_sampling = false);

  // The book-end to the initialization method that takes an ASCII filename
  // as an argument, this method writes the current map to an ASCII file using
  // the same formatting conventions.
  bool Write(std::string& OutputFile, bool hpixel_format = true,
             bool weighted_map = true);

  // Three simple functions for performing the same operation on the weights
  // of all of the pixels in the current map.  These are prelude to the next
  // set of functions for doing logical and arithmetic operations on StompMaps.
  void ScaleWeight(const double weight_scale);
  void AddConstantWeight(const double add_weight);
  void InvertWeight();

  // Now we begin the core of our class, the ability to treat the StompMap as
  // an abstract object which we can combine with other maps to form arbitrarily
  // complicated representations on the sphere.
  //
  // Starting simple, IngestMap simply takes the area associated with another
  // map and combines it with the current map.  If pixels overlap between the
  // two maps, then the weights are set to the average of the two maps.
  // Returns true if the procedure succeeded, false otherwise.
  bool IngestMap(StompVector& pix, bool destroy_copy = true);
  bool IngestMap(StompMap& stomp_map, bool destroy_copy = true);

  // Now we have intersection.  This method finds the area of intersection
  // between the current map and the argument map and makes that the new area
  // for this map.  Weights are drawn from the current map's values.  If there
  // is no overlapping area, the method returns false and does nothing.  A
  // true response indicates that the area of the current map has changed to
  // the overlapping area between the two and the area is non-zero.
  bool IntersectMap(StompVector& pix);
  bool IntersectMap(StompMap& stomp_map);

  // The inverse of IntersectMap, ExcludeMap removes the area associated with
  // the input map from the current map.  If this process would remove all of
  // the area from the current map, then the method returns false and does not
  // change the current map.  A true response indicates that the input maps has
  // been excluded and area remains.
  bool ExcludeMap(StompVector& pix, bool destroy_copy = true);
  bool ExcludeMap(StompMap& stomp_map, bool destroy_copy = true);

  // Two sets of methods that operate on the weights between two different
  // maps.  The first set adds the weights of the two maps and the second
  // takes their product.  The drop_single boolean indicates whether the
  // non-overlapping area should be excluded (true) or not (false).  If
  // drop_single is set to false, then the areas where the two maps don't
  // overlap will have their weights set to whatever they are in the map that
  // covers that area.
  bool AddMap(StompVector& pix, bool drop_single = true);
  bool AddMap(StompMap& stomp_map, bool drop_single = true);
  bool MultiplyMap(StompVector& pix, bool drop_single = true);
  bool MultiplyMap(StompMap& stomp_map, bool drop_single = true);


  // Like IntersectMap, except that the current map takes on the weight
  // values from the map given as the argument.
  bool ImprintMap(StompVector& pix);
  bool ImprintMap(StompMap& stomp_map);


  // Simple method for returning the vector representation of the current
  // StompMap.  If a superpixel index is given as the second argument, then
  // just the pixels for that superpixel are returned.
  void Pixels(StompVector& pix,
              unsigned long superpixnum = Stomp::MaxSuperpixnum());

  // Resets the StompMap to a completely clean slate.  No pixels, no area.
  void Clear();

  // Simple in-line method for checking to see if the current map has any area
  // in a given superpixel.
  inline bool ContainsSuperpixel(unsigned long superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Initialized() : false);
  };

  // Some general methods for querying the state of the current map.  If called
  // with no argument, then they return the corresponding value for the map as
  // a whole.  If a superpixel index is given as an argument, then the returned
  // value is valid for that superpixel alone.
  inline double Area(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Area() : area_);
  };
  inline int MinResolution(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].MinResolution() : min_resolution_);
  };
  inline int MaxResolution(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].MaxResolution() : max_resolution_);
  };
  inline double MinWeight(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].MinWeight() : min_weight_);
  };
  inline double MaxWeight(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].MaxWeight() : max_weight_);
  };
  inline unsigned long Size(unsigned long superpixnum=Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Size() : size_);
  };

private:
  SubMapVector sub_map_;
  double area_, min_weight_, max_weight_;
  int min_resolution_, max_resolution_;
  unsigned long size_;
};

typedef std::vector<StompMap> MapVector;
typedef MapVector::iterator MapIterator;
typedef std::pair<MapIterator,MapIterator> MapPair;


class StompDensitySubMap {
  // For the StompDensityMap, the sub-map class is more about book-keeping
  // than operations.  They are still assembled around the superpixels, but
  // since the methods need to talk to pixels from different superpixels so
  // often it doesn't make sense to break up the data to the same degree.

 public:
  StompDensitySubMap(unsigned long superpixnum);
  ~StompDensitySubMap();
  inline void AddToArea(int resolution, double weight) {
    area_ +=
        weight*Stomp::HPixArea()*
        Stomp::HPixResolution()*Stomp::HPixResolution()/
        (resolution*resolution);
  };
  inline void AddToDensity(double density) {
    total_density_ += density;
  };
  inline double Area() {
    return area_;
  };
  inline double Density() {
    return (initialized_ ? total_density_/area_ : 0.0);
  };
  inline void SetBegin(StompDensityIterator iter) {
    start_ = iter;
    finish_ = ++iter;
    initialized_ = true;
  };
  inline void SetEnd(StompDensityIterator iter) {
    finish_ = ++iter;
  };
  inline void SetNull(StompDensityIterator iter) {
    null_ = iter;
  };
  inline StompDensityIterator Begin() {
    return (initialized_ ? start_ : null_);
  };
  inline StompDensityIterator End() {
    return (initialized_ ? finish_ : null_);
  };
  inline bool Initialized() {
    return initialized_;
  };

 private:
  unsigned long superpixnum_;
  StompDensityIterator start_;
  StompDensityIterator finish_;
  StompDensityIterator null_;
  bool initialized_;
  double area_, total_density_;
};

typedef std::vector<StompDensitySubMap> DensitySubMapVector;
typedef DensitySubMapVector::iterator DensitySubMapIterator;
typedef std::pair<DensitySubMapIterator,DensitySubMapIterator> DensitySubMapPair;

class StompSection {
  // This is barely a class.  Really, it's just a small object that's necessary
  // for constructing the sub-regions in the StompDensityMap class.

 public:
  StompSection();
  ~StompSection();
  inline void SetMinStripe(int stripe) {
    stripe_min_ = stripe;
  };
  inline void SetMaxStripe(int stripe) {
    stripe_max_ = stripe;
  };
  inline int MinStripe() {
    return stripe_min_;
  };
  inline int MaxStripe() {
    return stripe_max_;
  };

 private:
  int stripe_min_, stripe_max_;
};


class StompDensityMap {
  // Unlike a StompMap, where the set of StompPixels is intended to match the
  // geometry of a particular region, StompDensityMaps are intended to be
  // a regular sampling map of a given scalar field over some region.  The
  // area covered by the map will be approximately the same as that covered
  // by the pixels in the map, but each pixel is assumed to have some covering
  // fraction to indicate what percentage of the map is in the underlying
  // region.  To phrase things another way, once you have a StompMap describing
  // the extent of some data set, a StompDensityMap is what you would use to
  // calculate clustering statistics on data contained in that region.

 public:
  // The StompDensityPixels in a StompDensityMap all have the same resolution.
  // Hence, we can construct a blank map from a StompMap (essentially
  // re-sampling the StompMap at a fixed resolution) or another StompDensityMap,
  // so long as that map has higher resolution than the one we're trying to
  // construct.
  //
  // The region_resolution parameter is a bit of forward planning that you
  // need to do at instantiation.  For the purposes of error calculation,
  // it's useful to be able to split your map up into sub-regions of nearly
  // the same area and shape.  region_resolution sets the resolution scale for
  // doing this splitting.  There's a memory-performance trade-off in choosing
  // the scale here.  The higher the resolution, the closer you'll get to
  // equal-area sub-regions, but you'll incur a higher memory cost.  This
  // memory isn't allocatated unless you initialize the regions, so if you
  // aren't going to use this feature of the class, don't worry about it.
  // If you do use it, the bounds are 4 <= region_resolution <= 256, in powers
  // of 2 (of course).
  StompDensityMap(StompMap& stomp_map,
                  int resolution,
		  double min_unmasked_fraction = 0.0000001,
                  int region_resolution = Stomp::HPixResolution(),
                  bool use_map_weight_as_density = false);
  StompDensityMap(StompDensityMap& density_map,
                  int resolution,
                  double min_unmasked_fraction = 0.0000001,
                  int region_resolution = Stomp::HPixResolution(),
                  bool use_weighted_average_resampling = false);


  // This may seem a bit of an oddity, but needing roughly circular patches
  // from maps comes up more frequently than one might think.  Or not.
  StompDensityMap(StompMap& stomp_map,
		  AngularCoordinate& center,
		  double theta_max,
                  int resolution,
                  double min_unmasked_fraction = 0.0000001,
                  double theta_min = -1.0,
                  int region_resolution = Stomp::HPixResolution());
  ~StompDensityMap();


  // Internal method that should probably never be called.
  void InitializeSubMap();


  // For the purposes of calculating sample variance errors, it can be useful
  // to break up your map into equal area, roughly similarly shaped sub-regions.
  // InitializeRegions does this, given a requested number of sub-regions.
  void InitializeRegions(int n_region);


  // Once we have our map set up, we'll want to add data points to it.  This
  // method offers two variations on that task.
  bool AddToMap(AngularCoordinate& ang, double object_weight = 1.0);
  bool AddToMap(WeightedAngularCoordinate& ang);


  // The equivalent of the same methods from the StompMap class.
  void Coverage(StompVector& superpix);
  double FindUnmaskedFraction(StompPixel& pix);
  double FindDensity(StompPixel& pix);
  bool Write(std::string& OutputFile, bool hpixel_format = true,
             bool include_weight = true);
  inline void Clear() {
    if (pix_.empty() == false) pix_.clear();
    if (sub_map_.empty() == false) sub_map_.clear();
    if (region_map_.empty() == false) region_map_.clear();
  };


  // These two methods allow you to sample the area and density within a given
  // annular radius around a point (where the inner radius is set to zero by
  // default.  The angular bounds (theta_min and theta_max) are taken to be in
  // degrees.
  double FindLocalDensity(AngularCoordinate& ang, double theta_max,
			  double theta_min = -1.0);
  double FindLocalArea(AngularCoordinate& ang, double theta_max,
		       double theta_min = -1.0);


  // If we're converting a map from high to low resolution, this method
  // re-calculates the weight and density parameters for a given lower
  // resolution pixel.  The appropriate value for the use_weighted_average
  // input depends on the nature of the current map.  If the map is additive
  // (e.g. the density parameter in the map represents the number of galaxies
  // in a given pixel), then resampling should just sum the parts and
  // use_weighted_average should be set to false.  If the map is sampling a
  // continuous field (e.g. a temperature or over-density field), then the
  // resampled value should be the average of the higher resolution values and
  // use_weighted_average should be set to true.
  void Resample(StompDensityPixel& pix, bool use_weighted_average = false);


  // In the case where one is adding data points to the map, once this is
  // done, those object counts will need to be translated into a local measure
  // of the fractional over-density.  If the mean density of the map is all
  // that's required, then the first method will do that.  If you want to
  // replace the current data counts with the fractional over-density the
  // second method will do that (and call the first method if you have not
  // already done so).
  void CalculateMeanDensity();
  void ConvertToOverDensity();


  // Given a StompMap, we export the density field in the current
  // StompDensityMap into its weight values.  This will naturally perform an
  // intersection between the areas covered by the two maps.  If there is no
  // overlapping area, then the method returns false and the input StompMap
  // will not be modified.  A true result means that there was at least some
  // overlap.
  bool ImprintMap(StompMap& stomp_map);


  // This method calculates the auto-correlation of the data in the
  // current map.  In both cases, the method takes an AngularCorrelation object
  // instance as an object.  This object contains a range of angular scales,
  // each of which have a given resolution level assigned to them.  The method
  // will look for the angular bins that should be measured with the resolution
  // of the current map and make the measurement based on the current map's
  // data and the angular bins involved.
  void AutoCorrelate(AngularCorrelation& wtheta);

  // Same as AutoCorrelate, but this uses the bounds in the region structure
  // to do jack-knife re-sampling of the data set.  This iteratively removes
  // part of the data set, does the calculation, replaces the missing chunk,
  // and then re-does the calculation after removing the next region's data.
  // This results in a set of measurements which have nearly the same area as
  // the total sample (so boundary effects are reduced compared to doing the
  // measurement in sub-samples alone), but which are also nearly degenerate
  // with each other.  The reduced variance in these measurements can give one
  // a rough-and-ready notion of the sample variance errors for the measurement.
  //
  // TODO: This method has not been implemented yet.
  void AutoCorrelateUsingRegions(WThetaVector& wtheta);

  // Same as the auto-correlation methods, except the current map is
  // cross-correlated with another StompDensityMap.  Only areas of overlap are
  // considered in the cross-correlation.
  //
  // TODO: The regions-based method is not implemented yet.
  void CrossCorrelate(StompDensityMap& stomp_map, AngularCorrelation& wtheta);
  void CrossCorrelateUsingRegions(StompDensityMap& stomp_map,
				  WThetaVector& wtheta);

  // Meaningful, since all of the pixels in the map share a common resolution.
  inline int Resolution() {
    return resolution_;
  };
  inline int NRegion() {
    return n_region_;
  };

  // Given a region_idx, this returns the corresponding region value for that
  // pixel.  region_idx is equivalent to the value given by the Pixnum method
  // in the StompPixel class.
  inline int Region(unsigned long region_idx) {
    return region_map_[region_idx];
  };
  inline double Area(unsigned long superpixnum = Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Area() : area_);
  };
  inline double Density(unsigned long superpixnum = Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Density() : total_density_/area_);
  };
  inline StompDensityIterator Begin(unsigned long superpixnum =
			     Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].Begin() : pix_.begin());
  };
  inline StompDensityIterator End(unsigned long superpixnum =
			   Stomp::MaxSuperpixnum()) {
    return (superpixnum < Stomp::MaxSuperpixnum() ?
            sub_map_[superpixnum].End() : pix_.end());
  };
  inline unsigned long Size() {
    return pix_.size();
  };
  inline double MeanDensity() {
    if (calculated_mean_density_ == false) CalculateMeanDensity();
    return mean_density_;
  };
  inline bool IsOverDensityMap() {
    return converted_to_overdensity_;
  }

 private:
  StompDensityVector pix_;
  DensitySubMapVector sub_map_;
  RegionMap region_map_;
  double area_, mean_density_, unmasked_fraction_minimum_, total_density_;
  int resolution_, region_resolution_, n_region_;
  bool converted_to_overdensity_, calculated_mean_density_;
  bool initialized_sub_map_, initialized_region_map_;
};

typedef std::vector<StompDensityMap> DensityMapVector;
typedef DensityMapVector::iterator DensityMapIterator;
typedef std::pair<DensityMapIterator,DensityMapIterator> DensityMapPair;

class FootprintBound {
  // This is the base class for generating footprints.  A footprint object is
  // essentially a scaffolding around the StompMap class that contains the
  // methods necessary for converting some analytic expression of a region on
  // a sphere into the equivalent StompMap.  All footprints do roughly the same
  // operations to go about this task, but the details differ based on how the
  // analytic decription is implemented.  This is a true abstract class and
  // should never actually be instantiated.  Instead, you should derive classes
  // from this one that replace the virtual methods with ones that are
  // appropriate to your particular footprint geometric description.

 public:
  FootprintBound();
  virtual ~FootprintBound();

  // All footprint derived classes need to replace these virtual methods
  // with ones specific to their respective geometries.  You need a method
  // for saying whether or not a point on the sphere is inside or outside of
  // your area, you need a method to give the footprint object an idea of where
  // to start looking for pixels that might be in your footprint and you need
  // a way to calculate the area of your footprint so that the class can figure
  // out how closely the area of its pixelization of your footprint matches
  // the analytic value.
  virtual bool CheckPoint(AngularCoordinate& ang);
  virtual bool FindAngularBounds();
  virtual bool FindArea();

  // The pixelization method is iteratively adaptive.  First, it tries to find
  // the largest pixels that will likely fit inside the footprint.  Then it
  // checks those pixels against the footprint and keeps the ones that are
  // fully inside the footprint.  For the ones that were at least partially
  // inside the footprint, it refines them to the next resolution level and
  // tests the sub-pixels.  Those that pass are kept, the misses are discarded
  // and the partials are refined again.  This continues until we reach the
  // maximum resolution level, at which point we keep enough of the partials
  // to match the footprint's area.  When doing the job of pixelizing a given
  // footprint, these three methods should be called subsquently, with the
  // output of FindStartingResolution fed into FindXYBounds as its
  // argument.  A false return value for either FindXYBounds or Pixelize
  // indicates a failure in the corresponding step.
  int FindStartingResolution();
  bool FindXYBounds(const int resolution);
  bool Pixelize();

  // Part of the pixelization process is figuring out what fraction of a
  // given pixel is within the bounds delineated by the footprint's geometry.
  // Pixels are scored on a scale from -1 <= score <= 0, with -1 indicating
  // that the pixel is completely inside of the bounds and 0 indicating that
  // it's completely outside.  This allows one to sort pixels by their score
  // and keep the ones that are closest to being completely inside the
  // footprint bounds.
  double ScorePixel(StompPixel& pix);

  // Once we've pixelized the footprint, we want to return a StompMap
  // representing the results.  This method returns a pointer to that map.
  inline StompMap::StompMap* StompMap() {
    return new StompMap::StompMap(pix_);
  }

  inline void StompMap(StompMap::StompMap& smap) {
	  smap.Initialize(pix_);
  }



  inline void SetMaxResolution(int resolution = Stomp::MaxPixelResolution()) {
    max_resolution_ = resolution;
  }

  // Since we store the area and pixelized area in this class, we need methods
  // for setting and getting those values.  Likewise with the weight that will
  // be assigned to the StompPixels that will make up the StompMap that results.
  inline void SetArea(double input_area) {
    area_ = input_area;
  };
  inline double Area() {
    return area_;
  };
  inline void AddToPixelizedArea(int resolution) {
    pixel_area_ += StompPixel::PixelArea(resolution);
  };
  inline double Weight() {
    return weight_;
  };
  inline void SetWeight(double input_weight) {
    weight_ = input_weight;
  };
  inline double PixelizedArea() {
    return pixel_area_;
  };
  inline unsigned long NPixel() {
    return pix_.size();
  };
  inline void SetAngularBounds(double lammin, double lammax,
                               double etamin, double etamax) {
    lammin_ = lammin;
    lammax_ = lammax;
    etamin_ = etamin;
    etamax_ = etamax;
  };
  inline double LambdaMin() {
    return lammin_;
  };
  inline double LambdaMax() {
    return lammax_;
  };
  inline double EtaMin() {
    return etamin_;
  };
  inline double EtaMax() {
    return etamax_;
  };
  inline unsigned long XMin() {
    return x_min_;
  };
  inline unsigned long XMax() {
    return x_max_;
  };
  inline unsigned long YMin() {
    return y_min_;
  };
  inline unsigned long YMax() {
    return y_max_;
  };
  inline StompIterator Begin() {
    return pix_.begin();
  };
  inline StompIterator End() {
    return pix_.end();
  };
  inline void Clear() {
    pix_.clear();
  };


 private:
  StompVector pix_;
  int max_resolution_;
  double area_, pixel_area_, lammin_, lammax_, etamin_, etamax_, weight_;
  unsigned long x_min_, x_max_, y_min_, y_max_;
};

class CircleBound : public FootprintBound {
  // An example of a derived FootprintBound class.  This implements a simple
  // circular footprint of a given radius (in degrees) around a central
  // angular position.

 public:
  CircleBound(const AngularCoordinate& ang, double radius, double weight);
  virtual ~CircleBound();
  virtual bool CheckPoint(AngularCoordinate& ang);
  virtual bool FindAngularBounds();
  virtual bool FindArea();

 private:
  AngularCoordinate ang_;
  double radius_, sin2radius_;
};

typedef std::vector<CircleBound> CircleVector;
typedef CircleVector::iterator CircleIterator;


class PolygonBound : public FootprintBound {
  // Another derived FootprintBoundClass, this one for a spherical polygon
  // represented by a vector of vertices.  In this case, the vertices need to
  // be in clockwise order as seen from outside the sphere, i.e. the order
  // is right-handed when your thumb is pointed towards the center of the
  // sphere.

 public:
  PolygonBound(AngularVector& ang, double weight);
  virtual ~PolygonBound();
  virtual bool CheckPoint(AngularCoordinate& ang);
  virtual bool FindAngularBounds();
  virtual bool FindArea();
  inline bool DoubleGE(const double x, const double y) {
    static double tolerance = 1.0e-10;
    return (x >= y - tolerance ? true : false);
  };
  inline bool DoubleLE(const double x, const double y) {
    static double tolerance = 1.0e-10;
    return (x <= y + tolerance ? true : false);
  };

 private:
  AngularVector ang_;
  std::vector<double> x_, y_, z_, dot_;
  unsigned long n_vert_;
};

typedef std::vector<PolygonBound> PolygonVector;
typedef PolygonVector::iterator PolygonIterator;
