#include "tile_mill.h"

#include <QDir>
#include <guitillmill.h>

#include <cmath>

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
	Logger(Debug) << "Run TileMill";
	//EPSG 900913
	double x1_extend = -20037508.342789244;
	double y1_extend = -20037508.342789244;
	double x2_extend = 20037508.342789244;
	double y2_extend = 20037508.342789244;

	double total_length_x = x2_extend - x1_extend;
	double total_length_y = y2_extend - y1_extend;

	global_counter++;
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

	Logger(Standard) << tilemill.getMapExtend()[0] << "/" << tilemill.getMapExtend()[1];
	Logger(Standard) << tilemill.getMapExtend()[2] << "/" << tilemill.getMapExtend()[3];

	int minzoomlevel = log2( total_length_x / (tilemill.getMapExtend()[2] - tilemill.getMapExtend()[0]) );
	Logger(Standard) << minzoomlevel;

	//for WGS84
	for (int zoomlevel = minzoomlevel; zoomlevel < minzoomlevel + this->maxZoomLevel ; zoomlevel++) {
		int elements = pow(2,zoomlevel);
		Logger(Standard) << elements;
		QString z_dir = rootdir + "/"+ QString::number(zoomlevel);
		root.mkdir(z_dir);
		int total_x =  (tilemill.getMapExtend()[2]  - tilemill.getMapExtend()[0])/total_length_x * elements + 2;
		int total_y =  (tilemill.getMapExtend()[3]  - tilemill.getMapExtend()[1])/total_length_y * elements + 2;
		Logger(Standard) << total_x <<"/" << total_y;
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


