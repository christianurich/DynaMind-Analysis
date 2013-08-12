#ifndef DMSYSTEMGEOJSON_H
#define DMSYSTEMGEOJSON_H

#include "ogrsf_frmts.h"
#include "gdal_priv.h"

#include <string>


namespace DM {
	class System;
	class View;
	class Face;
}
class DMSystemGeoJSON
{
private:
	int epsgcode;
	bool transformok;
	OGRCoordinateTransformation *poCT;
	bool transform(double *x, double *y);

public:
	DMSystemGeoJSON(int EPSGCode);
	 std::string ViewToGeoJSON(DM::System * sys, const DM::View & view);
	 std::string ConvertFace(DM::Face * f);



};

#endif // DMSYSTEMGEOJSON_H
