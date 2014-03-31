#include "dmsystemgeojson.h"
#include <dmsystem.h>
#include <dmface.h>
#include <dmnode.h>
#include <dmlogger.h>
#include <sstream>



DMSystemGeoJSON::DMSystemGeoJSON(int EPSGCode):
	epsgcode(EPSGCode),
	transformok(true)
{
	OGRSpatialReference *oSourceSRS, *oTargetSRS;
	oSourceSRS = new OGRSpatialReference();
	oSourceSRS->importFromEPSG(this->epsgcode);
	oTargetSRS = new OGRSpatialReference();
	oTargetSRS->importFromEPSG(4326);
	poCT = OGRCreateCoordinateTransformation( oSourceSRS, oTargetSRS );

	if(poCT == NULL)
	{
		transformok = false;
		DM::Logger(DM::Error) << "Unknown transformation to EPSG:" << this->epsgcode;
	}
}

std::string DMSystemGeoJSON::ViewToGeoJSON(DM::System *sys, const DM::View &view)
{


	std::stringstream featureCollection;

	featureCollection << "{\n";
	featureCollection << "\t\"type\": \"FeatureCollection\",\n";
	featureCollection << "\t\"features\": [ \n";


	bool first = true;
	int counter = 0;
	mforeach (DM::Component * cmp, sys->getAllComponentsInView(view)) {
		counter++;
		if (!first) { // Add , and new line
			featureCollection << "\t\t,\n";
		} else
			first = false;
		featureCollection << "\t\t\{\n";
		featureCollection << "\t\t\"type\": \"Feature\",\n";
		//Write Properties
		featureCollection << "\t\t \"properties\":";
		 std::map<std::string, DM::Attribute*> attr_map = cmp->getAllAttributes();
		 featureCollection << "\t\t{";
		 for (std::map<std::string, DM::Attribute*>::const_iterator it = attr_map.begin();
			  it != attr_map.end(); it++) {
				if (it != attr_map.begin()) {
					featureCollection<<  "\t\t,\n";
				}

				featureCollection << "\"" << 	it->first << "\"" << ":";
				DM::Attribute * attr = it->second;
				switch (attr->getType()) {
				case DM::Attribute::STRING:
					featureCollection << "\"" << attr->getString() << "\"";
					break;
				case DM::Attribute::DOUBLE:
					featureCollection << "\"" << attr->getDouble() << "\"";
					break;
				default:
					featureCollection << "\" not defined \"";
				}


		 }
		 featureCollection<<  "\t\t},\n";
		//Write Geometry
		switch (view.getType()) {
		case DM::FACE:
			 featureCollection << DMSystemGeoJSON::ConvertFace((DM::Face *) cmp);
			break;
		default:
			DM::Logger(DM::Warning) << "Type not supported";
			break;

		}
		featureCollection<< "\t\t,";
		featureCollection << "\t\t\"id\":" << counter << "\n";
		featureCollection << "\t\t}";

	}
	featureCollection << "\n";
	featureCollection << "\t\t]\n";
	featureCollection << "}";
	return featureCollection.str();
}

string DMSystemGeoJSON::ConvertFace(DM::Face *f)
{
	std::stringstream geomtry;
	geomtry.precision(12);

	geomtry << "\t\t\"geometry\": {\n";
	geomtry << "\t\t\t\"type\": \"MultiPolygon\",\n";
	geomtry << "\t\t\t\"coordinates\": [ \n";
	geomtry << "\t\t\t\t[ \n";
	geomtry << "\t\t\t\t\t [ \n";

	bool first = true;
	foreach (DM::Node * n, f->getNodePointers()) {
		if (!first) { // Add , and new line
			geomtry << ",\n";
		} else
			first = false;

		double x = n->getX();
		double y = n->getY();
		transform(&x,&y);
		geomtry << "\t\t\t\t\t\t[";
		geomtry << x << ", " << y;
		geomtry << "]";

	}

	geomtry << "\n\t\t\t\t\t ] \n";
	geomtry << "\t\t\t\t] \n";
	geomtry << "\t\t\t] \n";
	geomtry << "\t\t}\n";
	return geomtry.str();

}

bool DMSystemGeoJSON::transform(double *x, double *y)
{
	if(!transformok)
		return false;

	if( poCT == NULL || !poCT->Transform( 1, x, y ) )
		return false;

	return true;
}
