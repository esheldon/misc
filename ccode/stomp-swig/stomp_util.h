#ifndef STOMP_UTIL_H
#define STOMP_UTIL_H

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include "MersenneTwister.h"

namespace Stomp {

class Stomp;
class AngularCoordinate;
class WeightedAngularCoordinate;
class AngularBin;
class AngularCorrelation;
class Pixel;
class ScalarPixel;
class TreePixel;
class TreeNeighbor;
class NearestNeighborPixel;
class NearestNeighborPoint;
class BaseMap;
class RegionMap;
class Map;
class SubMap;
class ScalarMap;
class ScalarSubMap;
class TreeMap;
class Section;
class FootprintBound;
class CircleBound;
class PolygonBound;

class Stomp {
  // This is our singleton class for storing all of the constants that define
  // our pixelization scheme.  At its most basic, the pixelization is an
  // equal-area rectangular scheme.  This makes calculations like total area
  // and density simple, but it does mean that you encounter significant
  // pixel distortion as you approach the poles.  You should never actually
  // instantiate this class, but just call its methods directly.

 public:
  static const double Pi;
  static const double DegToRad;
  static const double RadToDeg;
  static const double StradToDeg;
  static const uint32_t Nx0;
  static const uint32_t Ny0;

  // For historical reasons, coordinate system is built around the SDSS
  // survey coordinates rather than traditional equatorial RA-DEC coordinates.
  // To switch to those coordinates, the next five functions would need to be
  // modified so that EtaOffSet, Node and EtaPole all return zero.
  static const double EtaOffSet;
  static const double SurveyCenterRA;
  static const double SurveyCenterDEC;
  static const double Node;
  static const double EtaPole;

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
  static const uint8_t HPixLevel;
  static const uint8_t MaxPixelLevel;
  static const uint16_t HPixResolution;
  static const uint16_t MaxPixelResolution;
  static const uint8_t ResolutionLevels;
  static const double HPixArea;
  static const uint32_t MaxPixnum;
  static const uint32_t MaxSuperpixnum;
  inline static bool DoubleLT(double a, double b) {
    return (a < b - 1.0e-10 ? true : false);
  };
  inline static bool DoubleLE(double a, double b) {
    return (a <= b + 1.0e-10 ? true : false);
  };
  inline static bool DoubleGT(double a, double b) {
    return (a > b + 1.0e-10 ? true : false);
  };
  inline static bool DoubleGE(double a, double b) {
    return (a >= b - 1.0e-10 ? true : false);
  };
  inline static bool DoubleEQ(double a, double b) {
    return (DoubleLE(a, b) && DoubleGE(a, b) ? true : false);
  };
  inline static void Tokenize(const std::string& str,
			      std::vector<std::string>& tokens,
			      const std::string& delimiters = " ") {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
  };
  inline static uint8_t MostSignificantBit(uint32_t input_int) {
    uint8_t ln_int = 0;
    while (input_int >>= 1) ln_int++;
    return ln_int;
  };
};

class Cosmology {
  // Some cosmological parameters.  We want to be able to convert our angular
  // scales into co-moving angular distances, so we need a few cosmological
  // parameters.  This needs to be done in an analytic manner, so we're limited
  // to a flat, LambdaCDM model where we can use a fitting function for
  // the co-moving distance as a function of redshift (M. Lampton 2005).  We'll
  // default to values from the WMAP5 cosmology, but these can be modified
  // if necessary.
 public:
  static double omega_m_;
  static double h_;
  static double s_;
  static double a_;
  static double b_;
  static const double AA_;
  static const double BB_;

  inline static double OmegaM() {
    return omega_m_;
  };
  inline static double HubbleConstant() {
    return h_*100.0;
  };
  inline static double HubbleDistance() {
    return 3000.0/h_;
  };
  inline static double OmegaL() {
    return 1.0 - omega_m_;
  };
  inline static void SetOmegaM(double omega_m) {
    omega_m_ = omega_m;
    a_ = AA_*omega_m_;
    b_ = BB_*sqrt(omega_m_);
  };
  inline static void SetHubbleConstant(double hubble) {
    h_ = hubble/100.0;
  };
  inline static void SetOmegaL(double omega_lambda) {
    omega_m_ = 1.0 - omega_lambda;
    a_ = AA_*omega_m_;
    b_ = BB_*sqrt(omega_m_);
  };
  inline static double ComovingDistance(double z) {
    // In Mpc/h.
    return HubbleDistance()*z/sqrt(1.0 + a_*z + b_*z*z);
  };
  inline static double AngularDiameterDistance(double z) {
    // In Mpc/h.
    return ComovingDistance(z)/(1.0+z);
  };
  inline static double LuminosityDistance(double z) {
    // In Mpc/h.
    return ComovingDistance(z)*(1.0+z);
  };
  inline static double ProjectedDistance(double z, double theta) {
    // where theta is assumed to be in degrees and the return value in Mpc/h.
    return theta*Stomp::DegToRad*AngularDiameterDistance(z);
  };
  inline static double ProjectedAngle(double z, double radius) {
    // where radius is in Mpc/h and the return angle is in degrees.
    return Stomp::RadToDeg*radius/AngularDiameterDistance(z);
  };
};

typedef std::vector<AngularCoordinate> AngularVector;
typedef AngularVector::iterator AngularIterator;
typedef std::vector<AngularCoordinate *> AngularPtrVector;
typedef AngularPtrVector::iterator AngularPtrIterator;

class AngularCoordinate {
  // Our generic class for handling angular positions.  The idea is that
  // locations on the celestial sphere should be abstract objects from which
  // you can draw whatever angular coordinate pair is necessary for a given
  // use case.  AngularCoordinate's can be instantiated with a particular
  // coordinate system in mind or that can be set later on.
 public:
  enum Sphere {
    Survey,
    Equatorial,
    Galactic
  };
  AngularCoordinate(double theta = 0.0, double phi = 0.0,
                    Sphere sphere = Survey);
  AngularCoordinate(double unit_sphere_x, double unit_sphere_y,
                    double unit_sphere_z);
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
  void SetGalacticCoordinates(double gal_lon, double gal_lat);
  void SetUnitSphereCoordinates(double unit_sphere_x, double unit_sphere_y,
				double unit_sphere_z);

  // The basic methods for extracting each of the angular coordinate values.
  inline double Lambda() {
    return -1.0*asin(us_x_)*Stomp::RadToDeg;
  };
  inline double Eta() {
    double eta = (atan2(us_z_, us_y_) - Stomp::EtaPole)*Stomp::RadToDeg;
    return (Stomp::DoubleLT(eta, 180.0) && Stomp::DoubleGT(eta, -180.0) ? eta :
	    (Stomp::DoubleGE(eta, 180) ? eta - 360.0 : eta + 360.0)) ;
  };
  inline double RA() {
    double ra = (atan2(us_y_, us_x_) + Stomp::Node)*Stomp::RadToDeg;
    return (Stomp::DoubleLT(ra, 360.0) && Stomp::DoubleGT(ra, 0.0) ? ra :
	    (Stomp::DoubleGE(ra, 360) ? ra - 360.0 : ra + 360.0)) ;
  };
  inline double DEC() {
    return asin(us_z_)*Stomp::RadToDeg;
  };
  inline double GalLon() {
    double gal_lon, gal_lat;
    EquatorialToGalactic(RA(), DEC(), gal_lon, gal_lat);

    return gal_lon;
  };
  inline double GalLat() {
    double gal_lon, gal_lat;
    EquatorialToGalactic(RA(), DEC(), gal_lon, gal_lat);

    return gal_lat;
  };

  // And the associated methods for doing the same with the unit sphere
  // Cartesian coordinates.
  inline double UnitSphereX() {
    return us_x_;
  };
  inline double UnitSphereY() {
    return us_y_;
  };
  inline double UnitSphereZ() {
    return us_z_;
  };

  // Once you have a single angular position on the sphere, very quickly you
  // end up wanting to know the angular distance between your coordinate and
  // another.  This method returns that value in degrees.
  inline double AngularDistance(AngularCoordinate& ang) {
    return acos(us_x_*ang.UnitSphereX() + us_y_*ang.UnitSphereY() +
                us_z_*ang.UnitSphereZ())*Stomp::RadToDeg;
  };
  inline double AngularDistance(AngularCoordinate* ang) {
    return acos(us_x_*ang->UnitSphereX() + us_y_*ang->UnitSphereY() +
                us_z_*ang->UnitSphereZ())*Stomp::RadToDeg;
  };

  // And these two methods return the dot-product and cross-product between
  // the unit vector represented by your angular position and another.
  inline double DotProduct(AngularCoordinate& ang) {
    return us_x_*ang.UnitSphereX() + us_y_*ang.UnitSphereY() +
      us_z_*ang.UnitSphereZ();
  };
  inline double DotProduct(AngularCoordinate* ang) {
    return us_x_*ang->UnitSphereX() + us_y_*ang->UnitSphereY() +
      us_z_*ang->UnitSphereZ();
  };
  inline AngularCoordinate CrossProduct(AngularCoordinate& ang) {
    return AngularCoordinate(us_y_*ang.UnitSphereZ() - us_z_*ang.UnitSphereY(),
			     us_x_*ang.UnitSphereZ() - us_z_*ang.UnitSphereX(),
			     us_x_*ang.UnitSphereY() - us_y_*ang.UnitSphereX());
  };
  inline AngularCoordinate CrossProduct(AngularCoordinate* ang) {
    return AngularCoordinate(us_y_*ang->UnitSphereZ()-us_z_*ang->UnitSphereY(),
			     us_x_*ang->UnitSphereZ()-us_z_*ang->UnitSphereX(),
			     us_x_*ang->UnitSphereY()-us_y_*ang->UnitSphereX());
  };

  // Given another AngularCoordinate, return the AngularCoordinate that defines
  // the great circle running through the current point and the input point.
  // This is different than just the cross-product in that we need to do the
  // calculation in the native coordinate system of the current point.
  void GreatCircle(AngularCoordinate& ang, AngularCoordinate& great_circle);

  // Given either another AngularCoordinate or a Pixel, return the position
  // angle (in degrees, East of North) from the current coordinate to the input
  // location on the sphere.  This angle will, of course, depend on the
  // projection of the current AngularCoordinate.  We also provide variants
  // where the return values are sines and cosines of the position angle.
  double PositionAngle(AngularCoordinate& ang, Sphere sphere = Equatorial);
  double PositionAngle(Pixel& pix, Sphere sphere = Equatorial);

  double CosPositionAngle(AngularCoordinate& ang, Sphere sphere = Equatorial);
  double CosPositionAngle(Pixel& pix, Sphere sphere = Equatorial);

  double SinPositionAngle(AngularCoordinate& ang, Sphere sphere = Equatorial);
  double SinPositionAngle(Pixel& pix, Sphere sphere = Equatorial);

  // Given an input position on the sphere, we can rotate our current position
  // about that point.  The rotation angle will be in degrees, East of
  // North.  In the second variant, we leave our current position static and
  // return a copy of the rotated position.
  void Rotate(AngularCoordinate& fixed_ang, double rotation_angle);
  void Rotate(AngularCoordinate& fixed_ang, double rotation_angle,
	      AngularCoordinate& rotated_ang);

  // Static methods for when users want to switch between coordinate systems
  // without instantiating the class.
  static void SurveyToGalactic(double lambda, double eta,
                               double& gal_lon, double& gal_lat);
  static void SurveyToEquatorial(double lambda, double eta,
                                 double& ra, double& dec);
  static void EquatorialToSurvey(double ra, double dec,
                                 double& lambda, double& eta);
  static void EquatorialToGalactic(double ra, double dec,
                                   double& gal_lon, double& gal_lat);
  static void GalacticToSurvey(double gal_lon, double gal_lat,
                               double& lambda, double& eta);
  static void GalacticToEquatorial(double gal_lon, double gal_lat,
                                   double& ra, double& dec);
  static void SurveyToXYZ(double lambda, double eta,
			  double& x, double& y, double& z);
  static void EquatorialToXYZ(double ra, double dec,
			      double& x, double& y, double& z);
  static void GalacticToXYZ(double gal_lon, double gal_lat,
			    double& x, double& y, double& z);

  // This is a bit more obscure.  The idea here is that, when you want to find
  // the pixel bounds that subtend a given angular scale about a point on the
  // sphere, finding those bounds in latitude is easier than in longitude.
  // Given a latitude in one of the coordinate systems, the multiplier tells
  // you how many more pixels you should check in the longitude direction
  // relative to the latitude direction.
  static double EtaMultiplier(double lam) {
    return 1.0 +
      lam*lam*(0.000192312 - lam*lam*(1.82764e-08 + 1.28162e-11*lam*lam));
  };
  static double RAMultiplier(double dec) {
    return 1.0 +
      dec*dec*(0.000192312 - dec*dec*(1.82764e-08 + 1.28162e-11*dec*dec));
  };
  static double GalLonMultiplier(double glat) {
    return 1.0 +
      glat*glat*(0.000192312 - glat*glat*(1.82764e-08 + 1.28162e-11*glat*glat));
  };

  // Finally, some operator overloading to handle input and output.  Based on
  // ubiquity, all I/O is done assuming that coordinates are in an Equatorial
  // projection.  The input and output are assumed to be whitespace-separated
  // columns in long-lat order.
  friend std::ostream& operator<<(std::ostream& out,
				  AngularCoordinate& ang) {
    out << ang.RA() << " " << ang.DEC();
    return out;
  }
  friend std::istream& operator>>(std::istream& in, AngularCoordinate& ang) {
    double ra;
    double dec;

    in >> ra >> dec;
    ang.SetEquatorialCoordinates(ra, dec);
    return in;
  }

 private:
  double us_x_, us_y_, us_z_;
};

typedef std::map<std::string, double> FieldDict;
typedef FieldDict::iterator FieldIterator;
typedef std::vector<WeightedAngularCoordinate> WAngularVector;
typedef WAngularVector::iterator WAngularIterator;
typedef std::vector<WeightedAngularCoordinate *> WAngularPtrVector;
typedef WAngularPtrVector::iterator WAngularPtrIterator;

class WeightedAngularCoordinate : public AngularCoordinate {
  // Sub-class of AngularCoordinate where we attach a weight value to that
  // angular position.

 public:
  WeightedAngularCoordinate();
  WeightedAngularCoordinate(double theta, double phi,
			    double weight, Sphere sphere = Survey);
  WeightedAngularCoordinate(double unit_sphere_x, double unit_sphere_y,
			    double unit_sphere_z, double weight);
  ~WeightedAngularCoordinate();
  inline void SetWeight(double weight) {
    weight_ = weight;
  };
  inline double Weight() {
    return weight_;
  };
  inline void SetField(const std::string& field_name, double weight) {
    field_[field_name] = weight;
  };
  inline double Field(const std::string& field_name) {
    return (field_.find(field_name) != field_.end() ? field_[field_name] : 0.0);
  };
  inline uint16_t NFields() {
    return field_.size();
  };
  inline bool HasFields() {
    return (field_.size() > 0 ? true : false);
  };
  inline void FieldNames(std::vector<std::string>& field_names) {
    field_names.clear();
    for (FieldIterator iter=field_.begin();iter!=field_.end();++iter)
      field_names.push_back(iter->first);
  };
  inline FieldIterator FieldBegin() {
    return field_.begin();
  };
  inline FieldIterator FieldEnd() {
    return field_.end();
  };
  inline void CopyFields(WeightedAngularCoordinate& w_ang) {
    for (FieldIterator iter=w_ang.FieldBegin();iter!=w_ang.FieldEnd();++iter)
      field_[iter->first] = iter->second;
  };
  inline void CopyFields(WeightedAngularCoordinate* w_ang) {
    for (FieldIterator iter=w_ang->FieldBegin();iter!=w_ang->FieldEnd();++iter)
      field_[iter->first] = iter->second;
  };
  inline void CopyFieldToWeight(const std::string& field_name) {
    if (field_.find(field_name) != field_.end()) {
      // Provided that the input field exists, place a copy of the Weight
      // value into the Field dictionary in case we want to revert to the
      // original value.
      _BackUpWeight();
      weight_ = field_[field_name];
    } else {
      weight_ = 0.0;
    }
  };
  inline void _BackUpWeight() {
    std::string temporary_field_name = "temporary_field_name_DO_NOT_USE";
    if (field_.find(temporary_field_name) == field_.end()) {
      // If this key doesn't already exist, then we've got the original Weight
      // value.  If not, then we must have already stored a copy, so we'll
      // ignore this request.
      field_[temporary_field_name] = weight_;
    }
  };
  inline void RestoreOriginalWeight() {
    // Provided that we can find the back-up copy of the original weight,
    // copy that value back into the Weight variable.
    std::string temporary_field_name = "temporary_field_name_DO_NOT_USE";
    if (field_.find(temporary_field_name) != field_.end()) {
      weight_ = field_[temporary_field_name];
    }
  };

 private:
  double weight_;
  FieldDict field_;
};

typedef std::vector<AngularBin> ThetaVector;
typedef ThetaVector::iterator ThetaIterator;
typedef std::pair<ThetaIterator,ThetaIterator> ThetaPair;
typedef std::vector<AngularBin *> ThetaPtrVector;
typedef ThetaPtrVector::iterator ThetaPtrIterator;

class AngularBin {
  // Class object for holding the data associated with a single angular
  // annulus.  Each instance of the class contains a lower and upper angular
  // limit that defines the annulus as well as methods for testing against
  // those limits and data fields that are used for calculating angular
  // auto-correlations and cross-correlations via the AngularCorrelation
  // class described next.  All of the methods for this class are implemented
  // in this header file.

