#include "tile_mill.h"

#include <QDir>
#include <guitillmill.h>

#include <cmath>
#include <dmgroup.h>
#include <dmsystem.h>
#include <mapnikrenderer.h>

#include "ogrsf_frmts.h"
#include "gdal_priv.h"

DM_DECLARE_NODE_NAME( TileMill,Viewer )

TileMill::TileMill()
{
	global_counter = -1;
	this->maxZoomLevel = 1;
	this->EPSGCode = 0;

	this->addParameter("ExportMaps", DM::STRING_MAP, &exportMaps);
	this->addParameter("Styles", DM::STRING_MAP, &styles);
	std::vector<DM::View> datastream;
	datastream.push_back(DM::View("dummy", DM::SUBSYSTEM, DM::READ));
	this->addParameter("ZoomLevels", DM::INT, &maxZoomLevel);
	this->addParameter("EPSGCode", DM::INT, &EPSGCode);
	this->addData("data",datastream);
	this->folderName = "/tmp";
	this->addParameter("fileName", DM::STRING, &this->folderName);

	this->currrentZoomLevel = 12;
	this->addParameter("currrentZoomLevel", DM::INT, &this->currrentZoomLevel);
	this->centre_x = 47.26;
	this->addParameter("centre_x", DM::DOUBLE, &this->centre_x);
	this->centre_y = 11.4;
	this->addParameter("centre_y", DM::DOUBLE, &this->centre_y);


}

void TileMill::init()
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


void TileMill::run() {

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
	double x1_extend = -20037508.342789244;
	double y1_extend = -20037508.342789244;
	double x2_extend = 20037508.342789244;
	double y2_extend = 20037508.342789244;

	double total_length_x = x2_extend - x1_extend;
	double total_length_y = y2_extend - y1_extend;

	DM::System * sys = this->getData("data");
	Logger(Debug) << "Init TileMill";
    MapnikRenderer tilemill = MapnikRenderer(sys, this->EPSGCode, 900913);
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

	QString rootdir(QString::fromStdString(this->folderName));

	if (!QDir(rootdir).exists()){
		DM::Logger(DM::Standard) <<  this->folderName << "  does not exist but created";
		QDir::current().mkpath(rootdir);
	}

	QDir root = QDir(rootdir);

	//Add current state to root dir
	rootdir =rootdir + "/"+QString::number(global_counter);
	root.mkdir(rootdir);

	Logger(Debug) << tilemill.getMapExtend()[0] << "/" << tilemill.getMapExtend()[1];
	Logger(Debug) << tilemill.getMapExtend()[2] << "/" << tilemill.getMapExtend()[3];

	int minzoomlevel = log2( total_length_x / (tilemill.getMapExtend()[2] - tilemill.getMapExtend()[0]) );
	Logger(Debug) << minzoomlevel;
	this->currrentZoomLevel = minzoomlevel;

	//
    this->initTransForm(900913,4326);

	//Calcuate Extend
	double x1 = tilemill.getMapExtend()[0];
	double y1 = tilemill.getMapExtend()[1];
	double x2 = tilemill.getMapExtend()[2];
	double y2 = tilemill.getMapExtend()[3];

	this->transform(&x1, &y1);
	this->transform(&x2, &y2);

	Logger(Debug) << x1 << "/" << x2;
	Logger(Debug) << y1 << "/" << y2;

	//Set Centre
	this->centre_x = x1 + (x2-x1) / 2.;
	this->centre_y = y1 + (y2-y1) / 2.;

	for (int zoomlevel = minzoomlevel; zoomlevel < minzoomlevel + this->maxZoomLevel ; zoomlevel++) {
        int elements = pow(2.0f,zoomlevel);
		Logger(Debug) << elements;
		QString z_dir = rootdir + "/"+ QString::number(zoomlevel);
		root.mkdir(z_dir);
		int total_x =  (tilemill.getMapExtend()[2]  - tilemill.getMapExtend()[0])/total_length_x * elements + 2;
		int total_y =  (tilemill.getMapExtend()[3]  - tilemill.getMapExtend()[1])/total_length_y * elements + 2;
		Logger(Debug) << total_x <<"/" << total_y;
		int offest_x = ( tilemill.getMapExtend()[0] -  x1_extend ) / total_length_x  * elements;
		int offest_y = ( -tilemill.getMapExtend()[1] - y1_extend ) / total_length_y  * elements;
		for ( int dx = 0; dx < (total_x); dx++ ) {
			int x = dx + offest_x;
			QString x_dir =z_dir + "/"+ QString::number(x);
			root.mkdir(x_dir);
			int k = offest_y;
			for ( int dy = 0; dy < total_y; dy++, k++ ) {
				int y = offest_y - dy;
				tilemill.setZoomLevel(1./float(elements));
				QString filename = x_dir + "/" + QString::number(y) + ".png";
				tilemill.renderGrid(x,y,filename);
			}
		}
	}
}


bool TileMill::createInputDialog() {
	//QWidget * w = new GUITillMill(this);
	//w->show();
	return false;
}

void TileMill::initTransForm(int EPSG, int EPGSTo)
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

bool TileMill::transform(double *x, double *y)
{
	if(!transformok)
		return false;

	if( poCT == NULL || !poCT->Transform( 1, x, y ) )
		return false;

	return true;
}


