#include "tile_mill.h"

#include <QDir>

#include <cmath>

#include <dmsystem.h>
#include <mapnikrenderer.h>

DM_DECLARE_NODE_NAME( TileMill,Viewer )

TileMill::TileMill()
{

	this->viewName = "";
	this->maxZoomLevel = 1;

	this->addParameter("ViewName", DM::STRING, &viewName);
	std::vector<DM::View> datastream;
	datastream.push_back(DM::View("dummy", DM::SUBSYSTEM, DM::READ));

	this->addParameter("ZoomLevels", DM::INT, &maxZoomLevel);

	this->addData("data",datastream);

}

void TileMill::init()
{
	if (viewName.empty()) return;

	DM::View v = this->getViewInStream("data", viewName);
	if (v.getType() == -1) {
		Logger(Error) << "View does not exist";
		return;
	}

	std::vector<DM::View> datastream;
	vData = DM::View(viewName, v.getType(), v.getType());
	datastream.push_back(vData);

	this->addData("data", datastream);
}

void TileMill::run() {
	DM::System * sys = this->getData("data");
	MapnikRenderer tilemill = MapnikRenderer(sys);
	tilemill.addLayer(QString::fromStdString(viewName));
	QString rootdir("/tmp/test");
	QDir root = QDir(rootdir);
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