 public:
  AngularBin() {
    theta_min_ = theta_max_ = sin2theta_min_ = sin2theta_max_ = 0.0;
    costheta_min_ = costheta_max_ = 1.0;
    weight_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    pixel_wtheta_ = pixel_weight_ = wtheta_ = wtheta_error_ = 0.0;
    counter_ = 0;
    resolution_ = 0;
    ClearRegions();
    set_wtheta_ = false;
    set_wtheta_error_ = false;
  };
  AngularBin(double theta_min, double theta_max) {
    SetThetaMin(theta_min);
    SetThetaMax(theta_max);
    weight_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    pixel_wtheta_ = pixel_weight_ = wtheta_ = wtheta_error_ = 0.0;
    counter_ = 0;
    ClearRegions();
    resolution_ = 0;
    set_wtheta_ = false;
    set_wtheta_error_ = false;
  };
  AngularBin(double theta_min, double theta_max, int16_t n_regions) {
    SetThetaMin(theta_min);
    SetThetaMax(theta_max);
    weight_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    pixel_wtheta_ = pixel_weight_ = wtheta_ = wtheta_error_ = 0.0;
    counter_ = 0;
    ClearRegions();
    if (n_regions > 0) InitializeRegions(n_regions);
    resolution_ = 0;
    set_wtheta_ = false;
    set_wtheta_error_ = false;
  };
  ~AngularBin() {
    theta_min_ = theta_max_ = sin2theta_min_ = sin2theta_max_ = 0.0;
    weight_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    pixel_wtheta_ = pixel_weight_ = wtheta_ = wtheta_error_ = 0.0;
    counter_ = 0;
    ClearRegions();
    resolution_ = 0;
    set_wtheta_ = false;
    set_wtheta_error_ = false;
  };
  inline void ClearRegions() {
    weight_region_.clear();
    gal_gal_region_.clear();
    gal_rand_region_.clear();
    rand_gal_region_.clear();
    rand_rand_region_.clear();
    pixel_wtheta_region_.clear();
    pixel_weight_region_.clear();
    wtheta_region_.clear();
    wtheta_error_region_.clear();
    counter_region_.clear();
    n_region_ = 0;
  };
  inline void InitializeRegions(int16_t n_regions) {
    ClearRegions();
    if (n_regions > 0) {
      n_region_ = n_regions;
      weight_region_.reserve(n_regions);
      gal_gal_region_.reserve(n_regions);
      gal_rand_region_.reserve(n_regions);
      rand_gal_region_.reserve(n_regions);
      rand_rand_region_.reserve(n_regions);
      pixel_wtheta_region_.reserve(n_regions);
      pixel_weight_region_.reserve(n_regions);
      wtheta_region_.reserve(n_regions);
      wtheta_error_region_.reserve(n_regions);
      counter_region_.reserve(n_regions);
      for (uint16_t k=0;k<n_regions;k++) {
	weight_region_[k] = 0.0;
	gal_gal_region_[k] = 0.0;
	gal_rand_region_[k] = 0.0;
	rand_gal_region_[k] = 0.0;
	rand_rand_region_[k] = 0.0;
	pixel_wtheta_region_[k] = 0.0;
	pixel_weight_region_[k] = 0.0;
	wtheta_region_[k] = 0.0;
	wtheta_error_region_[k] = 0.0;
	counter_region_[k] = 0;
      }
    }
  };
  inline void SetResolution(uint16_t resolution) {
    resolution_ = resolution;
  };
  inline void SetTheta(double theta) {
    theta_ = theta;
  };
  inline void SetThetaMin(double theta_min) {
    theta_min_ = theta_min;
    sin2theta_min_ =
      sin(theta_min_*Stomp::DegToRad)*sin(theta_min_*Stomp::DegToRad);
    costheta_max_ = cos(theta_min_*Stomp::DegToRad);
  };
  inline void SetThetaMax(double theta_max) {
    theta_max_ = theta_max;
    sin2theta_max_ =
      sin(theta_max_*Stomp::DegToRad)*sin(theta_max_*Stomp::DegToRad);
    costheta_min_ = cos(theta_max_*Stomp::DegToRad);
  };
  inline bool WithinBounds(double theta) {
    return (Stomp::DoubleGE(theta, theta_min_) &&
	    Stomp::DoubleLE(theta, theta_max_) ? true : false);
  };
  inline bool WithinSin2Bounds(double sin2theta) {
    return (Stomp::DoubleGE(sin2theta, sin2theta_min_) &&
	    Stomp::DoubleLE(sin2theta, sin2theta_max_) ? true : false);
  };
  inline bool WithinCosBounds(double costheta) {
    return (Stomp::DoubleGE(costheta, costheta_min_) &&
	    Stomp::DoubleLE(costheta, costheta_max_) ? true : false);
  };
  inline void SetWtheta(double wtheta, int16_t region = -1) {
    set_wtheta_ = true;
    if (region == -1) {
      wtheta_ = wtheta;
    } else {
      wtheta_region_[region] = wtheta;
    }
    set_wtheta_ = true;
  };
  inline void SetWthetaError(double dwtheta, int16_t region = -1) {
    if (region == -1) {
      wtheta_error_ = dwtheta;
    } else {
      wtheta_error_region_[region] = dwtheta;
    }
    set_wtheta_error_ = true;
  };
  inline void AddToPixelWtheta(double dwtheta, double dweight,
			       int16_t region_a = -1, int16_t region_b = -1) {
    if ((region_a == -1) || (region_b == -1)) {
      pixel_wtheta_ += dwtheta;
      pixel_weight_ += dweight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if ((k != region_a) && (k != region_b)) {
	  pixel_wtheta_region_[k] += dwtheta;
	  pixel_weight_region_[k] += dweight;
	}
      }
    }
  };
  inline void AddToWeight(double weight, int16_t region = -1) {
    if (region == -1) {
      weight_ += weight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) weight_region_[k] += weight;
      }
    }
  };
  inline void AddToCounter(uint32_t step=1, int16_t region = -1) {
    if (region == -1) {
      counter_ += step;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) counter_region_[k] += step;
      }
    }
  };
  inline void AddToGalGal(double weight, int16_t region = -1) {
    if (region == -1) {
      gal_gal_ += weight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) gal_gal_region_[k] += weight;
      }
    }
  };
  inline void AddToGalRand(double weight, int16_t region = -1) {
    if (region == -1) {
    gal_rand_ += weight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) gal_rand_region_[k] += weight;
      }
    }
  };
  inline void AddToRandGal(double weight, int16_t region = -1) {
    if (region == -1) {
    rand_gal_ += weight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) rand_gal_region_[k] += weight;
      }
    }
  };
  inline void AddToRandRand(double weight, int16_t region = -1) {
    if (region == -1) {
    rand_rand_ += weight;
    } else {
      for (int16_t k=0;k<n_region_;k++) {
	if (k != region) rand_rand_region_[k] += weight;
      }
    }
  };
  inline void MoveWeightToGalGal() {
    gal_gal_ += weight_;
    weight_ = 0.0;
    for (int16_t k=0;k<n_region_;k++) {
      gal_gal_region_[k] += weight_region_[k];
      weight_region_[k] = 0.0;
    }
  };
  inline void MoveWeightToGalRand(bool move_to_rand_gal = false) {
    gal_rand_ += weight_;
    if (move_to_rand_gal) rand_gal_ += weight_;
    weight_ = 0.0;
    for (int16_t k=0;k<n_region_;k++) {
      gal_rand_region_[k] += weight_region_[k];
      if (move_to_rand_gal) rand_gal_region_[k] += weight_region_[k];
      weight_region_[k] = 0.0;
    }
  };
  inline void MoveWeightToRandGal(bool move_to_gal_rand = false) {
    rand_gal_ += weight_;
    if (move_to_gal_rand) gal_rand_ += weight_;
    weight_ = 0.0;
    for (int16_t k=0;k<n_region_;k++) {
      rand_gal_region_[k] += weight_region_[k];
      if (move_to_gal_rand) gal_rand_region_[k] += weight_region_[k];
      weight_region_[k] = 0.0;
    }
  };
  inline void MoveWeightToRandRand() {
    rand_rand_ += weight_;
    weight_ = 0.0;
    for (int16_t k=0;k<n_region_;k++) {
      rand_rand_region_[k] += weight_region_[k];
      weight_region_[k] = 0.0;
    }
  };
  inline void RescaleGalGal(double weight) {
    gal_gal_ /= weight;
    for (int16_t k=0;k<n_region_;k++) gal_gal_region_[k] /= weight;
  };
  inline void RescaleGalRand(double weight) {
    gal_rand_ /= weight;
    for (int16_t k=0;k<n_region_;k++) gal_rand_region_[k] /= weight;
  };
  inline void RescaleRandGal(double weight) {
    rand_gal_ /= weight;
    for (int16_t k=0;k<n_region_;k++) rand_gal_region_[k] /= weight;
  };
  inline void RescaleRandRand(double weight) {
    rand_rand_ /= weight;
    for (int16_t k=0;k<n_region_;k++) rand_rand_region_[k] /= weight;
  };
  inline void Reset() {
    weight_ = gal_gal_ = gal_rand_ = rand_gal_ = rand_rand_ = 0.0;
    pixel_wtheta_ = pixel_weight_ = wtheta_ = wtheta_error_ = 0.0;
    counter_ = 0;
    if (n_region_ > 0) {
      for (int16_t k=0;k<n_region_;k++) {
	weight_region_[k] = 0.0;
	gal_gal_region_[k] = 0.0;
	gal_rand_region_[k] = 0.0;
	rand_gal_region_[k] = 0.0;
	rand_rand_region_[k] = 0.0;
	pixel_wtheta_region_[k] = 0.0;
	pixel_weight_region_[k] = 0.0;
	wtheta_region_[k] = 0.0;
	wtheta_error_region_[k] = 0.0;
	counter_region_[k] = 0;
      }
    }
  };
  inline void ResetPixelWtheta() {
    pixel_wtheta_ = 0.0;
    pixel_weight_ = 0.0;
    if (n_region_ > 0) {
      for (int16_t k=0;k<n_region_;k++) {
	pixel_wtheta_region_[k] = 0.0;
	pixel_weight_region_[k] = 0.0;
      }
    }
  };
  inline void ResetWeight() {
    weight_ = 0.0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) weight_region_[k] = 0.0;
  };
  inline void ResetCounter() {
    counter_ = 0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) counter_region_[k] = 0;
  };
  inline void ResetGalGal() {
    gal_gal_ = 0.0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) gal_gal_region_[k] = 0.0;
  };
  inline void ResetGalRand() {
    gal_rand_ = 0.0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) gal_rand_region_[k] = 0.0;
  };
  inline void ResetRandGal() {
    rand_gal_ = 0.0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) rand_gal_region_[k] = 0.0;
  };
  inline void ResetRandRand() {
    rand_rand_ = 0.0;
    if (n_region_ > 0)
      for (int16_t k=0;k<n_region_;k++) rand_rand_region_[k] = 0.0;
  };
  inline uint16_t Resolution() {
    return resolution_;
  };
  inline int16_t NRegion() {
    return n_region_;
  };
  inline double Theta() {
    return theta_;
  };
  inline double ThetaMin() {
    return theta_min_;
  };
  inline double ThetaMax() {
    return theta_max_;
  };
  inline double Sin2ThetaMin() {
    return sin2theta_min_;
  };
  inline double Sin2ThetaMax() {
    return sin2theta_max_;
  };
  inline double CosThetaMin() {
    return costheta_min_;
  };
  inline double CosThetaMax() {
    return costheta_max_;
  };
  inline double Wtheta(int16_t region = -1) {
    if (set_wtheta_) {
      return (region == -1 ? wtheta_ :
	      (region < n_region_ ? wtheta_region_[region] : -1.0));
    } else {
      if (resolution_ == 0) {
	return (region == -1 ?
		(gal_gal_ - gal_rand_ - rand_gal_ + rand_rand_)/rand_rand_ :
		(region < n_region_ ?
		 (gal_gal_region_[region] - gal_rand_region_[region] -
		  rand_gal_region_[region] + rand_rand_region_[region])/
		 rand_rand_region_[region] :
		 -1.0));
      } else {
	return (region == -1 ? pixel_wtheta_/pixel_weight_ :
		(region < n_region_ ?
		 pixel_wtheta_region_[region]/pixel_weight_region_[region] :
		 -1.0));
      }
    }
  };
  inline double WthetaError(int16_t region = -1) {
    if (set_wtheta_error_) {
      return (region == -1 ? wtheta_error_ :
	      (region < n_region_ ? wtheta_error_region_[region] : -1.0));
    } else {
      if (resolution_ == 0) {
	return (region == -1 ? 1.0/sqrt(gal_gal_) :
		(region < n_region_ ?
		 1.0/sqrt(gal_gal_region_[region]) : -1.0));
      } else {
	return (region == -1 ? 1.0/sqrt(pixel_weight_) :
		(region < n_region_ ? 1.0/sqrt(pixel_weight_region_[region]) :
		 -1.0));
      }
    }
  };
  inline double WeightedCrossCorrelation(int16_t region = -1) {
    return (region == -1 ? weight_/counter_ :
	    (region < n_region_ ?
	     weight_region_[region]/counter_region_[region] : -1.0));
  };
  inline double Weight(int16_t region = -1) {
    return (region == -1 ? weight_ :
	    (region < n_region_ ? weight_region_[region] : -1.0));
  };
  inline uint32_t Counter(int16_t region = -1) {
    return (region == -1 ? counter_ :
	    (region < n_region_ ? counter_region_[region] : -1));
  };
  inline double GalGal(int16_t region = -1) {
    return (region == -1 ? gal_gal_ :
	    (region < n_region_ ? gal_gal_region_[region] : -1.0));
  };
  inline double GalRand(int16_t region = -1) {
    return (region == -1 ? gal_rand_ :
	    (region < n_region_ ? gal_rand_region_[region] : -1.0));
  };
  inline double RandGal(int16_t region = -1) {
    return (region == -1 ? rand_gal_ :
	    (region < n_region_ ? rand_gal_region_[region] : -1.0));
  };
  inline double RandRand(int16_t region = -1) {
    return (region == -1 ? rand_rand_ :
	    (region < n_region_ ? rand_rand_region_[region] : -1.0));
  };
  inline double MeanWtheta() {
    double mean_wtheta = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_wtheta += Wtheta(k)/(1.0*n_region_);
    return mean_wtheta;
  };
  inline double MeanWthetaError() {
    double mean_wtheta = MeanWtheta();
    double mean_wtheta_error = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_wtheta_error += (mean_wtheta - Wtheta(k))*(mean_wtheta - Wtheta(k));
    return (n_region_ == 0 ? 0.0 :
	    (n_region_ - 1.0)*sqrt(mean_wtheta_error)/n_region_);
  };
  inline double MeanWeightedCrossCorrelation() {
    double mean_weight_cross_correlation = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_weight_cross_correlation +=
	1.0*weight_region_[k]/counter_region_[k]/(1.0*n_region_);
    return mean_weight_cross_correlation;
  };
  inline double MeanWeightedCrossCorrelationError() {
    double mean_weighted_cross_correlation = MeanWeightedCrossCorrelation();
    double mean_weighted_cross_correlation_error = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_weighted_cross_correlation_error +=
	(mean_weighted_cross_correlation - WeightedCrossCorrelation(k))*
	(mean_weighted_cross_correlation - WeightedCrossCorrelation(k));
    return (n_region_ == 0 ? 0.0 :
	    (n_region_ - 1.0)*
	    sqrt(mean_weighted_cross_correlation_error)/n_region_);
  };
  inline double MeanWeight() {
    double mean_weight = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_weight += weight_region_[k]/(1.0*n_region_);
    return mean_weight;
  };
  inline double MeanCounter() {
    double mean_counter = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_counter += 1.0*counter_region_[k]/(1.0*n_region_);
    return mean_counter;
  };
  inline double MeanGalGal() {
    double mean_gal_gal = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_gal_gal += gal_gal_region_[k]/(1.0*n_region_);
    return mean_gal_gal;
  };
  inline double MeanGalRand() {
    double mean_gal_rand = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_gal_rand += gal_rand_region_[k]/(1.0*n_region_);
    return mean_gal_rand;
  };
  inline double MeanRandGal() {
    double mean_rand_gal = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_rand_gal += rand_gal_region_[k]/(1.0*n_region_);
    return mean_rand_gal;
  };
  inline double MeanRandRand() {
    double mean_rand_rand = 0.0;
    for (int16_t k=0;k<n_region_;k++)
      mean_rand_rand += rand_rand_region_[k]/(1.0*n_region_);
    return mean_rand_rand;
  };
  inline static bool ThetaOrder(AngularBin theta_a, AngularBin theta_b) {
    return (theta_a.ThetaMin() < theta_b.ThetaMin() ? true : false);
  }
  inline static bool SinThetaOrder(AngularBin theta_a, AngularBin theta_b) {
    return (theta_a.Sin2ThetaMin() < theta_b.Sin2ThetaMin() ? true : false);
  }
  inline static bool ReverseResolutionOrder(AngularBin theta_a,
					    AngularBin theta_b) {
    return (theta_b.Resolution() < theta_a.Resolution() ? true : false);
  }

 private:
  double theta_min_, theta_max_, theta_;
  double costheta_min_, costheta_max_, sin2theta_min_, sin2theta_max_;
  double weight_, gal_gal_, gal_rand_, rand_gal_, rand_rand_;
  double pixel_wtheta_, pixel_weight_, wtheta_, wtheta_error_;
  uint32_t counter_;
  std::vector<double> weight_region_, gal_gal_region_;
  std::vector<double> gal_rand_region_, rand_gal_region_, rand_rand_region_;
  std::vector<double> pixel_wtheta_region_, pixel_weight_region_;
  std::vector<double> wtheta_region_, wtheta_error_region_;
  std::vector<uint32_t> counter_region_;
  uint16_t resolution_;
  int16_t n_region_;
  bool set_wtheta_error_, set_wtheta_;
};


typedef std::vector<AngularCorrelation> WThetaVector;
typedef WThetaVector::iterator WThetaIterator;

class AngularCorrelation {
  // Class object for calculating auto-correlations and cross-correlations
  // given a set of objects and a Map.  Broadly speaking, this is a
  // container class for a set of AngularBin objects which collectively
  // span some range of angular scales.  Accordingly, the methods are generally
  // intended to package the machinery of the auto-correlation and
  // cross-correlation calculations into simple, one-line calls.

 public:
  // The first constructor takes an angular miniumu and maximum (in degrees)
  // and constructs a logrithmic binning scheme using the specified number
  // of bins per decade (which can be a non-integer value, obviously).  The
  // bins are such that the minimum angular scale of the first bin will be
  // theta_min and the maximum angular scale of the last bin with be
  // theta_max.  The last boolean argument controls whether or not an
  // pixel resolution will be assigned to the bins.  If it is false, then
  // the resolution values will all be -1.
  AngularCorrelation(double theta_min, double theta_max,
		     double bins_per_decade, bool assign_resolutions = true);

  // The alternate constructor is used for a linear binning scheme.  The
  // relationship between theta_min and theta_max remains the same and the
  // spacing of the bins is determined based on the requested number of bins.
  AngularCorrelation(uint32_t n_bins, double theta_min, double theta_max,
		     bool assign_resolutions = true);
  ~AngularCorrelation() {
    thetabin_.clear();
  };

