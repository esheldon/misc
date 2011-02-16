#include "stomp_util.h"

class DESFootprint : public FootprintBound {
  // This is a derived class for pixelizing the footprint associated with the
  // Dark Energy Survey camera's footprint.  The camera has a non-convex
  // shape, so we can't use a simple PolygonBound footprint.  Instead, we'll
  // break the camera up into 9 abutting convex polygons and pixelize across
  // them.  Then our virtual methods will iterate over the polygons to do things
  // like point checking and the like.

 public:
  DESFootprint(AngularCoordinate& ang, double weight);
  virtual ~DESFootprint();
  virtual bool CheckPoint(AngularCoordinate& ang);
  virtual bool FindAngularBounds();
  virtual bool FindArea();

 private:
  AngularCoordinate center_ang_;
  PolygonVector poly_;
};

typedef std::vector<DESFootprint> DESVector;
typedef DESVector::iterator DESIterator;



