#include "exportgeojson.h"
#include "dmsystemgeojson.h"

#include <dmsystem.h>
#include <fstream>

DM_DECLARE_NODE_NAME( ExportGeoJSON,Viewer )

ExportGeoJSON::ExportGeoJSON()
{
	this->ViewName = "";
	this->addParameter("ViewName", DM::STRING, &this->ViewName);

	this->EPSGCode = 31254;
	this->addParameter("EPSGCode", DM::INT, &this->EPSGCode);

	this->FileName = "";
	this->addParameter("FileName", DM::STRING, &this->FileName);
	std::vector<DM::View> datastream;
	datastream.push_back(DM::View("dummy", DM::SUBSYSTEM, DM::READ));
	this->addData("data",datastream);
}

void ExportGeoJSON::run()
{

	DMSystemGeoJSON geoJSON(this->EPSGCode);
	DM::System * sys = this->getData("data");
	this->vName = this->getViewInStream("data",this->ViewName);

	//Logger(Standard)
	std::fstream txtout;
	txtout.open(FileName.c_str(),std::ios::out);
	txtout << "var test_json = ";
	txtout << geoJSON.ViewToGeoJSON(sys, vName);
	txtout.close();

}