  // Find the resolution we would use to calculate correlation functions for
  // each of the bins.  If this method is not called, then the resolution
  // for each bin is set to -1, which would indicate that any correlation
  // calculation with that bin should be done using a pair-based estimator.
  void AssignBinResolutions(double lammin = -70.0, double lammax = 70.0,
			    uint16_t min_resolution =
			    Stomp::MaxPixelResolution);

  // However, for small angular scales, it's usually faster and more memory
  // efficient to use a pair-based estimator.  To set this scale, we choose
  // a maximum resolution scale we're willing to use our pixel-based estimator
  // on and modify all smaller angular bins to use the pair-based estimator.
  void SetMaxResolution(uint16_t resolution);

  // Additionally, if we are using regions to calculate correlation functions,
  // we need to set the minimum resolution to match the resolution used to
  // divide the total survey area.
  void SetMinResolution(uint16_t resolution);

  // Some wrapper methods for find the auto-correlation and cross-correlations
  void FindAutoCorrelation(Map& stomp_map,
			   WAngularVector& galaxy,
			   uint8_t random_iterations = 1);
  void FindCrossCorrelation(Map& stomp_map,
			    WAngularVector& galaxy_a,
			    WAngularVector& galaxy_b,
			    uint8_t random_iterations = 1);

  // Variation on the wrapper methods that use regions to calculate the
  // cosmic variance on the correlation functions.  If you don't specify the
  // number of regions to use, the code will default to twice the number of
  // angular bins.
  void FindAutoCorrelationWithRegions(Map& stomp_map,
				      WAngularVector& galaxy,
				      uint8_t random_iterations = 1,
				      uint16_t n_regions = 0);
  void FindCrossCorrelationWithRegions(Map& stomp_map,
				       WAngularVector& galaxy_a,
				       WAngularVector& galaxy_b,
				       uint8_t random_iterations = 1,
				       uint16_t n_regions = 0);

  // In general, the code will use a pair-based method for small angular
  // scanes and a pixel-based method for large angular scales.  In the above
  // methods, this happens automatically.  If you want to run these processes
  // separately, these methods allow you to do this.  If the Map used
  // to call these methods has initialized regions, then the estimators will
  // use the region-based methods.
  void FindPixelAutoCorrelation(Map& stomp_map, WAngularVector& galaxy);
  void FindPixelAutoCorrelation(ScalarMap& stomp_map);
  void FindPixelCrossCorrelation(Map& stomp_map, WAngularVector& galaxy_a,
				 WAngularVector& galaxy_b);
  void FindPixelCrossCorrelation(ScalarMap& stomp_map_a,
				 ScalarMap& stomp_map_b);
  void FindPairAutoCorrelation(Map& stomp_map, WAngularVector& galaxy,
			       uint8_t random_iterations = 1);
  void FindPairCrossCorrelation(Map& stomp_map,
				WAngularVector& galaxy_a,
				WAngularVector& galaxy_b,
				uint8_t random_iterations = 1);

  // Now, some accessor methods for finding the angular range of the bins
  // with a given resolution attached to them (the default value returns the
  // results for all angular bins; for pair-based bins, resolution = -1).
  double ThetaMin(uint16_t resolution = 1);
  double ThetaMax(uint16_t resolution = 1);
  double Sin2ThetaMin(uint16_t resolution = 1);
  double Sin2ThetaMax(uint16_t resolution = 1);
  ThetaIterator Begin(uint16_t resolution = 1);
  ThetaIterator End(uint16_t resolution = 1);
  ThetaIterator Find(ThetaIterator begin, ThetaIterator end,
		     double sin2theta);
  ThetaIterator BinIterator(uint8_t bin_idx = 0);
  uint32_t NBins() {
    return thetabin_.size();
  };
  uint16_t MinResolution() {
    return min_resolution_;
  };
  uint16_t MaxResolution() {
    return max_resolution_;
  };

 private:
  ThetaVector thetabin_;
  ThetaIterator theta_pixel_begin_;
  ThetaIterator theta_pair_begin_, theta_pair_end_;
  double theta_min_, theta_max_, sin2theta_min_, sin2theta_max_;
  uint16_t min_resolution_, max_resolution_;
};


typedef std::vector<Pixel> PixelVector;
typedef PixelVector::iterator PixelIterator;
typedef std::pair<PixelIterator, PixelIterator> PixelPair;
typedef std::vector<Pixel *> PixelPtrVector;
typedef PixelPtrVector::iterator PixelPtrIterator;

class Pixel {
  // The core class for this library.  An instance of this class represents
  // a single pixel covering a particular region of the sky, with a particular
  // weight represented by a float.  Pixels can be instantiated with an
  // AngularCoordinate and resolution level or pixel indices or just
  // instantiated with no particular location.

 public:
  Pixel();
  Pixel(const uint16_t resolution, const uint32_t pixnum,
	const double weight = 0.0);
  Pixel(const uint16_t resolution, const uint32_t hpixnum,
	const uint32_t superpixnum, const double weight = 0.0);
  Pixel(AngularCoordinate& ang, const uint16_t resolution,
	const double weight = 0.0);
  Pixel(const uint32_t x, const uint32_t y,
	const uint16_t resolution, const double weight = 0.0);
  virtual ~Pixel();

  // For the purposes of simple ordering (as would be done in various STL
  // containers and the like), we always use the equivalent of
  // LocalOrdering defined below.
  inline bool operator<(Pixel& pix) {
    if (this->Resolution() == pix.Resolution()) {
      if (this->PixelY() == pix.PixelY()) {
	return (this->PixelX() < pix.PixelX() ? true : false);
      } else {
	return (this->PixelY() < pix.PixelY() ? true : false);
      }
    } else {
      return (this->Resolution() < pix.Resolution() ? true : false);
    }
  }
  // Likewise, when testing equality, we ignore the pixel weight and just
  // concentrate on geometry.
  inline bool operator==(Pixel& pix) {
    return (this->Resolution() == pix.Resolution() &&
	    this->PixelY() == pix.PixelY() &&
	    this->PixelX() == pix.PixelX() ? true : false);
  };
  inline bool operator!=(Pixel& pix) {
    return (this->Resolution() != pix.Resolution() ||
	    this->PixelY() != pix.PixelY() ||
	    this->PixelX() != pix.PixelX() ? true : false);
  };

