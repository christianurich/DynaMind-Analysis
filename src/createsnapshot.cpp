#include "createsnapshot.h"

#include <QDir>
#include <guitillmill.h>

#include <cmath>
#include <dmgroup.h>
#include <dmsystem.h>
#include <mapnikrenderer.h>

#include "ogrsf_frmts.h"
#include "gdal_priv.h"

DM_DECLARE_NODE_NAME( CreateSnapshot,Viewer )

CreateSnapshot::CreateSnapshot()
{
	global_counter = -1;
	this->EPSGCode = 0;

	this->addParameter("ExportMaps", DM::STRING_MAP, &exportMaps);
	this->addParameter("Styles", DM::STRING_MAP, &styles);
	std::vector<DM::View> datastream;
	datastream.push_back(DM::View("dummy", DM::SUBSYSTEM, DM::READ));
	this->addParameter("EPSGCode", DM::INT, &EPSGCode);
	this->addData("data",datastream);
	this->filename = "test";
	this->addParameter("fileName", DM::STRING, &this->fileName);

	this->height = 400;
	this->addParameter("height", DM::INT, &this->height);

	this->width = 400;
	this->addParameter("width", DM::INT, &this->width);


}

void CreateSnapshot::init()
{
	if (exportMaps.size() == 0) return;

	for (std::map<std::string, std::string>::const_iterator it = exportMaps.begin();
		 it != exportMaps.end();
		 ++it) {
		std::string name = it->second;
		DM::View v = this->getViewInStream("data", it->second);
		if (v.getType() == -1) {
			Logger(Error) << "View " << name << " does not exist";
			return;
		}
	}

	std::vector<DM::View> datastream;
	for (std::map<std::string, std::string>::const_iterator it = exportMaps.begin();
		 it != exportMaps.end();
		 ++it) {
		std::string name = it->second;
		DM::View v = this->getViewInStream("data", name);
		vData = DM::View(name, v.getType(), DM::READ);
	}
	datastream.push_back(vData);
	this->addData("data", datastream);
}


void CreateSnapshot::run() {

	Group* lg = dynamic_cast<Group*>(getOwner());
	if(lg) {
		global_counter = lg->getGroupCounter();
	}
	else
	{
		global_counter = 0;
	}
	Logger(Debug) << "Run TileMill";
	//EPSG 900913
	DM::System * sys = this->getData("data");
	Logger(Debug) << "Init TileMill";
    MapnikRenderer tilemill = MapnikRenderer(sys, this->EPSGCode, 3856);
	Logger(Debug) << "Init TileMill Successful";
	for (std::map<std::string, std::string>::const_iterator it = exportMaps.begin();
		 it != exportMaps.end();
		 ++it) {
		std::string name = it->second;
		if (styles.find(it->first) == styles.end()) {
			tilemill.addLayer(QString::fromStdString(name));
			continue;
		}

		QString filename = QString::fromStdString(styles[it->first]);
		tilemill.loadStyle(filename);
	}

	QString filename(QString::fromStdString(this->fileName));

	filename =filename +".png";

    this->initTransForm(3856,4326);
	Logger(DM::Debug) << "Export to" << filename.toStdString();
	tilemill.saveToPicture(this->width,this->height,filename);
}


bool CreateSnapshot::createInputDialog() {
	return false;
}

void CreateSnapshot::initTransForm(int EPSG, int EPGSTo)
{
	transformok = true;
	if (EPSG == 0 || EPGSTo == 0) {
		transformok = false;
	}
	if (EPSG == EPGSTo) {
		transformok = false;
	}

	OGRSpatialReference *oSourceSRS, *oTargetSRS;
	oSourceSRS = new OGRSpatialReference();
	oSourceSRS->importFromEPSG(EPSG);
	oTargetSRS = new OGRSpatialReference();
	oTargetSRS->importFromEPSG(EPGSTo);
	poCT = OGRCreateCoordinateTransformation( oSourceSRS, oTargetSRS );

	if(poCT == NULL)
	{
		transformok = false;
		DM::Logger(DM::Error) << "Unknown transformation to EPSG:" << EPGSTo;
	}
}

bool CreateSnapshot::transform(double *x, double *y)
{
	if(!transformok)
		return false;

	if( poCT == NULL || !poCT->Transform( 1, x, y ) )
		return false;

	return true;
}
