#include "tile_mill.h"

#include <QDir>
#include <guitillmill.h>

#include <cmath>

#include <dmsystem.h>
#include <mapnikrenderer.h>


DM_DECLARE_NODE_NAME( TileMill,Viewer )

TileMill::TileMill()
{
	global_counter = -1;
	this->maxZoomLevel = 1;
	this->addParameter("ExportMaps", DM::STRING_MAP, &exportMaps);
	this->addParameter("Styles", DM::STRING_MAP, &styles);
	std::vector<DM::View> datastream;
	datastream.push_back(DM::View("dummy", DM::SUBSYSTEM, DM::READ));
	this->addParameter("ZoomLevels", DM::INT, &maxZoomLevel);
	this->addData("data",datastream);
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
	global_counter++;

	DM::System * sys = this->getData("data");
	MapnikRenderer tilemill = MapnikRenderer(sys);
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
	QString rootdir("/tmp/test");
	QDir root = QDir(rootdir);
	//Add current state to root dir
	rootdir =rootdir + "/"+QString::number(global_counter);
	root.mkdir(rootdir);
	for (int zoomlevel = 0; zoomlevel <this->maxZoomLevel ; zoomlevel++) {
		QString z_dir = rootdir + "/"+ QString::number(zoomlevel);
		root.mkdir(z_dir);
		int number_of_elements = pow (2, zoomlevel);
		for ( int x = 0; x < (number_of_elements); x++ ) {
			QString x_dir =z_dir + "/"+ QString::number(x);
			root.mkdir(x_dir);
			int k = 0;
			for ( int y = number_of_elements-1; y >=0 ; y--, k++ ) {
				tilemill.setZoomLevel(1./float(number_of_elements));
				QString filename = x_dir + "/" + QString::number(k) + ".png";
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