  // For the iterators, we use the same LocalOrdering principle, with the
  // additional caveat that a pixel doesn't go beyond the maximum allowed
  // index+1.
  inline Pixel operator++() {
    if (x_ == Stomp::Nx0*Resolution() - 1) {
      x_ = 0;
      y_++;
    } else {
      if (y_ < Stomp::Ny0*Resolution()) x_++;
    }
    return *this;
  };
  void SetPixnumFromAng(AngularCoordinate& ang);
  inline void SetResolution(uint16_t resolution) {
    resolution_ = Stomp::MostSignificantBit(resolution);
    x_ = 0;
    y_ = 0;
  };
  inline void SetPixnumFromXY(uint32_t x, uint32_t y) {
    x_ = x;
    y_ = y;
  };
  inline uint16_t Resolution() {
    return static_cast<uint16_t>(1 << resolution_);
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
  inline uint32_t PixelX() {
    return x_;
  };
  inline uint32_t PixelY() {
    return y_;
  };

  // These methods all relate the hierarchical nature of the pixelization
  // method.  SetToSuperPix degrades a high resolution pixel into the lower
  // resolution pixel that contains it.  SubPix returns either a vector of
  // higher resolution pixels contained by this pixel or the X-Y pixel index
  // bounds that will let one iterate through the sub-pixels without
  // instantiating them.
  bool SetToSuperPix(uint16_t lo_resolution);
  void SubPix(uint16_t hi_resolution, PixelVector& pix);
  void SubPix(uint16_t hi_resolution, uint32_t& x_min, uint32_t& x_max,
	      uint32_t& y_min, uint32_t& y_max);

  // CohortPix returns the pixels at the same resolution that would combine
  // with the current pixel to form the pixel at the next coarser resolution
  // level.
  void CohortPix(Pixel& pix_a, Pixel& pix_b, Pixel& pix_c);

  // FirstCohort returns a boolean indicating true if the current pixel would
  // be the first pixel in a sorted list of the cohort pixels and false
  // otherwise.
  inline bool FirstCohort() {
    return (2*(x_/2) == x_ && 2*(y_/2) == y_ ? true : false);
  };

  // Since the pixels are equal-area, we only need to know how many times
  // we've sub-divided to get from the HpixResolution to our current resolution.
  inline double Area() {
    return Stomp::HPixArea*Stomp::HPixResolution*Stomp::HPixResolution/
      (Resolution()*Resolution());
  };

  // This returns the index of the pixel that contains the current pixel at
  // a coarser resolution.
  inline uint32_t SuperPix(uint16_t lo_resolution) {
    return (Resolution() < lo_resolution ?
            Stomp::Nx0*Stomp::Ny0*lo_resolution*lo_resolution :
            Stomp::Nx0*lo_resolution*
            static_cast<uint32_t>(y_*lo_resolution/Resolution()) +
            static_cast<uint32_t>(x_*lo_resolution/Resolution()));
  };

  // The fundamental unit of the spherical pixelization.  There are 7488
  // superpixels covering the sky, which makes isolating any searches to just
  // the pixels within a single superpixel a very quick localization strategy.
  inline uint32_t Superpixnum() {
    return Stomp::Nx0*Stomp::HPixResolution*
      static_cast<uint32_t>(y_*Stomp::HPixResolution/Resolution()) +
      static_cast<uint32_t>(x_*Stomp::HPixResolution/Resolution());
  };

  // Single index ordering within a superpixel.  The cast finds the x-y
  // position of the superpixel and then we scale that up to the
  // pseudo resolution within the superpixel (where HPixResolution() is
  // effectively 0 and we scale up from there.  This is a cheat that lets
  // us single index pixels up to a higher resolution without going to a
  // 64 bit index.
  inline uint32_t HPixnum() {
    return
      Resolution()/Stomp::HPixResolution*
      (y_ - Resolution()/Stomp::HPixResolution*
       static_cast<uint32_t>(y_*Stomp::HPixResolution/Resolution())) +
      (x_ - Resolution()/Stomp::HPixResolution*
       static_cast<uint32_t>(x_*Stomp::HPixResolution/Resolution()));
  };

  // Single index ordering for the whole sphere.  Unforunately, the limits of
  // a 32 bit integer only take us up to about 17 arcsecond resolution.
  inline uint32_t Pixnum() {
    return Stomp::Nx0*Resolution()*y_ + x_;
  };

  // Given either the X-Y-resolution, Pixel or AngularCoordinate, return
  // true or false based on whether the implied location is within the current
  // Pixel.
  inline bool Contains(uint16_t pixel_resolution, uint32_t pixel_x,
		       uint32_t pixel_y) {
    return ((pixel_resolution >= Resolution()) &&
	    (pixel_x*Resolution()/pixel_resolution == x_) &&
	    (pixel_y*Resolution()/pixel_resolution == y_) ? true : false);
  };
  inline bool Contains(Pixel& pix) {
    return ((pix.Resolution() >= Resolution()) &&
	    (pix.PixelX()*Resolution()/pix.Resolution() == x_) &&
	    (pix.PixelY()*Resolution()/pix.Resolution() == y_) ? true : false);
  };
  bool Contains(AngularCoordinate& ang);

  // Given a set of lon-lat bounds, return true/false if the pixel is within
  // those bounds.
  bool WithinBounds(double lon_min, double lon_max,
		    double lat_min, double lat_max,
		    AngularCoordinate::Sphere sphere);

  // And slightly more permissive version that checks to see if any part of
  // the pixel is within the input bounds.
  bool IntersectsBounds(double lon_min, double lon_max,
			double lat_min, double lat_max,
			AngularCoordinate::Sphere sphere);

  // Given an angle in degrees (or upper and lower angular bounds in degrees),
  // return a list of pixels at the same resolution within those bounds.
  void WithinRadius(double theta_max, PixelVector& pix,
                    bool check_full_pixel=false);
  void WithinAnnulus(double theta_min, double theta_max,
		     PixelVector& pix, bool check_full_pixel=false);
  void WithinAnnulus(AngularBin& theta, PixelVector& pix,
                     bool check_full_pixel=false);

  // While the methods above are useful, sometimes we want a bit less precision.
  // The return vector of pixels include any possible pixels that might be
  // within the input radius.  We also include options for specifiying an
  // AngularCoordinate other than the one at the center of the pixel.
  void BoundingRadius(double theta_max, PixelVector& pix);
  void BoundingRadius(AngularCoordinate& ang, double theta_max,
		      PixelVector& pix);

  // Similar to the previous methods, but the return values here are the
  // X-Y indices rather than the pixels themselves.  The second instance allows
  // the X index bounds to vary with Y index value to take into account the
  // effects of curvature on the sphere, while the first instance just uses
  // the outer limits.  The third and fourth options allow for centering the
  // X-Y bounds at some angular position other than the pixel center.
  void XYBounds(double theta, uint32_t& x_min, uint32_t& x_max,
		uint32_t& y_min, uint32_t& y_max,
		bool add_buffer = false);
  void XYBounds(double theta, std::vector<uint32_t>& x_min,
		std::vector<uint32_t>& x_max,
		uint32_t& y_min, uint32_t& y_max,
		bool add_buffer = false);
  void XYBounds(AngularCoordinate& ang, double theta,
		uint32_t& x_min, uint32_t& x_max,
		uint32_t& y_min, uint32_t& y_max,
		bool add_buffer = false);
  void XYBounds(AngularCoordinate& ang, double theta,
		std::vector<uint32_t>& x_min,
		std::vector<uint32_t>& x_max,
		uint32_t& y_min, uint32_t& y_max,
		bool add_buffer = false);
  uint8_t EtaStep(double theta);

  // Additionally, it can be useful to know the projected angular distance
  // between an angular location and the nearest edge of the pixel.  This
  // routine returns the maximum projected distance to the closest edge (i.e.,
  // if the projected distance to the nearest edge in eta is small, but large
  // in lambda, the returned value is the lambda value, indicating that the
  // effective angular distance to the area covered by pixel is large).  For
  // cases where you want to know whether any part of the pixel is within some
  // fixed angular distance, this is the relavent quantity.
  //
  // The second version does the same, but for the far edge.  The combination
  // of the two should be sufficient for determining if a pixel is within some
  // angular radius as well as determining if a part of the pixel might be
  // within an angular annulus.
  //
  // In both cases, the return value is (sin(theta))^2 rather than just the
  // angle theta.  For small angles (where this method is most likely used),
  // this is a more useful quantity from a computing speed standpoint.
  double NearEdgeDistance(AngularCoordinate& ang);
  double FarEdgeDistance(AngularCoordinate& ang);

  // Likewise, we also want to be able to find the near and far corner
  // distances if necessary.  This is more expensive because there's a trig
  // function involved in generating the distances.
  double NearCornerDistance(AngularCoordinate& ang);
  double FarCornerDistance(AngularCoordinate& ang);

  // Finishing up the angle-checking methods, we have two more methods that
  // return true or false based on whether the current pixel is within a given
  // angular range of a point on the sphere (specified by either a raw angular
  // coordinate or the center point of another Pixel).  For the slippery
  // case where an annulus might only partially contain the pixel, we have
  // the last two methods.  A return value of 1 indicates that the pixel is
  // fully within the annulus (by calling IsWithinAnnulus with full pixel
  // checking), a return value of 0 indicates that it is fully outside the
  // annulus and a return value of -1 indicates a partial containment.
  bool IsWithinRadius(AngularCoordinate& ang, double theta_max,
                      bool check_full_pixel=false);
  bool IsWithinRadius(Pixel& pix, double theta_max,
                      bool check_full_pixel=false);
  bool IsWithinAnnulus(AngularCoordinate& ang, double theta_min,
                       double theta_max, bool check_full_pixel=false);
  bool IsWithinAnnulus(Pixel& pix, double theta_min, double theta_max,
                       bool check_full_pixel=false);
  bool IsWithinAnnulus(AngularCoordinate& ang, AngularBin& theta,
                       bool check_full_pixel=false);
  bool IsWithinAnnulus(Pixel& pix, AngularBin& theta,
                       bool check_full_pixel=false);
  int8_t IntersectsAnnulus(AngularCoordinate& ang,
			    double theta_min, double theta_max);
  int8_t IntersectsAnnulus(Pixel& pix,
			    double theta_min, double theta_max);
  int8_t IntersectsAnnulus(AngularCoordinate& ang, AngularBin& theta);
  int8_t IntersectsAnnulus(Pixel& pix, AngularBin& theta);

  // A hold-over from the SDSS coordinate system, this converts the current
  // pixel index into an SDSS stripe number.  Although this is generally not
  // useful information in an of itself, stripe number is used as a proxy
  // for constructing roughly square subsections of Maps.
  uint32_t Stripe(uint16_t resolution = Stomp::HPixResolution);

  // Some methods for extracting the angular position of the pixel center...
  double RA();
  double DEC();
  double GalLon();
  double GalLat();
  inline void Ang(AngularCoordinate& ang) {
    ang.SetSurveyCoordinates(90.0 - Stomp::RadToDeg*
                             acos(1.0-2.0*(y_+0.5)/(Stomp::Ny0*Resolution())),
                             Stomp::RadToDeg*(2.0*Stomp::Pi*(x_+0.5))/
                             (Stomp::Nx0*Resolution()) + Stomp::EtaOffSet);
  };
  inline double Lambda() {
    return 90.0 -
      Stomp::RadToDeg*acos(1.0 - 2.0*(y_+0.5)/(Stomp::Ny0*Resolution()));
  }
  inline double Eta() {
    double eta =
      Stomp::RadToDeg*2.0*Stomp::Pi*(x_+0.5)/(Stomp::Nx0*Resolution()) +
      Stomp::EtaOffSet;
    return (eta >= 180.0 ? eta - 360.0 : eta);
  };

  // ... likewise for the Cartesian coordinates on the unit sphere.
  inline double UnitSphereX() {
    return -1.0*sin(Lambda()*Stomp::DegToRad);
  };
  inline double UnitSphereY() {
    return cos(Lambda()*Stomp::DegToRad)*
      cos(Eta()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereZ() {
    return cos(Lambda()*Stomp::DegToRad)*
      sin(Eta()*Stomp::DegToRad+Stomp::EtaPole);
  };

  // Since the pixels are rectangular in survey coordinates, we have meaningful
  // notions of the bounds in lambda-eta space.
  inline double LambdaMin() {
    return 90.0 -
      Stomp::RadToDeg*acos(1.0 - 2.0*(y_+1)/(Stomp::Ny0*Resolution()));
  };
  inline double LambdaMax() {
    return 90.0 -
      Stomp::RadToDeg*acos(1.0 - 2.0*y_/(Stomp::Ny0*Resolution()));
  };
  inline double EtaMin() {
    double etamin =
      Stomp::RadToDeg*2.0*Stomp::Pi*x_/(Stomp::Nx0*Resolution()) +
      Stomp::EtaOffSet;
    return (etamin >= 180.0 ? etamin - 360.0 : etamin);
  };
  inline double EtaMax() {
    double etamax =
      Stomp::RadToDeg*2.0*Stomp::Pi*(x_+1)/(Stomp::Nx0*Resolution()) +
      Stomp::EtaOffSet;
    return (etamax >= 180.0 ? etamax - 360.0 : etamax);
  };

  // In some cases, it can be handy to have a variant on the previous method
  // where we return a value of EtaMax that might technically violate the
  // proper bounds on Eta (-180 < Eta < 180), but is continous with the
  // value of EtaMin (i.e. EtaMin < EtaMax, regardless of the discontinuity).
  inline double EtaMaxContinuous() {
    double etamax = EtaMax();
    double etamin = EtaMin();
    return (etamax > etamin ? etamax : etamax + 360.0);
  };
  // Alternatively, we may just want to know if the pixel crosses the Eta
  // discontinuity.
  inline bool SurveyContinuous() {
    return (EtaMaxContinuous() > 180.0 ? false : true);
  }

  // Corresponding bounds for Equatorial and Galactic coordinates.  These are
  // more expensive since we need to check the bounds at each corner.
  double DECMin();
  double DECMax();
  double RAMin();
  double RAMax();
  double RAMaxContinuous();
  inline bool EquatorialContinuous() {
    return (RAMaxContinuous() > 360.0 ? false : true);
  };
  double GalLatMin();
  double GalLatMax();
  double GalLonMin();
  double GalLonMax();
  double GalLonMaxContinuous();
  inline bool GalacticContinuous() {
    return (GalLonMaxContinuous() > 360.0 ? false : true);
  };

  // And one last function to check continuity for a given angular coordinate
  // system.
  inline bool ContinuousBounds(AngularCoordinate::Sphere sphere) {
    return (sphere == AngularCoordinate::Survey ? SurveyContinuous() :
	    (sphere == AngularCoordinate::Equatorial ? EquatorialContinuous() :
	     GalacticContinuous()));
  };

  // And it can be useful to be able to quickly extract the x-y-z positions of
  // the pixel corners.
  inline double UnitSphereX_UL() {
    return -1.0*sin(LambdaMax()*Stomp::DegToRad);
  };
  inline double UnitSphereY_UL() {
    return cos(LambdaMax()*Stomp::DegToRad)*
      cos(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereZ_UL() {
    return cos(LambdaMax()*Stomp::DegToRad)*
      sin(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereX_UR() {
    return -1.0*sin(LambdaMax()*Stomp::DegToRad);
  };
  inline double UnitSphereY_UR() {
    return cos(LambdaMax()*Stomp::DegToRad)*
      cos(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereZ_UR() {
    return cos(LambdaMax()*Stomp::DegToRad)*
      sin(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereX_LL() {
    return -1.0*sin(LambdaMin()*Stomp::DegToRad);
  };
  inline double UnitSphereY_LL() {
    return cos(LambdaMin()*Stomp::DegToRad)*
      cos(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereZ_LL() {
    return cos(LambdaMin()*Stomp::DegToRad)*
      sin(EtaMin()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereX_LR() {
    return -1.0*sin(LambdaMin()*Stomp::DegToRad);
  };
  inline double UnitSphereY_LR() {
    return cos(LambdaMin()*Stomp::DegToRad)*
      cos(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  };
  inline double UnitSphereZ_LR() {
    return cos(LambdaMin()*Stomp::DegToRad)*
      sin(EtaMax()*Stomp::DegToRad+Stomp::EtaPole);
  };

  inline void Iterate(bool wrap_pixel = true) {
    if (x_ == Stomp::Nx0*Resolution() - 1) {
      x_ = 0;
      if (!wrap_pixel) y_++;
    } else {
      x_++;
    }
  };

  // Like PixelX, but this returns the first x value for the current pixel's
  // superpixel (useful for knowing the bounds for SuperPixelBasedOrder'd lists.
  inline uint32_t PixelX0() {
    return static_cast<uint32_t>(x_*Stomp::HPixResolution/Resolution())*
      Resolution()/Stomp::HPixResolution;
  };

  // Same as PixelX0, but for the y index.
  inline uint32_t PixelY0() {
    return static_cast<uint32_t>(y_*Stomp::HPixResolution/Resolution())*
      Resolution()/Stomp::HPixResolution;
  };

  // This would be the x value just beyond the limit for the current pixel's
  // superpixel.  Hence, all the pixels with PixelX0 <= x < PixelX1 are in
  // the same column of superpixels.  For pixels in
  // superpixel = MaxSuperpixum, this is Nx0*_resolution, so it can be used
  // as an iteration bound for all superpixels.
  inline uint32_t PixelX1() {
    return PixelX0() + Resolution()/Stomp::HPixResolution;
  };

  // Same as PixelX1, but for the y index.
  inline uint32_t PixelY1() {
    return PixelY0() + Resolution()/Stomp::HPixResolution;
  };

  // Given a requested number of points, return a vector of Poisson random
  // angular positions within the current Pixel's area.
  void GenerateRandomPoints(AngularVector& ang, uint32_t n_point = 1);

  // This next block of code is there to provide backwards compatibility
  // to a straight C interface.  True, we're using references which aren't
  // in C, but these methods still allow users to access the basic interfaces
  // without instantiating a Pixel, which can be handy in some cases.
  static void Ang2Pix(const uint16_t resolution, AngularCoordinate& ang,
		      uint32_t& pixnum);
  static void Pix2Ang(uint16_t resolution, uint32_t pixnum,
                      AngularCoordinate& ang);
  static void Pix2HPix(uint16_t input_resolution, uint32_t input_pixnum,
		       uint32_t& output_hpixnum,
		       uint32_t& output_superpixnum);
  static void HPix2Pix(uint16_t input_resolution, uint32_t input_hpixnum,
		       uint32_t input_superpixnum,
		       uint32_t& output_pixnum);
  static void SuperPix(uint16_t hi_resolution, uint32_t hi_pixnum,
                       uint16_t lo_resolution, uint32_t& lo_pixnum);
  static void SubPix(uint16_t lo_resolution, uint32_t hi_pixnum,
		     uint16_t hi_resolution, uint32_t& x_min,
		     uint32_t& x_max, uint32_t& y_min,
		     uint32_t& y_max);
  static void NextSubPix(uint16_t input_resolution, uint32_t input_pixnum,
			 uint32_t& sub_pixnum1,
			 uint32_t& sub_pixnum2,
			 uint32_t& sub_pixnum3,
			 uint32_t& sub_pixnum4);
  static void AreaIndex(uint16_t resolution, double lammin, double lammax,
			double etamin, double etamax, uint32_t& x_min,
			uint32_t& x_max, uint32_t& y_min,
			uint32_t& y_max);
  static void PixelBound(uint16_t resolution, uint32_t pixnum, double& lammin,
			 double& lammax, double& etamin, double& etamax);
  static void CohortPix(uint16_t resolution, uint32_t hpixnum,
			uint32_t& pixnum1, uint32_t& pixnum2,
			uint32_t& pixnum3);
  static double PixelArea(uint16_t resolution) {
    return Stomp::HPixArea*Stomp::HPixResolution*Stomp::HPixResolution/
      (resolution*resolution);
  };
  static uint8_t Pix2EtaStep(uint16_t resolution, uint32_t pixnum,
			     double theta);
  static void Ang2HPix(uint16_t resolution, AngularCoordinate& ang,
		       uint32_t& hpixnum, uint32_t& superpixnum);
  static void HPix2Ang(uint16_t resolution, uint32_t hpixnum,
                       uint32_t superpixnum, AngularCoordinate& ang);
  static void XY2HPix(uint16_t resolution, uint32_t x, uint32_t y,
                      uint32_t& hpixnum, uint32_t& superpixnum);
  static void HPix2XY(uint16_t resolution, uint32_t hpixnum,
		      uint32_t superpixnum, uint32_t& x,
		      uint32_t& y);
  static void SuperHPix(uint16_t hi_resolution, uint32_t hi_hpixnum,
                        uint16_t lo_resolution, uint32_t& lo_hpixnum);
  static void NextSubHPix(uint16_t resolution, uint32_t hpixnum,
			  uint32_t& hpixnum1, uint32_t& hpixnum2,
			  uint32_t& hpixnum3, uint32_t& hpixnum4);
  static void SubHPix(uint16_t lo_resolution, uint32_t hi_hpixnum,
		      uint32_t hi_superpixnum, uint16_t hi_resolution,
		      uint32_t& x_min, uint32_t& x_max,
		      uint32_t& y_min, uint32_t& y_max);
  static void HPixelBound(uint16_t resolution, uint32_t hpixnum,
			  uint32_t superpixnum, double& lammin,
			  double& lammax, double& etamin, double& etamax);
  static void CohortHPix(uint16_t resolution, uint32_t hpixnum,
			 uint32_t& hpixnum1, uint32_t& hpixnum2,
			 uint32_t& hpixnum3);
  static double HPixelArea(uint16_t resolution) {
    return Stomp::HPixArea*Stomp::HPixResolution*Stomp::HPixResolution/
      (resolution*resolution);
  };
  static uint8_t HPix2EtaStep(uint16_t resolution, uint32_t hpixnum,
			      uint32_t superpixnum, double theta);
  static void XY2Pix(uint16_t resolution, uint32_t x, uint32_t y,
		     uint32_t& pixnum) {
    pixnum = Stomp::Nx0*resolution*y + x;
  };
  static void Pix2XY(uint16_t resolution, uint32_t pixnum,
		     uint32_t& x, uint32_t& y) {
    y = pixnum/(Stomp::Nx0*resolution);
    x = pixnum - Stomp::Nx0*resolution*y;
  };

  // Now we've got the various methods to establish ordering on the pixels.
  // LocalOrder is the the simplest, just arranging all of the pixels in
  // vanilla row-column order.  That's useful for some operations where you
  // want to be able to access nearby pixels simply.  However, if you're
  // doing a search on a large region of the sky, it often helps to be able to
  // limit the search more drastically at the outset.  For that, we have
  // SuperPixelBasedOrder where pixels are grouped by their lowest resolution
  // superpixel and then locally sorted within that bound.  This is the
  // default sorting method for the Map class to make searching on those
  // maps more efficient.  Finally, we have some methods for checking
  // whether or not we're looking at equivalent pixels, one where the weights
  // associated with the pixels matter and one that's purely geometric.
  static bool LocalOrder(const Pixel pix_a, const Pixel pix_b);
  static bool LocalOrderByReference(const Pixel pix_a, const Pixel pix_b);
  static bool SuperPixelBasedOrder(const Pixel pix_a, const Pixel pix_b);
  static bool SuperPixelOrder(const Pixel pix_a, const Pixel pix_b);
  inline static bool WeightedOrder(Pixel pix_a, Pixel pix_b) {
    return (pix_a.Weight() < pix_b.Weight() ? true : false);
  };
  inline static bool WeightMatch(Pixel& pix_a, Pixel& pix_b) {
    return ((pix_b.Weight() < pix_a.Weight() + 0.000001) &&
            (pix_b.Weight() > pix_a.Weight() - 0.000001) ? true : false);
  };
  inline static bool WeightedPixelMatch(Pixel& pix_a, Pixel& pix_b) {
    return ((pix_a.Resolution() == pix_b.Resolution()) &&
            (pix_a.PixelX() == pix_b.PixelX()) &&
            (pix_a.PixelY() == pix_b.PixelY()) &&
            (pix_b.Weight() < pix_a.Weight() + 0.000001) &&
            (pix_b.Weight() > pix_a.Weight() - 0.000001) ? true : false);
  };
  inline static bool PixelMatch(Pixel& pix_a, Pixel& pix_b) {
    return ((pix_a.Resolution() == pix_b.Resolution()) &&
            (pix_a.PixelX() == pix_b.PixelX()) &&
            (pix_a.PixelY() == pix_b.PixelY()) ? true : false);
  };

  // Finally, these methods handle maps consisting of vectors of Pixels.
  // One could make the argument that this should be in the Map class,
  // but one of the primary purposes of these methods is to take a list of
  // pixels where there may be duplication or cases where smaller pixels are
  // within larger pixels and generate a set of pixels that uniquely covers
  // a give region of the sky.  That extra applicability makes it appropriate
  // to put here.  The main method to call is ResolvePixel, which will call
  // ResolveSuperPixel individually for each fo the superpixels covered by
  // the vector of Pixels.  The resulting vector will be sorted by
  // SuperPixelBasedOrder.
  static void ResolveSuperPixel(PixelVector& pix, bool ignore_weight = false);
  static void ResolvePixel(PixelVector& pix, bool ignore_weight = false);
  static void FindUniquePixels(PixelVector& input_pix, PixelVector& unique_pix);

 private:
  double weight_;
  uint32_t x_, y_;
  uint8_t resolution_;
};


typedef std::vector<ScalarPixel> ScalarVector;
typedef ScalarVector::iterator ScalarIterator;
typedef std::pair<ScalarIterator, ScalarIterator> ScalarPair;
typedef std::vector<ScalarPixel *> ScalarPtrVector;
typedef ScalarPtrVector::iterator ScalarPtrIterator;

class ScalarPixel : public Pixel {
  // In order to do correlation function calculations, we need some
  // functionality beyond the normal Pixel object.  In particular, we want
  // to be able to encode fields, which may take one of three forms:
  //
  // * Pure scalar quantities (e.g. CMB temperature or radio flux).
  // * Point-field densities (e.g. the projected galaxy density over some area).
  // * Point-sampled averages (e.g. the mean galaxy magnitude over some area).
  //
  // In order to accomodate those three cases, we need an extra float and
  // an extra int (the Weight() value in Pixel will encode the fraction of
  // the pixel area that's contained in the survey area).  Pixels are taken
  // to be units of a Map where the geometry of the map is the union of all of
  // the Pixels and the Pixels are not assumed to be at the same resolution
  // level.  ScalarPixels, OTOH, form the basis for ScalarMaps where the
  // map is taken to be a regular sampling of some field over a given area.
  // The total area for the map can be calculated, but operations like
  // determining whether or not a given position is inside or outside the
  // map is not generically available.
 public:
  ScalarPixel();
  ScalarPixel(const uint16_t resolution, const uint32_t pixnum,
	      const double weight = 0.0, const double intensity = 0.0,
	      const uint32_t n_points = 0);
  ScalarPixel(const uint16_t resolution, const uint32_t hpixnum,
	      const uint32_t superpixnum, const double weight = 0.0,
	      const double intensity = 0.0, const uint32_t n_points = 0);
  ScalarPixel(AngularCoordinate& ang, const uint16_t resolution,
	      const double weight = 0.0, const double intensity = 0.0,
	      const uint32_t n_points = 0);
  ScalarPixel(const uint32_t x, const uint32_t y,
	      const uint16_t resolution, const double weight = 0.0,
	      const double intensity = 0.0, const uint32_t n_points = 0);
  virtual ~ScalarPixel();

  // To encode the three usage cases, we use an extra float (intensity) and
  // an int (n_points).  In the pure scalar field case, we ignore the
  // n_points variable and merely modify the intensity.  If we are treating
  // the pixel as a container for something like galaxy density, then we also
  // only care about the intensity, but when it comes to using this for doing
  // correlation calculations, we'll want to use a different form for the
  // over-density than for the scalar field case.  Finally, the point-sampled
  // field case requires us to keep track of the number of points we've added
  // to the pixel so that we can calculate the field average later on.
  inline void SetIntensity(const double intensity) {
    intensity_ = intensity;
  };
  inline void SetNPoints(const uint32_t n_point) {
    n_point_ = n_point;
  };
  inline double Intensity() {
    return intensity_;
  };
  inline uint32_t NPoints() {
    return n_point_;
  };

  // If we've gotta scalar field encoded on the pixel, the number of points
  // will be zero, so we just return the intensity value.  Otherwise, return
  // the average intensity for all of the points added to the pixel.
  inline double MeanIntensity() {
    return (n_point_ == 0 ? intensity_ : intensity_/n_point_);
  };
  inline void AddToIntensity(const double intensity,
			     const uint32_t n_point = 1) {
    intensity_ += intensity;
    n_point_ += n_point;
  };
  inline void ScaleIntensity(double scale_factor) {
    intensity_ *= scale_factor;
  };

  // Once we're done adding points to the pixel, we may want to normalize to a
  // scalar field value so that we can access this field through the
  // Intensity() method.
  inline void NormalizeIntensity() {
    intensity_ /= n_point_;
    n_point_ = 0;
  };

  // For the first and third usage cases, this is the form of the over-density
  // we'll want to use.
  inline void ConvertToOverDensity(double expected_intensity) {
    intensity_ -= expected_intensity;
    is_overdensity_ = true;
  };

  // For the second use case (something like a galaxy density field), we
  // want to normalize our over-density by the average field (density) value.
  inline void ConvertToFractionalOverDensity(double expected_intensity) {
    intensity_ =
      (intensity_ - expected_intensity*Weight()*Area())/
      (expected_intensity*Weight()*Area());
    is_overdensity_ = true;
  };

  // And two complementary methods to take us from over-densities back to raw
  // intensities.
  inline void ConvertFromOverDensity(double expected_intensity) {
    intensity_ += expected_intensity;
    is_overdensity_ = false;
  };
  inline void ConvertFromFractionalOverDensity(double expected_intensity) {
    double norm_intensity = expected_intensity*Weight()*Area();
    intensity_ = intensity_*norm_intensity + norm_intensity;
    is_overdensity_ = false;
  };

  inline bool IsOverDensity() {
    return is_overdensity_;
  };

 private:
  double intensity_;
  uint32_t n_point_;
  bool is_overdensity_;
};

typedef std::map<const uint16_t, uint32_t> ResolutionDict;
typedef ResolutionDict::iterator ResolutionIterator;
typedef std::pair<ResolutionIterator, ResolutionIterator> ResolutionPair;

typedef std::vector<TreePixel> TreeVector;
typedef TreeVector::iterator TreeIterator;
typedef std::pair<TreeIterator, TreeIterator> TreePair;
typedef std::vector<TreePixel *> TreePtrVector;
typedef TreePtrVector::iterator TreePtrIterator;

typedef std::map<const uint32_t, TreePixel *> TreeDict;
typedef TreeDict::iterator TreeDictIterator;
typedef std::pair<TreeDictIterator, TreeDictIterator> TreeDictPair;

typedef std::pair<double, TreePixel*> DistancePixelPair;
typedef std::priority_queue<DistancePixelPair,
  std::vector<DistancePixelPair>, NearestNeighborPixel> PixelQueue;

class NearestNeighborPixel {
 public:
  int operator()(const DistancePixelPair& x, const DistancePixelPair& y) {
    // This has the opposite ordering since we want pixels ordered with the
    // closest at the top of the heap.
    return x.first > y.first;
  }
};

class TreePixel : public Pixel {
  // Our second variation on the Pixel.  Like ScalarPixel, the idea
  // here is to use the Pixel as a scaffold for sampling a field over an
  // area.  Instead of storing a density, however, TreePixel stores a
  // vector of WeightedAngularCoordinates and the weight stored in the pixel
  // is the sum of the weights of the WeightedAngularCoordinates.  Finally,
  // the TreePixel contains pointers to its sub-pixels.  When a point is
  // added to the pixel, it checks the number of points against the total
  // allowed for the pixel (specified on construction).  If the pixel is at
  // capacity, it passes the point along to the sub-pixels, generating a tree
  // structure which can be traversed later on for operations like
  // pair-counting.
 public:
  friend class NearestNeighborPixel;
  TreePixel();
  TreePixel(const uint16_t resolution, const uint32_t pixnum,
	    const uint16_t maximum_points=200);
  TreePixel(const uint16_t resolution, const uint32_t hpixnum,
	    const uint32_t superpixnum, const uint16_t maximum_points=200);
  TreePixel(AngularCoordinate& ang, const uint16_t resolution,
	    const uint16_t maximum_points=200);
  TreePixel(const uint32_t x, const uint32_t y, const uint16_t resolution,
	    const uint16_t maximum_points=200);
  virtual ~TreePixel();


  // When the pixel has reached its carrying capacity, we want to split its
  // contents to the sub-pixels.  In this case, we create the map object that
  // contains the sub-pixels and move each of the points contained in the
  // current pixel to the correct sub-pixel.  The total weight and point count
  // for this pixel remains the same.
  bool _InitializeSubPixels();


  // The primary purpose of this class is to enable fast pair-finding for a
  // set of angular locations.  These methods implement that functionality
  // with a couple different modes of operation.  FindPairs returns an integer
  // counting of the AngularCoordinates within the specified radius or
  // annulus.  FindWeightedPairs does the same, but the value returned is
  // the sum of the weights for the objects satisfying the angular bounds.  Note
  // that the argument in this case is still an AngularCoordinate, so any
  // weight associated with that point is ignored.  The AngularCorrelation
  // versions put the number of pairs in the Counter and Weight values for each
  // angular bin.
  uint32_t DirectPairCount(AngularCoordinate& ang, AngularBin& theta,
			   int16_t region = -1);
  uint32_t FindPairs(AngularCoordinate& ang, AngularBin& theta,
		     int16_t region = -1);
  uint32_t FindPairs(AngularCoordinate& ang,
		     double theta_min, double theta_max);
  uint32_t FindPairs(AngularCoordinate& ang, double theta_max);
  double DirectWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
			     int16_t region = -1);
  double FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
			   int16_t region = -1);
  double FindWeightedPairs(AngularCoordinate& ang,
			   double theta_min, double theta_max);
  double FindWeightedPairs(AngularCoordinate& ang, double theta_max);


  // And for the case where we want to scale things by a weight associated with
  // each angular point explicitly.
  double DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
			     AngularBin& theta, int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   AngularBin& theta, int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_min, double theta_max);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_max);

  // In these cases, the sum of the pairs are put into the Counter field for
  // the corresponding angular bin and the sum of the products of the weights
  // are put into the Weight field, if applicable.
  void FindPairs(AngularVector& ang, AngularBin& theta,
		 int16_t region = -1);
  void FindPairs(AngularVector& ang, AngularCorrelation& wtheta,
		 int16_t region = -1);
  void FindWeightedPairs(AngularVector& ang, AngularBin& theta,
			 int16_t region = -1);
  void FindWeightedPairs(AngularVector& ang, AngularCorrelation& wtheta,
			 int16_t region = -1);
  void FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta,
			 int16_t region = -1);
  void FindWeightedPairs(WAngularVector& w_ang,
			 AngularCorrelation& wtheta, int16_t region = -1);

  // Since the WeightedAngularCoordinates that are fed into our tree also
  // have an arbitrary number of named Fields associated with them, we need
  // to be able to access those values as well in our pair counting.
  double DirectWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
			     const std::string& field_name,
			     int16_t region = -1);
  double FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
			   const std::string& field_name, int16_t region = -1);
  double FindWeightedPairs(AngularCoordinate& ang,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(AngularCoordinate& ang, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(AngularVector& ang, AngularBin& theta,
			 const std::string& field_name, int16_t region = -1);
  void FindWeightedPairs(AngularVector& ang, AngularCorrelation& wtheta,
			 const std::string& field_name, int16_t region = -1);

  // If we have a WeightedAngularCoordinate as the input, then we need to
  // account for the case where you want to use the weight associated with
  // the coordinate as well as the case where you want to use a field from
  // the input coordinate.  First the Weight vs. Field case.
  double DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
			     AngularBin& theta, const std::string& field_name,
			     int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang, AngularBin& theta,
			   const std::string& field_name, int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta,
			 const std::string& field_name, int16_t region = -1);
  void FindWeightedPairs(WAngularVector& w_ang, AngularCorrelation& wtheta,
			 const std::string& field_name, int16_t region = -1);

  // And finally, the Field vs. Field case.
  double DirectWeightedPairs(WeightedAngularCoordinate& w_ang,
			     const std::string& ang_field_name,
			     AngularBin& theta, const std::string& field_name,
			     int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name, AngularBin& theta,
			   const std::string& field_name, int16_t region = -1);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang,
			 const std::string& ang_field_name,
			 AngularBin& theta, const std::string& field_name,
			 int16_t region = -1);
  void FindWeightedPairs(WAngularVector& w_ang,
			 const std::string& ang_field_name,
			 AngularCorrelation& wtheta,
			 const std::string& field_name, int16_t region = -1);

  // In addition to pair finding, we can also use the tree structure we've
  // built to do efficient nearest neighbor searches.  In the general case,
  // we'll be finding the k nearest neighbors of an input point.  The return
  // value is the number of nodes touched during the assemblage.
  //
  // NOTE: There is no duplication checking.  Hence, if the input point is a
  // copy of a point in the tree, then that point will be included in the
  // returned vector of points.
  uint16_t FindKNearestNeighbors(AngularCoordinate& ang, uint8_t n_neighbors,
				 WAngularVector& neighbors_ang);

  // The special case where we're only interested in the nearest matching point.
  uint16_t FindNearestNeighbor(AngularCoordinate& ang,
			       WeightedAngularCoordinate& neighbor_ang);

  // In some cases, we're only interested in the distance to the kth nearest
  // neighbor.  The return value will be the angular distance in degrees.
  double KNearestNeighborDistance(AngularCoordinate& ang, uint8_t n_neighbors,
				  uint16_t& nodes_visited);

  // Or in the distance to the nearest neighbor.
  inline double NearestNeighborDistance(AngularCoordinate& ang,
					uint16_t& nodes_visited) {
    return KNearestNeighborDistance(ang, 1, nodes_visited);
  };

  // For the recursion necessary to do the neighbor finding, we use this
  // internal method.
  void _NeighborRecursion(AngularCoordinate& ang, TreeNeighbor& neighbor);


  // For the pair finding, we end up checking the X-Y-Z corners of the pixels
  // a lot, so we store those values internally and use an internal method for
  // finding the intersection of the pixel with the input annulus;
  int8_t _IntersectsAnnulus(AngularCoordinate& ang, AngularBin& theta);

  // We also have an internal version of the code for finding the edge
  // distances.
  bool _EdgeDistances(AngularCoordinate& ang, double& min_edge_distance,
		      double& max_edge_distance);

  // And a method to set these values up internally.
  void InitializeCorners();

  // Add a given point on the sphere to either this pixel (if the capacity for
  // this pixel hasn't been reached) or one of the sub-pixels.  Return true
  // if the point was successfully added (i.e. the point was contained in the
  // bounds of the current pixel); false, otherwise.
  bool AddPoint(WeightedAngularCoordinate* ang);

  // The default method for adding WeightedAngularCoordinates to the pixel
  // takes a pointer to the object.  This means that the pixel now owns that
  // object and it shouldn't be deleted from the heap except by the pixel.
  // For cases where we want to retain a copy of the point outside of the
  // pixel, we provide a second method which takes a reference to the object
  // and creates and stores an internal copy.  The input object can thus be
  // modified or deleted without affecting the tree.
  inline bool AddPoint(WeightedAngularCoordinate& w_ang) {
    WeightedAngularCoordinate* ang_copy =
      new WeightedAngularCoordinate(w_ang.UnitSphereX(), w_ang.UnitSphereY(),
				    w_ang.UnitSphereZ(), w_ang.Weight());
    ang_copy->CopyFields(w_ang);
    return AddPoint(ang_copy);
  };

  // Complimentary method for specifying a weight separately when adding a
  // point to the pixel.
  inline bool AddPoint(AngularCoordinate& ang, double object_weight = 1.0) {
    WeightedAngularCoordinate* w_ang =
      new WeightedAngularCoordinate(ang.UnitSphereX(), ang.UnitSphereY(),
				    ang.UnitSphereZ(), object_weight);
    return AddPoint(w_ang);
  };

  // Return the number of points contained in the current pixel and all
  // sub-pixels.
  inline uint32_t NPoints() {
    return point_count_;
  };

  // A variation on the above method, returns the number of points associated
  // with the current pixel that are also contained in the input pixel.
  uint32_t NPoints(Pixel& pix);

  // Likewise, we can provide a similar method for returning the weight
  // associated with an input pixel.
  double PixelWeight(Pixel& pix);

  // The downside of the TreePixel is that it doesn't really encode geometry
  // in the same way that Pixels and ScalarPixels do.  This makes it hard to
  // do things like split TreeMaps (defined below) into roughly equal areas
  // like we can do with Maps and ScalarMaps.  Coverage attempts to do this
  // based on the number of sub-nodes with data in them.  The first version
  // works on the pixel itself.  The second does the same calculation for
  // another pixel, based on the data in the current pixel.  Like the unmasked
  // fraction measures for Pixels and ScalarPixels, the return values cover
  // the range [0,1].  However, the accuracy of the measure is going to be a
  // function of how many points are in the pixel (and sub-pixels) and how
  // localized they are.
  double Coverage();
  double Coverage(Pixel& pix);


  // If we want to extract a copy of all of the points that have been added
  // to this pixel, this method allows for that.
  void Points(WAngularVector& w_ang);

  // And an associated method that will extract a copy of the points associated
  // with an input pixel.
  void Points(WAngularVector& w_ang, Pixel& pix);

  // Recurse through the nodes below this one to return the number of nodes in
  // the tree.
  uint16_t Nodes();
  void _AddSubNodes(uint16_t& n_nodes);

  // Modify the weight of the pixel.  Generally this is only called when adding
  // a point to the pixel.  Calling it directly will result in a pixel weight
  // which is no longer the sum of the contained points' weights.
  inline void AddToWeight(double weight) {
    SetWeight(Weight() + weight);
  };

  // Since our WeightedAngularCoordinate objects have an arbitrary number
  // of Fields associated with them, we store that information as well when
  // we're building our tree structure.  These methods allow for access to
  // the aggregate values for a given Field.
  inline double FieldTotal(const std::string& field_name) {
    return (field_total_.find(field_name) != field_total_.end() ?
	    field_total_[field_name] : 0.0);
  };
  double FieldTotal(const std::string& field_name, Pixel& pix);
  inline void AddToField(const std::string& field_name, double weight) {
    if (field_total_.find(field_name) != field_total_.end()) {
      field_total_[field_name] += weight;
    } else {
      field_total_[field_name] = weight;
    }
  };
  inline uint16_t NField() {
    return field_total_.size();
  };
  inline bool HasFields() {
    return (field_total_.size() > 0 ? true : false);
  };
  inline void FieldNames(std::vector<std::string>& field_names) {
    field_names.clear();
    for (FieldIterator iter=field_total_.begin();
	 iter!=field_total_.end();++iter) field_names.push_back(iter->first);
  };

  // Modify and return the point capacity for the pixel, respectively.
  inline void SetPixelCapacity(uint16_t maximum_points) {
    maximum_points_ = maximum_points;
  };
  inline uint16_t PixelCapacity() {
    return maximum_points_;
  };

  // Since we're storing pointers to the WeightedAngularCoordinates, we need
  // to explicitly delete them to clear all of the memory associated with the
  // pixel.
  inline void Clear() {
    if (!ang_.empty())
      for (WAngularPtrIterator iter=ang_.begin();iter!=ang_.end();++iter)
	delete *iter;
    ang_.clear();
    if (!subpix_.empty())
      for (uint32_t i=0;i<subpix_.size();i++) {
	subpix_[i]->Clear();
	delete subpix_[i];
      }
    subpix_.clear();
  };

 private:
  WAngularPtrVector ang_;
  FieldDict field_total_;
  uint16_t maximum_points_;
  uint32_t point_count_;
  bool initialized_subpixels_;
  double unit_sphere_x_, unit_sphere_y_, unit_sphere_z_;
  double unit_sphere_x_ul_, unit_sphere_y_ul_, unit_sphere_z_ul_;
  double unit_sphere_x_ll_, unit_sphere_y_ll_, unit_sphere_z_ll_;
  double unit_sphere_x_ur_, unit_sphere_y_ur_, unit_sphere_z_ur_;
  double unit_sphere_x_lr_, unit_sphere_y_lr_, unit_sphere_z_lr_;
  TreePtrVector subpix_;
};

typedef std::pair<double, WeightedAngularCoordinate*> DistancePointPair;
typedef std::priority_queue<DistancePointPair,
  std::vector<DistancePointPair>, NearestNeighborPoint> PointQueue;

class NearestNeighborPoint {
 public:
  int operator()(const DistancePointPair& x, const DistancePointPair& y) {
    return x.first < y.first;
  }
};

class TreeNeighbor {
  // In order to do the nearest neighbor finding in the TreePixel class, we
  // need a secondary class to handle storage of the nearest neighbor list.
  // The natural data structure for that list is a priority queue, which we're
  // using, but the fact that our list of points is sorted based on their
  // distance to a reference point means that we need a little extra plumbing
  // in order to pull that off.  Hence, the TreeNeighbor class.
 public:
  friend class NearestNeighborPoint;
  TreeNeighbor(AngularCoordinate& reference_ang,
	       uint8_t n_neighbors = 1);
  ~TreeNeighbor();

  // Return a list of the nearest neighbors found so far.
  void NearestNeighbors(WAngularVector& w_ang, bool save_neighbors = true);

  // Return the number of neighbors in the list.  This should always be at most
  // the value used to instantiate the class.
  inline uint8_t Neighbors() {
    return ang_queue_.size();
  };

  inline uint8_t MaxNeighbors() {
    return n_neighbors_;
  };

  // Submit a point for possible inclusion.  Return value indicates whether the
  // point was successfully included in the list (i.e., the distance between
  // the input point and the reference point was smaller than the current most
  // distant point in the list) or not.
  bool TestPoint(WeightedAngularCoordinate* test_ang);

  // Return the maximum distance of the current list.
  inline double MaxDistance() {
    return max_distance_;
  };

  // The default distance returned is in sin^2(theta) units since that's what
  // the edge detection code uses.  If we're interested in human units, this
  // provides that distance in degrees.
  inline double MaxAngularDistance() {
    // Numerical precision can sometimes make the max_distance_ negative.
    return Stomp::RadToDeg*asin(sqrt(fabs(max_distance_)));
  };

  // For accounting purposes, it can be useful to keep track of how many nodes
  // we have visited during our traversal through the tree.
  inline uint16_t NodesVisited() {
    return n_nodes_visited_;
  };
  inline void AddNode() {
    n_nodes_visited_++;
  };

 private:
  AngularCoordinate reference_ang_;
  PointQueue ang_queue_;
  uint8_t n_neighbors_;
  uint16_t n_nodes_visited_;
  double max_distance_;
};

typedef std::map<const uint32_t, int16_t> RegionDict;
typedef RegionDict::iterator RegionIterator;
typedef std::pair<RegionIterator, RegionIterator> RegionPair;

typedef std::map<const int16_t, double> RegionAreaDict;
typedef RegionAreaDict::iterator RegionAreaIterator;
typedef std::pair<RegionAreaIterator, RegionAreaIterator> RegionAreaPair;

class RegionMap {
  // This class provides the functionality for dividing the area subtended by a
  // BaseMap-derived object into roughly equal-area, equal-sized regions.  The
  // class is not intended to be instantiated outside of the BaseMap class.

 public:
  RegionMap();
  virtual ~RegionMap();

  // This method initializes the regions on our current map.  There are two
  // parameters: the resolution to use for breaking up the current map and
  // the number of regions we should divide the current map into.  The higher
  // the resolution value, the more precise our split will be at the expense
  // of more memory.  Resolution values above 2048 are forbidden.  If no
  // resolution value is given, the code will attempt to find a reasonable
  // value based on the requested number of regions.
  //
  // The number of regions can't exceed the number of pixels at the specified
  // resolution for obvious reasons.  Likewise, as the number of regions
  // increases, our ability to make them equal area becomes more constrained.
  // So, don't go crazy here.  The return value is the number of regions that
  // we used in the final splitting, in case the specified number had to be
  // reduced to match the available number of pixels.
  uint16_t InitializeRegions(BaseMap* stomp_map, uint16_t n_region,
			     uint16_t region_resolution = 0);

  // Alternatively, we could import our region map from another BaseMap.  The
  // return value indicates success or failure.
  bool InitializeRegions(BaseMap* base_map, BaseMap& source_map);

  // Once we have the map divided into sub-regions, there are number of things
  // we might do.  The simplest would be to take in an AngularCoordinate object
  // and return the index of the sub-region that contained that point.  If
  // the point is not in any of the regions (and hence, outside of the map),
  // then the return value is -1.
  int16_t FindRegion(AngularCoordinate& ang);

  // And finally, a method for removing the current sub-region setup so that
  // a new version can be imposed on the map.  This method is called before
  // InitializeRegions does anything, so two successive calls to
  // InitializeRegions won't cause problems.
  void ClearRegions();

  // Given a region index, return a Map corresponding to its area.
  void RegionAreaMap(int16_t region, Map& stomp_map);

  // Given a pixel index (the Pixnum method in Pixel), return the corresponding
  // region value.
  inline int16_t Region(uint32_t region_idx) {
    return (region_map_.find(region_idx) != region_map_.end() ?
	    region_map_[region_idx] : -1);
  };
  // Given a region index, return the area associated with that region.
  inline double RegionArea(int16_t region) {
    return (region_area_.find(region) != region_area_.end() ?
	    region_area_[region] : 0.0);
  };

  inline uint16_t NRegion() {
    return n_region_;
  };
  inline uint16_t Resolution() {
    return region_resolution_;
  };
  inline bool Initialized() {
    return (n_region_ > 0 ? true : false);
  };

  // Return iterators for the set of RegionMap objects.
  inline RegionIterator Begin() {
    return region_map_.begin();
  };
  inline RegionIterator End() {
    return region_map_.end();
  };

 private:
  RegionDict region_map_;
  RegionAreaDict region_area_;
  uint16_t region_resolution_, n_region_;
};


class BaseMap {
  // This is the abstract base class that all of the map classes will inherit,
  // prototyping all of the basic map functionality.  In particular, it
  // includes all of the functionality for dividing the map area with the
  // RegionMapper sub-class.  This class should never be instantiated directly.
 public:
  BaseMap();
  virtual ~BaseMap();

  // These four methods are the core methods required for running the
  // RegionMapper code.  It only calls Coverage directly, but the Weight value
  // stored in the Pixels returned indicates the fraction of that pixel
  // contained in the map.  This calculation in turn requires FindUnmaskedArea
  // and FindUnmaskedStatus.  Covering is added as useful description of the
  // overall BaseMap geometry.  See the Map class for more thorough
  // documentation of each of these methods.
  virtual void Coverage(PixelVector& superpix,
			uint16_t resolution = Stomp::HPixResolution);
  virtual bool Covering(Map& stomp_map, uint32_t maximum_pixels);
  virtual double FindUnmaskedFraction(Pixel& pix);
  virtual int8_t FindUnmaskedStatus(Pixel& pix);

  // In addition, each map instance should have some basic book-keeping methods
  // for determining whether the map contains any data (Empty), removing any
  // data (Clear), returning the size of the data load (Size), and giving a
  // notion of the subtended area (Area).
  virtual bool Empty();
  virtual void Clear();
  virtual uint32_t Size();
  virtual double Area();

  // Further, each map needs to give a sense of the range of pixel resolutions
  // involved.  This is important because the RegionMap can't be created at
  // a resolution smaller than the map itself can resolve.
  virtual uint16_t MinResolution();
  virtual uint16_t MaxResolution();

  // These methods all act as wrappers for the RegionMapper object contained
  // in the class.  See that class for documentation.
  inline uint16_t InitializeRegions(uint16_t n_regions,
				    uint16_t region_resolution = 0) {
    return region_map_.InitializeRegions(this, n_regions, region_resolution);
  };
  inline bool InitializeRegions(BaseMap& scalar_map) {
    return region_map_.InitializeRegions(this, scalar_map);
  };
  inline int16_t FindRegion(AngularCoordinate& ang) {
    return region_map_.FindRegion(ang);
  };
  inline void ClearRegions() {
    region_map_.ClearRegions();
  };
  inline void RegionAreaMap(int16_t region, Map& stomp_map) {
    region_map_.RegionAreaMap(region, stomp_map);
  };
  inline int16_t Region(uint32_t region_idx) {
    return region_map_.Region(region_idx);
  };
  inline double RegionArea(int16_t region) {
    return region_map_.RegionArea(region);
  };
  inline uint16_t NRegion() {
    return region_map_.NRegion();
  };
  inline uint16_t RegionResolution() {
    return region_map_.Resolution();
  };
  inline bool RegionsInitialized() {
    return region_map_.Initialized();
  };
  inline RegionIterator RegionBegin() {
    return region_map_.Begin();
  };
  inline RegionIterator RegionEnd() {
    return region_map_.End();
  };

 private:
  RegionMap region_map_;
};


typedef std::vector<SubMap> SubMapVector;
typedef SubMapVector::iterator SubMapIterator;
typedef std::pair<SubMapIterator, SubMapIterator> SubMapPair;

class SubMap {
  // While the preferred interface for interacting with a Map is through
  // that class, the actual work is deferred to the SubMap class.  Each
  // instance contains all of the pixels for a corresponding superpixel as well
  // as some summary statistics.  All of the operations done on the Map
  // end up calling corresponding methods for each of the SubMap instances
  // contained in that Map.  See the comments around those methods in the
  // Map class declaration for an explaination of what each method does.

 public:
  SubMap(uint32_t superpixnum);
  ~SubMap();
  void AddPixel(Pixel& pix);
  void Resolve(bool force_resolve = false);
  void SetMinimumWeight(double minimum_weight);
  void SetMaximumWeight(double maximum_weight);
  void SetMaximumResolution(uint16_t maximum_resolution, bool average_weights);
  bool FindLocation(AngularCoordinate& ang, double& weight);
  double FindUnmaskedFraction(Pixel& pix);
  int8_t FindUnmaskedStatus(Pixel& pix);
  double FindAverageWeight(Pixel& pix);
  void FindMatchingPixels(Pixel& pix, PixelVector& match_pix,
			  bool use_local_weights = false);
  double AverageWeight();
  void Soften(PixelVector& softened_pix, uint16_t maximum_resolution,
	      bool average_weights);
  bool Add(Map& stomp_map, bool drop_single);
  bool Multiply(Map& stomp_map, bool drop_single);
  bool Exclude(Map& stomp_map);
  void ScaleWeight(const double weight_scale);
  void AddConstantWeight(const double add_weight);
  void InvertWeight();
  void Pixels(PixelVector& pix);
  void CheckResolution(uint16_t resolution);
  void Clear();
  inline uint32_t Superpixnum() {
    return superpixnum_;
  };
  inline PixelIterator Begin() {
    return (initialized_ ? pix_.begin() : pix_.end());
  };
  inline PixelIterator End() {
    return pix_.end();
  };
  inline double Area() {
    return area_;
  };
  inline bool Initialized() {
    return initialized_;
  };
  inline bool Unsorted() {
    return unsorted_;
  };
  inline uint16_t MinResolution() {
    return min_resolution_;
  };
  inline uint16_t MaxResolution() {
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
  inline uint32_t Size() {
    return size_;
  };
  inline uint32_t PixelCount(uint16_t resolution) {
    return (!(resolution % 2) ? pixel_count_[resolution] : 0);
  };

private:
  uint32_t superpixnum_, size_;
  PixelVector pix_;
  double area_, lambda_min_, lambda_max_, eta_min_, eta_max_, z_min_, z_max_;
  double min_weight_, max_weight_;
  uint16_t min_resolution_, max_resolution_;
  bool initialized_, unsorted_;
  ResolutionDict pixel_count_;
};


class Section {
  // This is barely a class.  Really, it's just a small object that's necessary
  // for constructing the sub-regions in the Map classes.

 public:
  Section();
  ~Section();
  inline void SetMinStripe(uint32_t stripe) {
    stripe_min_ = stripe;
  };
  inline void SetMaxStripe(uint32_t stripe) {
    stripe_max_ = stripe;
  };
  inline uint32_t MinStripe() {
    return stripe_min_;
  };
  inline uint32_t MaxStripe() {
    return stripe_max_;
  };

 private:
  uint32_t stripe_min_, stripe_max_;
};

typedef std::pair<uint32_t, PixelIterator> MapIterator;
typedef std::pair<MapIterator, MapIterator> MapPair;

class Map : public BaseMap {
  // A Map is intended to function as a region on the sky whose geometry
  // is given by a set of Pixels of various resolutions which combine to
  // cover that area.  Since each Pixel has an associated weight, the map
  // can also encode a scalar field (temperature, observing depth, local
  // seeing, etc.) over that region.  A Map can be combined with other
  // Maps with all of the logical operators you would expect (union,
  // intersection and exclusion as well as addition and multiplication of the
  // weights as a function of position).  Likewise, you can test angular
  // positions and pixels against a Map to see if they are within it or
  // query the angular extent and area of a Map on the Sky.

 public:
  // The preferred constructor for a Map takes a vector of Pixels
  // as its argument.  However, it can be constructed from a properly formatted
  // ASCII text file as well.
  Map();
  Map(PixelVector& pix, bool force_resolve = true);
  Map(std::string& InputFile,
      const bool hpixel_format = true,
      const bool weighted_map = true);
  virtual ~Map();

  // Initialize is called to organize the Map internally.  Unless the
  // map is being reset with a new set of pixels, as in the second instance of
  // this method, Initialize should probably never be invoked.
  bool Initialize();
  bool Initialize(PixelVector& pix, bool force_resolve = true);

  // Simple call to determine if a point is within the current Map
  // instance.  Returns true if the point is within the map; false, otherwise.
  bool FindLocation(AngularCoordinate& ang);

  // An variation on FindLocation that also assigns a value to the input
  // "weight" reference variable.  If the location is within the Map, then
  // the weight of value of the map is stored in the "weight" variable.  If
  // not, then the value in "weight" is meaningless.
  bool FindLocation(AngularCoordinate& ang, double& weight);

  // Another variation on FindLocation, this is mostly in place for the Python
  // wrapper.  Instead of returning a boolean, this returns the weight value
  // for the Map at the input location.  If the location is not within
  // the map, then the default value of -1.0e30 is returned.
  double FindLocationWeight(AngularCoordinate& ang);

  // In the same spirit, we can pose similar queries for both Pixels and Maps.
  // Later on, we'll have more sophisticated indicators as to whether these
  // areas are fully, partially or not contained in our Map, but for now we only
  // consider the question of full containment.
  inline bool Contains(Pixel& pix) {
    return (FindUnmaskedStatus(pix) == 1 ? true : false);
  };
  inline bool Contains(Map& stomp_map) {
    return (FindUnmaskedStatus(stomp_map) == 1 ? true : false);
  };

  // Given a Pixel, this returns the fraction of that pixel's area that is
  // contained within the current map (0 <= fraction <= 1).  Alternatively, a
  // vector of pixels can be processed in a single call, in which case a
  // vector of coverages is returned or the unmasked fraction is stored in the
  // weight element of the vector of pixels.
  virtual double FindUnmaskedFraction(Pixel& pix);
  void FindUnmaskedFraction(PixelVector& pix,
                            std::vector<double>& unmasked_fraction);
  void FindUnmaskedFraction(PixelVector& pix);
  double FindUnmaskedFraction(Map& stomp_map);

  // Similar to FindUnmaskedFraction, these routines return an integer
  // indicating whether the pixel is fully contained in the map (1),
  // fully outside the map (0) or partially contained in the map (-1).  For
  // cases where we don't have to come up with an accurate number for the
  // map coverage of a given pixel, this should be faster.
  virtual int8_t FindUnmaskedStatus(Pixel& pix);
  void FindUnmaskedStatus(PixelVector& pix,
			  std::vector<int8_t>& unmasked_status);
  int8_t FindUnmaskedStatus(Map& stomp_map);

  // Similar to FindUnmaskedFraction, this returns the area-averaged weight of
  // the map over the area covered by the input pixel (or pixels).
  // AverageWeight does the same task, but over the entire Map.
  double FindAverageWeight(Pixel& pix);
  void FindAverageWeight(PixelVector& pix,
                         std::vector<double>& average_weight);
  void FindAverageWeight(PixelVector& pix);
  double AverageWeight();

  // This is part of the process for finding the intersection between two maps.
  // For a given pixel, we return the pixels in our map that are contained
  // within that test pixel.  If use_local_weights is set to true, then the
  // pixel weights are set to match the weights in the current map.  If not,
  // then the matching pixels are set to the weight from the test Pixel.
  void FindMatchingPixels(Pixel& pix,
			  PixelVector& match_pix,
			  bool use_local_weights = false);
  void FindMatchingPixels(PixelVector& pix,
			  PixelVector& match_pix,
			  bool use_local_weights = false);

  // Return a vector of SuperPixels that cover the Map.  This serves two
  // purposes.  First, it acts as a rough proxy for the area of the current
  // map, which can occasionally be useful.  More importantly, all of the real
  // work in a Map is done on a superpixel-by-superpixel basis, so this
  // becomes an important thing to know when querying the map.
  //
  // If the resolution argument is given, then the resulting set of pixels will
  // be generated at that resolution instead.  In either case, the weight for
  // the pixels will reflect the fraction of the pixel that is within the
  // current map.
  virtual void Coverage(PixelVector& superpix,
			uint16_t resolution = Stomp::HPixResolution);

  // Instead of a set of vectors at the same resolution, we may want to
  // generate a lower resolution version of our current map where the needs of
  // matching the geometry of a given region on the sky can be compromised a
  // bit in the interest of a smaller memory footprint.  In this case, the
  // return object is another Map of greater or equal area which is
  // composed of at most maximum_pixels pixels.  In some cases, the maximum
  // number of pixels is smaller than the number of superpixels in the current
  // map.  In that case, the returned boolean is false and the stomp_map is
  // composed of the superpixels for the current map.  If the method is able
  // to generate a map with at most maximum_pixels the return value is true.
  //
  // Unlike Coverage, the weights in the returned stomp_map will be based on
  // the area-averaged weights in the current Map.
  virtual bool Covering(Map& stomp_map, uint32_t maximum_pixels);

  // As a middle ground between these two cases, we may want a variation of
  // Coverage that outputs a Map where we have reduced the maximum resolution
  // of our current map.  Again, the geometry will be less precise than the
  // current map, but the total number of pixels should be smaller (but not
  // explicitly specified as with Covering).  If the input resolution is larger
  // than the current maximum resolution, then the returned map will be a
  // copy of the current map.  If the average_weights flag is set to true, then
  // the resulting map will retain the current map's weight, averaging the
  // weight for any pixels which are resampled.  If set to false, the resulting
  // map will have unity weight, except for the pixels were resampled, where the
  // value will indicate the included fraction.
  void Soften(Map& stomp_map, uint16_t maximum_resolution,
	      bool average_weights=false);

  // Rather than creating a new Map, we can Soften the current Map
  void Soften(uint16_t maximum_resolution, bool average_weights=false);

  // In addition to modifying the maximum resolution of the Map (which is
  // basically what Soften does), we can also filter the current Map based on
  // the Weight, removing any area that violates the Weight limits.
  void SetMinimumWeight(double min_weight);
  void SetMaximumWeight(double max_weight);

  // After we initialize regions on this Map, we might want to produce a Map
  // corresponding to a given region.  The boolean here is to indicate if the
  // Map was properly constructed or not, depending on whether the specified
  // region index was within the valid range: 0 < region_index < n_region-1.
  bool RegionOnlyMap(int16_t region_index, Map& stomp_map);

  // Conversely, we often want to know what our map looks like when we've
  // excluded a specific region, as you'd do if you were using jack-knife error
  // estimates.  Similar operation as RegionMap with regards to the returned
  // boolean.
  bool RegionExcludedMap(int16_t region_index, Map& stomp_map);

  // Given a requested number of points, return a vector of Poisson random
  // angular positions within the current Map's area.
  //
  // If the use_weighted_sampling flag is set to true, then the local weight is
  // taken into account when generating random points.  In this case, a pixel
  // with the same area but twice the weight as another pixel should, in the
  // limit of infinite realizations, have twice as many points as the
  // lower-weighted one.
  void GenerateRandomPoints(AngularVector& ang,
                            uint32_t n_point = 1,
                            bool use_weighted_sampling = false);

  // Instead of a fixed number of random positions, we may have either a
  // vector of WeightedAngularCoordinate objects where we want to randomize
  // the positions or a vector of weights that need random angular positions
  // to go along with them.  In either case, the number of random positions
  // is taken from the size of the input vector.
  void GenerateRandomPoints(WAngularVector& ang, WAngularVector& input_ang);
  void GenerateRandomPoints(WAngularVector& ang, std::vector<double>& weights);

  // The book-end to the initialization method that takes an ASCII filename
  // as an argument, this method writes the current map to an ASCII file using
  // the same formatting conventions.
  bool Write(std::string& OutputFile, bool hpixel_format = true,
             bool weighted_map = true);

  // Alternatively, if the default constructor is used, this method will
  // re-initialize the map with the contents of the input file.
  bool Read(std::string& InputFile, const bool hpixel_format = true,
	    const bool weighted_map = true);

  // Three simple functions for performing the same operation on the weights
  // of all of the pixels in the current map.  These are prelude to the next
  // set of functions for doing logical and arithmetic operations on Maps.
  void ScaleWeight(const double weight_scale);
  void AddConstantWeight(const double add_weight);
  void InvertWeight();

  // Now we begin the core of our class, the ability to treat the Map as
  // an abstract object which we can combine with other maps to form arbitrarily
  // complicated representations on the sphere.
  //
  // Starting simple, IngestMap simply takes the area associated with another
  // map and combines it with the current map.  If pixels overlap between the
  // two maps, then the weights are set to the average of the two maps.
  // Returns true if the procedure succeeded, false otherwise.
  bool IngestMap(PixelVector& pix, bool destroy_copy = true);
  bool IngestMap(Map& stomp_map, bool destroy_copy = true);

  // Now we have intersection.  This method finds the area of intersection
  // between the current map and the argument map and makes that the new area
  // for this map.  Weights are drawn from the current map's values.  If there
  // is no overlapping area, the method returns false and does nothing.  A
  // true response indicates that the area of the current map has changed to
  // the overlapping area between the two and the area is non-zero.
  bool IntersectMap(PixelVector& pix);
  bool IntersectMap(Map& stomp_map);

  // The inverse of IntersectMap, ExcludeMap removes the area associated with
  // the input map from the current map.  If this process would remove all of
  // the area from the current map, then the method returns false and does not
  // change the current map.  A true response indicates that the input maps has
  // been excluded and area remains.
  bool ExcludeMap(PixelVector& pix, bool destroy_copy = true);
  bool ExcludeMap(Map& stomp_map, bool destroy_copy = true);

  // Two sets of methods that operate on the weights between two different
  // maps.  The first set adds the weights of the two maps and the second
  // takes their product.  The drop_single boolean indicates whether the
  // non-overlapping area should be excluded (true) or not (false).  If
  // drop_single is set to false, then the areas where the two maps don't
  // overlap will have their weights set to whatever they are in the map that
  // covers that area.
  bool AddMap(PixelVector& pix, bool drop_single = true);
  bool AddMap(Map& stomp_map, bool drop_single = true);
  bool MultiplyMap(PixelVector& pix, bool drop_single = true);
  bool MultiplyMap(Map& stomp_map, bool drop_single = true);

  // Like IntersectMap, except that the current map takes on the weight
  // values from the map given as the argument.
  bool ImprintMap(PixelVector& pix);
  bool ImprintMap(Map& stomp_map);

  // Simple method for returning the vector representation of the current
  // Map.  If a superpixel index is given as the second argument, then
  // just the pixels for that superpixel are returned.
  void Pixels(PixelVector& pix,
              uint32_t superpixnum = Stomp::MaxSuperpixnum);

  // For a more efficient way to iterate through the pixels that make up the
  // Map, we have these methods.  The standard for loop to iterate through all
  // of the Pixels in a Map would be
  //
  // for (MapIterator iter=stomp_map.Begin();
  //      iter!=stomp_map.End();stomp_map.Iterate(&iter)) {
  //   double weight = iter.second->Weight();
  // }
  inline MapIterator Begin() {
    return begin_;
  };
  inline MapIterator End() {
    return end_;
  };
  void Iterate(MapIterator* iter);

  // Resets the Map to a completely clean slate.  No pixels, no area.
  virtual void Clear();
  void Clear(uint32_t superpixnum) {
    if (superpixnum < Stomp::MaxSuperpixnum)
      sub_map_[superpixnum].Clear();
  };

  // Simple in-line method for checking to see if the current map has any area
  // in a given superpixel.
  inline bool ContainsSuperpixel(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
            sub_map_[superpixnum].Initialized() : false);
  };

  // Some general methods for querying the state of the current map.
  inline virtual double Area() {
    return area_;
  };
  inline double Area(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].Area() : 0.0);
  };
  inline virtual uint16_t MinResolution() {
    return min_resolution_;
  };
  inline uint16_t MinResolution(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].MinResolution() : 0);
  };
  inline virtual uint16_t MaxResolution() {
    return max_resolution_;
  };
  inline uint16_t MaxResolution(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].MaxResolution() : 0);
  };
  inline double MinWeight() {
    return min_weight_;
  };
  inline double MinWeight(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].MinWeight() : 0.0);
  };
  inline double MaxWeight() {
    return max_weight_;
  };
  inline double MaxWeight(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].MaxWeight() : 0.0);
  };
  inline virtual uint32_t Size() {
    return size_;
  };
  inline uint32_t Size(uint32_t superpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
	    sub_map_[superpixnum].Size() : 0);
  };
  inline virtual bool Empty() {
    return (size_ == 0 ? true : false);
  };
  inline uint32_t PixelCount(uint16_t resolution) {
    return (!(resolution % 2) ? pixel_count_[resolution] : 0);
  };


private:
  SubMapVector sub_map_;
  MapIterator begin_, end_;
  double area_, min_weight_, max_weight_;
  uint16_t min_resolution_, max_resolution_;
  uint32_t size_;
  ResolutionDict pixel_count_;
};


typedef std::vector<ScalarSubMap> ScalarSubMapVector;
typedef ScalarSubMapVector::iterator ScalarSubMapIterator;
typedef std::pair<ScalarSubMapIterator, ScalarSubMapIterator> ScalarSubMapPair;

class ScalarSubMap {
  // For the ScalarMap, the sub-map class is more about book-keeping
  // than operations.  They are still assembled around the superpixels, but
  // since the methods need to talk to pixels from different superpixels so
  // often it doesn't make sense to break up the data to the same degree.

 public:
  ScalarSubMap(uint32_t superpixnum);
  ~ScalarSubMap();
  inline void AddToArea(uint16_t resolution, double weight) {
    area_ +=
        weight*Stomp::HPixArea*
        Stomp::HPixResolution*Stomp::HPixResolution/
        (resolution*resolution);
    size_++;
  };
  inline void AddToIntensity(const double intensity,
			     const uint16_t n_point = 1) {
    total_intensity_ += intensity;
    total_points_ += n_point;
  };
  inline void SetIntensity(const double intensity) {
    total_intensity_ = intensity;
  };
  inline void SetNPoints(const int n_point) {
    total_points_ = n_point;
  };
  inline double Area() {
    return area_;
  };
  inline double Intensity() {
    return (initialized_ ? total_intensity_ : 0.0);
  };
  inline int NPoints() {
    return (initialized_ ? total_points_ : 0);
  };
  inline double Density() {
    return (initialized_ ? total_intensity_/area_ : 0.0);
  };
  inline double PointDensity() {
    return (initialized_ ? static_cast<double>(total_points_)/area_ : 0.0);
  };
  inline void SetBegin(ScalarIterator iter) {
    start_ = iter;
    finish_ = ++iter;
    initialized_ = true;
  };
  inline void SetEnd(ScalarIterator iter) {
    finish_ = ++iter;
  };
  inline void SetNull(ScalarIterator iter) {
    null_ = iter;
  };
  inline ScalarIterator Begin() {
    return (initialized_ ? start_ : null_);
  };
  inline ScalarIterator End() {
    return (initialized_ ? finish_ : null_);
  };
  inline bool Initialized() {
    return initialized_;
  };
  inline uint32_t Size() {
    return (initialized_ ? size_ : 0);
  };

 private:
  uint32_t superpixnum_;
  ScalarIterator start_;
  ScalarIterator finish_;
  ScalarIterator null_;
  bool initialized_;
  double area_, total_intensity_;
  int total_points_;
  uint32_t size_;
};


typedef std::vector<ScalarMap> ScalarMapVector;
typedef ScalarMapVector::iterator ScalarMapIterator;
typedef std::pair<ScalarMapIterator, ScalarMapIterator> ScalarMapPair;

class ScalarMap : public BaseMap {
  // Unlike a Map, where the set of Pixels is intended to match the
  // geometry of a particular region, ScalarMaps are intended to be
  // a regular sampling map of a given scalar field over some region.  The
  // area covered by the map will be approximately the same as that covered
  // by the pixels in the map, but each pixel is assumed to have some covering
  // fraction to indicate what percentage of the map is in the underlying
  // region.  To phrase things another way, once you have a Map describing
  // the extent of some data set, a ScalarMap is what you would use to
  // calculate clustering statistics on data contained in that region.

 public:
  // The ScalarPixels in a ScalarMap all have the same resolution.
  // Hence, we can construct a blank map from a Map (essentially
  // re-sampling the Map at a fixed resolution) or another ScalarMap,
  // so long as that map has higher resolution than the one we're trying to
  // construct.
  //
  // As described in the ScalarPixel class, there are three basic use cases:
  //
  // * Pure scalar field (e.g. CMB temperature or radio flux).
  // * Point-based density (e.g. the projected galaxy density over some area).
  // * Point-sampled field (e.g. the mean galaxy magnitude over some area).
  //
  // The way that we'll interact with the map will vary somewhat with each
  // case.  To make sure that we're doing the correct thing, we encode which
  // of these regimes we're operating under with the ScalarMapType enum:
  enum ScalarMapType {
    ScalarField,
    DensityField,
    SampledField
  };

  ScalarMap();

  // Initialize a ScalarMap based on the geometry of an input Map.  If the
  // use_map_weight_as_intensity flag is set to true, the MapType will be
  // set to ScalarField regardless of the value used in calling the
  // constructor.  A warning will be issued if the input value is not
  // "ScalarField".
  ScalarMap(Map& stomp_map,
	    uint16_t resolution,
	    ScalarMapType scalar_map_type = ScalarField,
	    double min_unmasked_fraction = 0.0000001,
	    bool use_map_weight_as_intensity = false);

  // If the map used to initialize the current one is also a ScalarMap, then
  // the ScalarMapType will be based on the input map, as will the geometry.
  ScalarMap(ScalarMap& scalar_map,
	    uint16_t resolution,
	    double min_unmasked_fraction = 0.0000001);

  // Initialize based on a vector of ScalarPixels.  If the input vector contains
  // pixels with heterogeneous resolutions, the code will exit automatically.
  ScalarMap(ScalarVector& pix,
	    ScalarMapType scalar_map_type = ScalarField,
	    double min_unmasked_fraction = 0.0000001);

  // This may seem a bit of an oddity, but needing roughly circular patches
  // from maps comes up more frequently than one might think.  Or not.
  ScalarMap(Map& stomp_map,
	    AngularCoordinate& center,
	    double theta_max,
	    uint16_t resolution,
	    ScalarMapType scalar_map_type = ScalarField,
	    double min_unmasked_fraction = 0.0000001,
	    double theta_min = -1.0);
  virtual ~ScalarMap();

  // Internal method that should probably never be called.
  bool _InitializeSubMap();

  // This is generally set through the constructor.  However, if
  // you want to re-initialize the same object with different parameters or
  // use the constructor without any arguments, this will set the
  // resolution of the map.
  inline void SetResolution(uint16_t resolution) {
    Clear();
    resolution_ = resolution;
  };

  // If you change the map resolution, then you will need to re-initialize
  // the coverage of the map from either a Map or a higher resolution
  // ScalarMap.  These methods allow for that functionality.  In each
  // case, if a resolution is supplied it will over-ride the current map's
  // resolution value.  This will also reset any previously set region
  // information.
  void InitializeFromMap(Map& stomp_map, uint16_t resolution=0,
			 bool use_map_weight_as_intensity = false);
  void InitializeFromScalarMap(ScalarMap& scalar_map, uint16_t resolution=0);

  // If you re-initialize from a vector of ScalarPixels, the resolution of
  // those pixels will automatically over-ride the current map's resolution
  // value.
  void InitializeFromScalarPixels(ScalarVector& pix,
				  ScalarMapType map_type = ScalarField);

  // Once we have our map set up, we'll want to add data points to it.  This
  // method offers two variations on that task.  If the MapType is ScalarField,
  // then the corresponding pixel will take on the value of the weight attached
  // to the input object.  Hence, adding another object which is located in
  // the same pixel will over-ride the old weight value with the new one.  In
  // all cases, the return value is false if the object doesn't localize to any
  // pixel in the map.
  bool AddToMap(AngularCoordinate& ang, double object_weight = 1.0);
  bool AddToMap(WeightedAngularCoordinate& ang);

  // Alternatively, if we are encoding a pure scalar field, then this method
  // will import the weight value from the input pixel into the proper fields.
  // If the input pixel is at a higher resolution than the current resolution
  // of the ScalarMap or the ScalarMap type is not ScalarField, the return
  // value is false.  For wholesale sampling from a Map, use InitializeFromMap.
  bool AddToMap(Pixel& pix);

  // Finally, if we want to set the map values directly, we can add a
  // ScalarPixel to the map.  If the resolution of the pixel doesn't match the
  // map resolution or the pixel can't be found in the map, the return value is
  // false.
  bool AddToMap(ScalarPixel& pix);


  // The instance of the same methods from the BaseMap class.  Note that, if
  // the input pixel is at a higher resolution than the current map,
  // FindUnmaskedFraction will return an invalid value (-1.0).  Likewise,
  // Coverage will over-ride the input resolution value if it is higher than
  // the map resolution.  FindUnmaskedStatus will work for higher resolution
  // pixels, but values indicating that the pixel is within the map don't
  // necessarily mean the same as they do for the Map class.
  virtual void Coverage(PixelVector& superpix,
			uint16_t resolution = Stomp::HPixResolution);
  virtual bool Covering(Map& stomp_map, uint32_t maximum_pixels);
  virtual double FindUnmaskedFraction(Pixel& pix);
  virtual int8_t FindUnmaskedStatus(Pixel& pix);


  // If we're converting a map from high to low resolution, this method
  // re-calculates the weight and intensity parameters for a given lower
  // resolution pixel.  Exactly how this is done will depend on the
  // ScalarMapType.  For a ScalarField, the resampling will be an
  // area-weighted average.  For DensityField and SampledField, the resampling
  // will be a direct sum of the intensity and points.  Resample will also
  // handle the case where the map has been converted to an over-density (the
  // resampled pixel will be based on the raw values).
  void Resample(ScalarPixel& pix);


  // When we access the field properties of the map, there are three options.
  // We can return the average intensity for the input pixel, the "density"
  // (intensity over the unmasked area), or the "point density" (number of
  // points over the unmasked area).  Which of these three will be most
  // meaningful will depend on the map type.
  double FindIntensity(Pixel& pix);
  double FindDensity(Pixel& pix);
  double FindPointDensity(Pixel& pix);


  // These four methods allow you to sample the area, intensity and density
  // within a given annular area around a point (where the inner radius is
  // set to zero by default.  The angular bounds (theta_min and theta_max) are
  // taken to be in degrees.
  double FindLocalArea(AngularCoordinate& ang, double theta_max,
		       double theta_min = -1.0);
  double FindLocalIntensity(AngularCoordinate& ang, double theta_max,
			    double theta_min = -1.0);
  double FindLocalDensity(AngularCoordinate& ang, double theta_max,
			  double theta_min = -1.0);
  double FindLocalPointDensity(AngularCoordinate& ang, double theta_max,
			       double theta_min = 0.0);


  // In the case where one is adding data points to the map, once this is
  // done, those object counts will need to be translated into a local measure
  // of the fractional over-density.  If the mean density of the map is all
  // that's required, then the first method will do that.  If you want to
  // replace the current data counts with the fractional over-density the
  // second method will do that (and call the first method if you have not
  // already done so).  And if you need to convert back from over-density to
  // raw values, the final method will do that for you.
  void CalculateMeanIntensity();
  void ConvertToOverDensity();
  void ConvertFromOverDensity();


  // Given a Map, we export the scalar field in the current
  // ScalarMap into its weight values.  This will naturally perform an
  // intersection between the areas covered by the two maps.  If there is no
  // overlapping area, then the method returns false and the input Map
  // will not be modified.  A true result means that there was at least some
  // overlap.
  bool ImprintMap(Map& stomp_map);


  // This method calculates the auto-correlation of the scalar field in the
  // current map.  The simpler case is the one where the argument is an
  // iterator for an angular bin.  Given the angular extent of that bin, the
  // code will find the auto-correlation of the field.  If the second option
  // is used, then the code will find the auto-correlation for all of the
  // angular bins whose resolution values match that of the current map.
  void AutoCorrelate(ThetaIterator theta_iter);
  void AutoCorrelate(AngularCorrelation& wtheta);


  // Alternatively, we may want to do the auto-correlation using the regions
  // we've divided the area into as a set of jack-knife samples.  These variants
  // will measure the auto-correlation for each of the jack-knife samples
  // simultaneously.
  void AutoCorrelateWithRegions(AngularCorrelation& wtheta);
  void AutoCorrelateWithRegions(ThetaIterator theta_iter);


  // Same as the auto-correlation methods, except the current map is
  // cross-correlated with another DensityMap.  Only areas of overlap are
  // considered in the cross-correlation.
  void CrossCorrelate(ScalarMap& scalar_map, AngularCorrelation& wtheta);
  void CrossCorrelate(ScalarMap& scalar_map, ThetaIterator theta_iter);
  void CrossCorrelateWithRegions(ScalarMap& scalar_map,
				 AngularCorrelation& wtheta);
  void CrossCorrelateWithRegions(ScalarMap& scalar_map,
				 ThetaIterator theta_iter);


  // Meaningful, since all of the pixels in the map share a common resolution.
  inline uint16_t Resolution() {
    return resolution_;
  };

  // Some global methods for accessing the aggregate area, intensity, density
  // and so for for the map.  If a superpixnum index is given as an
  // argument, then the values for that pixel are returned.  Otherwise, the
  // values for the whole map are given.  The extent to which these values
  // are meaningful will depend on the type of scalar field encoded in the map.
  inline virtual double Area() {
    return area_;
  };
  inline double Area(uint32_t superpixnum) {
    return sub_map_[superpixnum].Area();
  };
  inline double Intensity() {
    return total_intensity_;
  };
  inline double Intensity(uint32_t superpixnum) {
    return sub_map_[superpixnum].Intensity();
  };
  inline int NPoints() {
    return total_points_;
  };
  inline int NPoints(uint32_t superpixnum) {
    return sub_map_[superpixnum].NPoints();
  };
  inline double Density() {
    return total_intensity_/area_;
  };
  inline double Density(uint32_t superpixnum) {
    return sub_map_[superpixnum].Density();
  };
  inline double PointDensity() {
    return 1.0*total_points_/area_;
  };
  inline double PointDensity(uint32_t superpixnum) {
    return sub_map_[superpixnum].PointDensity();
  };
  inline ScalarIterator Begin(uint32_t superpixnum =
			      Stomp::MaxSuperpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
            sub_map_[superpixnum].Begin() : pix_.begin());
  };
  inline ScalarIterator End(uint32_t superpixnum =
			    Stomp::MaxSuperpixnum) {
    return (superpixnum < Stomp::MaxSuperpixnum ?
            sub_map_[superpixnum].End() : pix_.end());
  };
  inline virtual uint32_t Size() {
    return pix_.size();
  };
  inline uint32_t Size(uint32_t superpixnum) {
    return sub_map_[superpixnum].Size();
  };

  // We need these methods to comply with the BaseMap signature.
  inline virtual uint16_t MinResolution() {
    return resolution_;
  };
  inline virtual uint16_t MaxResolution() {
    return resolution_;
  };

  inline virtual bool Empty() {
    return (pix_.empty() ? true : false);
  };
  inline virtual void Clear() {
    area_ = 0.0;
    resolution_ = -1;
    total_points_ = 0;
    mean_intensity_ = 0.0;
    total_intensity_ = 0.0;
    if (!pix_.empty()) pix_.clear();
    if (!sub_map_.empty()) sub_map_.clear();
    converted_to_overdensity_ = false;
    calculated_mean_intensity_ = false;
    ClearRegions();
  };
  inline double MeanIntensity() {
    if (!calculated_mean_intensity_) CalculateMeanIntensity();
    return mean_intensity_;
  };
  inline bool IsOverDensityMap() {
    return converted_to_overdensity_;
  }
  inline ScalarMapType MapType() {
    return map_type_;
  };

 private:
  ScalarVector pix_;
  ScalarSubMapVector sub_map_;
  ScalarMapType map_type_;
  double area_, mean_intensity_, unmasked_fraction_minimum_, total_intensity_;
  uint16_t resolution_, total_points_;
  bool converted_to_overdensity_, calculated_mean_intensity_;
  bool initialized_sub_map_;
};


typedef std::vector<TreeMap> TreeMapVector;
typedef TreeMapVector::iterator TreeMapIterator;
typedef std::pair<TreeMapIterator, TreeMapIterator> TreeMapPair;

class TreeMap : public BaseMap {
  // Another variation on the Map.  Unlike the Map and DensityMap
  // classes, TreeMap objects start off with no defined geometry.  Instead,
  // as AngularCoordinate objects are placed into the TreeMap instance,
  // the map automatically generates the necessary nodes for storing the data.
  // The resulting structure can then be used to very quickly find object
  // pairs on a number of different scales.

 public:
  friend class NearestNeighborPixel;
  // Since the geometry is specified when points are added to the map, the
  // number of parameters for the constructor is potentially very small.  By
  // default the constructor will set things up to have the base resolution of
  // the tree match the resolution of the superpixels and the maximum number of
  // points per node to be 50.  The latter is somewhat arbitrary.  However, the
  // resolution for the base level of the map is important if you want to do
  // pair counting in conjunction with a DensityMap that uses sub-regions.
  // The region resolution for that DensityMap must match the resolution
  // chosen for the base level of the TreeMap.
  TreeMap(uint16_t resolution=Stomp::HPixResolution,
	  uint16_t maximum_points=50);
  ~TreeMap();

  // The primary purpose of this class is to enable fast pair-finding for a
  // set of angular locations.  These methods implement that functionality
  // with a couple different modes of operation.  FindPairs returns an integer
  // counting of the AngularCoordinates within the specified radius or
  // annulus.  FindWeightedPairs does the same, but the value returned is
  // the sum of the weights for the objects satisfying the angular bounds.  Note
  // that the argument in this case is still an AngularCoordinate, so any
  // weight associated with that point is ignored.  The AngularCorrelation
  // versions put the number of pairs in the Counter and Weight values for each
  // angular bin.
  uint32_t FindPairs(AngularCoordinate& ang, AngularBin& theta);
  uint32_t FindPairs(AngularCoordinate& ang,
			  double theta_min, double theta_max);
  uint32_t FindPairs(AngularCoordinate& ang, double theta_max);
  void FindPairs(AngularCoordinate& ang, AngularCorrelation& wtheta);
  void FindPairs(AngularVector& ang, AngularBin& theta);
  void FindPairs(AngularVector& ang, AngularCorrelation& wtheta);

  double FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta);
  double FindWeightedPairs(AngularCoordinate& ang,
			   double theta_min, double theta_max);
  double FindWeightedPairs(AngularCoordinate& ang, double theta_max);
  void FindWeightedPairs(AngularVector& ang, AngularBin& theta);
  void FindWeightedPairs(AngularVector& ang, AngularCorrelation& wtheta);

  // And for the case where we want to scale things by a weight associated with
  // each angular point explicitly.
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   AngularBin& theta);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_min, double theta_max);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_max);
  void FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta);
  void FindWeightedPairs(WAngularVector& w_ang,
			 AngularCorrelation& wtheta);

  // And for the cases where we want to access the Field values in the tree.
  double FindWeightedPairs(AngularCoordinate& ang, AngularBin& theta,
			   const std::string& field_name);
  double FindWeightedPairs(AngularCoordinate& ang,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(AngularCoordinate& ang, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(AngularVector& ang, AngularBin& theta,
			 const std::string& field_name);
  void FindWeightedPairs(AngularVector& ang, AngularCorrelation& wtheta,
			 const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang, AngularBin& theta,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang, AngularBin& theta,
			 const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang, AngularCorrelation& wtheta,
			 const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name, AngularBin& theta,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name,
			   double theta_min, double theta_max,
			   const std::string& field_name);
  double FindWeightedPairs(WeightedAngularCoordinate& w_ang,
			   const std::string& ang_field_name, double theta_max,
			   const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang,
			 const std::string& ang_field_name,
			 AngularBin& theta, const std::string& field_name);
  void FindWeightedPairs(WAngularVector& w_ang,
			 const std::string& ang_field_name,
			 AngularCorrelation& wtheta,
			 const std::string& field_name);

  // And for a selected set of the above variations, we also include forms
  // which allow the regions to come into play.  Generally speaking, these
  // are the versions that are most likely to be called from the correlation
  // codes.
  void FindPairsWithRegions(AngularVector& ang, AngularBin& theta);
  void FindPairsWithRegions(AngularVector& ang, AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(AngularVector& ang, AngularBin& theta);
  void FindWeightedPairsWithRegions(AngularVector& ang,
                                    AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang, AngularBin& theta);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang,
                                    AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(AngularVector& ang, AngularBin& theta,
                                    const std::string& field_name);
  void FindWeightedPairsWithRegions(AngularVector& ang,
                                    AngularCorrelation& wtheta,
                                    const std::string& field_name);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang, AngularBin& theta,
                                    const std::string& field_name);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang,
                                    AngularCorrelation& wtheta,
                                    const std::string& field_name);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang,
                                    const std::string& ang_field_name,
                                    AngularBin& theta,
                                    const std::string& field_name);
  void FindWeightedPairsWithRegions(WAngularVector& w_ang,
                                    const std::string& ang_field_name,
                                    AngularCorrelation& wtheta,
                                    const std::string& field_name);

  // In many cases, we want to find pairs within the TreeMap itself.  These
  // methods allow for that without having to incur the full memory hit of
  // having a full copy of the data in memory at any one time.
  void FindPairs(AngularBin& theta);
  void FindPairs(AngularCorrelation& wtheta);
  void FindWeightedPairs(AngularBin& theta);
  void FindWeightedPairs(AngularCorrelation& wtheta);
  void FindWeightedPairs(AngularBin& theta, const std::string& field_name);
  void FindWeightedPairs(AngularCorrelation& wtheta,
			 const std::string& field_name);
  void FindWeightedPairs(const std::string& ang_field_name, AngularBin& theta,
			 const std::string& field_name);
  void FindWeightedPairs(const std::string& ang_field_name,
			 AngularCorrelation& wtheta,
			 const std::string& field_name);
  void FindPairsWithRegions(AngularBin& theta);
  void FindPairsWithRegions(AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(AngularBin& theta);
  void FindWeightedPairsWithRegions(AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(AngularBin& theta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(AngularCorrelation& wtheta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(const std::string& ang_field_name,
				    AngularBin& theta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(const std::string& ang_field_name,
				    AngularCorrelation& wtheta,
				    const std::string& field_name);

  // The final set of iterations takes another TreeMap as an argument.  Like
  // the previous set, this allows for cross-correlation between data sets
  // while keeping the memory footprint smaller.
  void FindPairs(TreeMap& tree_map, AngularBin& theta);
  void FindPairs(TreeMap& tree_map, AngularCorrelation& wtheta);
  void FindWeightedPairs(TreeMap& tree_map, AngularBin& theta);
  void FindWeightedPairs(TreeMap& tree_map, AngularCorrelation& wtheta);
  void FindWeightedPairs(TreeMap& tree_map, AngularBin& theta,
			 const std::string& field_name);
  void FindWeightedPairs(TreeMap& tree_map, AngularCorrelation& wtheta,
			 const std::string& field_name);
  void FindWeightedPairs(TreeMap& tree_map, const std::string& ang_field_name,
			 AngularBin& theta, const std::string& field_name);
  void FindWeightedPairs(TreeMap& tree_map, const std::string& ang_field_name,
			 AngularCorrelation& wtheta,
			 const std::string& field_name);
  void FindPairsWithRegions(TreeMap& tree_map, AngularBin& theta);
  void FindPairsWithRegions(TreeMap& tree_map, AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(TreeMap& tree_map, AngularBin& theta);
  void FindWeightedPairsWithRegions(TreeMap& tree_map,
				    AngularCorrelation& wtheta);
  void FindWeightedPairsWithRegions(TreeMap& tree_map, AngularBin& theta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(TreeMap& tree_map,
				    AngularCorrelation& wtheta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(TreeMap& tree_map,
				    const std::string& ang_field_name,
				    AngularBin& theta,
				    const std::string& field_name);
  void FindWeightedPairsWithRegions(TreeMap& tree_map,
				    const std::string& ang_field_name,
				    AngularCorrelation& wtheta,
				    const std::string& field_name);

  // In addition to pair finding, we can also use the tree structure we've
  // built to do efficient nearest neighbor searches.  In the general case,
  // we'll be finding the k nearest neighbors of an input point.  The return
  // value is the number of nodes visited during assemblage.
  //
  // NOTE: There is no duplication checking.  Hence, if the input point is a
  // copy of a point in the tree, then that point will be included in the
  // returned vector of points.
  uint16_t FindKNearestNeighbors(AngularCoordinate& ang, uint8_t n_neighbors,
				 WAngularVector& neighbors_ang);

  // The special case where we're only interested in the nearest matching point.
  uint16_t FindNearestNeighbor(AngularCoordinate& ang,
			       WeightedAngularCoordinate& neighbor_ang);

  // In some cases, we're only interested in the distance to the kth nearest
  // neighbor.  The return value will be the angular distance in degrees.
  double KNearestNeighborDistance(AngularCoordinate& ang, uint8_t n_neighbors,
				  uint16_t& nodes_visited);

  // Or in the distance to the nearest neighbor.
  inline double NearestNeighborDistance(AngularCoordinate& ang,
					uint16_t& nodes_visited) {
    return KNearestNeighborDistance(ang, 1, nodes_visited);
  };

  // For the recursion necessary to do the neighbor finding, we use this
  // internal method.
  void _NeighborRecursion(AngularCoordinate& ang, TreeNeighbor& neighbor);


  // Add a given point on the sphere to the map.
  bool AddPoint(WeightedAngularCoordinate* ang);

  // The default method for adding WeightedAngularCoordinates to the map
  // takes a pointer to the object.  This means that the map now owns that
  // object and it shouldn't be deleted from the heap except by the map.
  // For cases where we want to retain a copy of the point outside of the
  // map, we provide a second method which takes a reference to the object
  // and creates and stores an internal copy.  The input object can thus be
  // modified or deleted without affecting the map.
  inline bool AddPoint(WeightedAngularCoordinate& w_ang) {
    WeightedAngularCoordinate* ang_copy =
      new WeightedAngularCoordinate(w_ang.UnitSphereX(), w_ang.UnitSphereY(),
				    w_ang.UnitSphereY(), w_ang.Weight());
    ang_copy->CopyFields(w_ang);
    return AddPoint(ang_copy);
  }

  // Complimentary method for specifying a weight separately when adding a
  // point to the pixel.
  inline bool AddPoint(AngularCoordinate& ang, double object_weight = 1.0) {
    WeightedAngularCoordinate* w_ang =
      new WeightedAngularCoordinate(ang.UnitSphereX(), ang.UnitSphereY(),
				    ang.UnitSphereZ(), object_weight);
    return AddPoint(w_ang);
  };

  // Equivalent methods as their namesakes in the BaseMap class.
  virtual void Coverage(PixelVector& superpix,
			uint16_t resolution = Stomp::HPixResolution);
  virtual bool Covering(Map& stomp_map, uint32_t maximum_pixels);
  virtual double FindUnmaskedFraction(Pixel& pix);
  virtual int8_t FindUnmaskedStatus(Pixel& pix);

  // And if we're not interested in the number of pixels, but want a Map
  // equivalent of the area covered by the nodes in the map.  If the map was
  // built with base level nodes at Stomp::HPixResolution resolution, the
  // results of Coverage and NodeMap will equivalent, albeit in different
  // functional forms.
  void NodeMap(Map& stomp_map);

  // Return the base level resolution for the tree map.
  inline uint16_t Resolution() {
    return resolution_;
  };
  inline uint16_t PixelCapacity() {
    return maximum_points_;
  };

  // Using either of these two methods will automatically remove any data
  // from the TreeMap.
  inline void SetResolution(uint16_t resolution) {
    Clear();
    resolution_ = resolution;
  };
  inline void SetPixelCapacity(int pixel_capacity) {
    Clear();
    maximum_points_ = pixel_capacity;
  };

  // Total number of points in the tree map or total number of points in
  // a given base level node.
  inline uint32_t NPoints(uint32_t k = Stomp::MaxPixnum) {
    return (k == Stomp::MaxPixnum ? point_count_ :
	    (tree_map_.find(k) != tree_map_.end() ?
	     tree_map_[k]->NPoints() : 0));
  };

  // A variation on the above method, returns the number of points associated
  // with the current map that are also contained in the input pixel.
  uint32_t NPoints(Pixel& pix);

  // If we want to extract a copy of all of the points that have been added
  // to this map, this method allows for that.
  void Points(WAngularVector& w_ang);

  // And an associated method that will extract a copy of the points associated
  // with an input pixel.
  void Points(WAngularVector& w_ang, Pixel& pix);

  // Total weight for all of the points in the tree map or total weight in
  // a given base level node.
  inline double Weight(uint32_t k = Stomp::MaxPixnum) {
    return (k == Stomp::MaxPixnum ? weight_ :
	    (tree_map_.find(k) != tree_map_.end() ?
	     tree_map_[k]->Weight() : 0.0));
  };

  // Likewise, we can provide a similar method for returning the weight
  // associated with an input pixel.
  double Weight(Pixel& pix);

  // And the equivalent functions for FieldTotals...
  inline double FieldTotal(const std::string& field_name,
			   uint32_t k = Stomp::MaxPixnum) {
    return (k == Stomp::MaxPixnum ?
	    (field_total_.find(field_name) != field_total_.end() ?
	     field_total_[field_name] : 0.0) :
	    (tree_map_.find(k) != tree_map_.end() ?
	     tree_map_[k]->FieldTotal(field_name) : 0.0));
  };
  double FieldTotal(const std::string& field_name, Pixel& pix);
  inline uint16_t NField() {
    return field_total_.size();
  };
  inline bool HasFields() {
    return (field_total_.size() > 0 ? true : false);
  };
  inline void FieldNames(std::vector<std::string>& field_names) {
    field_names.clear();
    for (FieldIterator iter=field_total_.begin();
	 iter!=field_total_.end();++iter) field_names.push_back(iter->first);
  };

  // Total number of base level nodes.
  inline uint16_t BaseNodes() {
    return tree_map_.size();
  };

  // Total number of all nodes.
  uint16_t Nodes();

  inline virtual uint32_t Size() {
    return point_count_;
  };
  virtual double Area();
  void CalculateArea();

  // We need these methods to comply with the BaseMap signature.
  inline virtual uint16_t MinResolution() {
    return resolution_;
  };
  inline virtual uint16_t MaxResolution() {
    return resolution_;
  };

  inline virtual bool Empty() {
    return (tree_map_.empty() ? true : false);
  };
  inline virtual void Clear() {
    if (!tree_map_.empty()) {
      for (TreeDictIterator iter=tree_map_.begin();
	   iter!=tree_map_.end();++iter) {
	iter->second->Clear();
	delete iter->second;
      }
      tree_map_.clear();
      field_total_.clear();
      weight_ = 0.0;
      area_ = 0.0;
      point_count_ = 0;
      modified_ = false;
    }
    ClearRegions();
  };

 private:
  TreeDict tree_map_;
  FieldDict field_total_;
  uint16_t maximum_points_, resolution_, nodes_;
  uint32_t point_count_;
  double weight_, area_;
  bool modified_;
};


class FootprintBound {
  // This is the base class for generating footprints.  A footprint object is
  // essentially a scaffolding around the Map class that contains the
  // methods necessary for converting some analytic expression of a region on
  // a sphere into the equivalent Map.  All footprints do roughly the same
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
  //
  // Alternatively, just call Pixelize() and it will call the other two
  // routines as necessary.
  uint8_t FindStartingResolutionLevel();
  bool FindXYBounds(const uint8_t resolution_level);
  bool Pixelize();

  // Part of the pixelization process is figuring out what fraction of a
  // given pixel is within the bounds delineated by the footprint's geometry.
  // Pixels are scored on a scale from -1 <= score <= 0, with -1 indicating
  // that the pixel is completely inside of the bounds and 0 indicating that
  // it's completely outside.  This allows one to sort pixels by their score
  // and keep the ones that are closest to being completely inside the
  // footprint bounds.
  double ScorePixel(Pixel& pix);

  // Once we've pixelized the footprint, we want to return a Map
  // representing the results.  This method returns a pointer to that map.
  inline Map::Map* ExportMap() {
    return new Map::Map(pix_);
  }
  inline void ExportMap(Map& stomp_map) {
    stomp_map.Clear();
    stomp_map.Initialize(pix_);
  }
  inline void SetMaxResolution(uint16_t resolution =
			       Stomp::MaxPixelResolution) {
    max_resolution_level_ = Stomp::MostSignificantBit(resolution);
  }

  // Since we store the area and pixelized area in this class, we need methods
  // for setting and getting those values.  Likewise with the weight that will
  // be assigned to the Pixels that will make up the Map that results.
  inline void SetArea(double input_area) {
    area_ = input_area;
  };
  inline double Area() {
    return area_;
  };
  inline void AddToPixelizedArea(uint16_t resolution) {
    pixel_area_ += Pixel::PixelArea(resolution);
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
  inline uint32_t NPixel() {
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
  inline uint32_t XMin() {
    return x_min_;
  };
  inline uint32_t XMax() {
    return x_max_;
  };
  inline uint32_t YMin() {
    return y_min_;
  };
  inline uint32_t YMax() {
    return y_max_;
  };
  inline PixelIterator Begin() {
    return pix_.begin();
  };
  inline PixelIterator End() {
    return pix_.end();
  };
  inline void Clear() {
    pix_.clear();
  };


 private:
  PixelVector pix_;
  bool found_starting_resolution_, found_xy_bounds_;
  uint8_t max_resolution_level_;
  double area_, pixel_area_, lammin_, lammax_, etamin_, etamax_, weight_;
  uint32_t x_min_, x_max_, y_min_, y_max_;
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


class WedgeBound : public FootprintBound {
  // A variant of the CircleBound class.  Instead of pixelizing the entire
  // circle, we only pixelize a wedge from the circle.  The position angle
  // values and radius should be specified in degrees.
 public:
  WedgeBound(const AngularCoordinate& ang, double radius,
	     double position_angle_min, double position_angle_max,
	     double weight, AngularCoordinate::Sphere sphere =
	     AngularCoordinate::Survey);
  virtual ~WedgeBound();
  virtual bool CheckPoint(AngularCoordinate& ang);
  virtual bool FindAngularBounds();
  virtual bool FindArea();

 private:
  AngularCoordinate ang_;
  double radius_, sin2radius_, position_angle_min_, position_angle_max_;
  AngularCoordinate::Sphere sphere_;
};

typedef std::vector<WedgeBound> WedgeVector;
typedef WedgeVector::iterator WedgeIterator;


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
  uint32_t n_vert_;
};

typedef std::vector<PolygonBound> PolygonVector;
typedef PolygonVector::iterator PolygonIterator;

} // end namespace Stomp

#endif
