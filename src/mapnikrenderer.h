#ifndef MAPNIKRENDERER_H
#define MAPNIKRENDERER_H

#include <QString>
#include <QPixmap>
#include <QMap>
#include <QPoint>

#include <mapnik/box2d.hpp>

struct style_struct {
	QString layer;
	QString symbolizer;
	QString name;
	QString buildingHeight;
	QString filter;
	QColor color;
	double linewidth;
	double opacity;

};

namespace DM {
	class System;
}

namespace mapnik {
	class Map;
	class layer;
}

struct mapnik_private;

class MapnikRenderer
{

public:
	MapnikRenderer(DM::System * sys=0, int EPSG = 0, int EPSGTo = 0);
	QString getFilterForStyle(QString style);
	~MapnikRenderer();

	void addDefaultLayer(mapnik::layer &lyr, QString dm_layer);
	void init_mapnik();
	void setPan(int x, int y);
	void setZoomLevel(double level);
	void drawMap();
	void setSystem(DM::System * sys);
	void addLayer(QString dm_layer, bool withdefault = true);
	void editStyleDefintionGUI(QString layer);
	void removeStyleDefinition(QString layer_name, QString stylename);
	void addNewStyle(style_struct ss);
	void saveToPicture(unsigned width, unsigned height, QString filename);
	void increaseZoomLevel(double factor);
	void decreaseZoomLevel(double factor);
	void changeSystem(DM::System * sys);
	std::string save_style_to_file();
	void renderGrid(unsigned dx, unsigned dy, QString filename);
	void loadStyle(QString filename);
	int minZoomLevel();

	mapnik::box2d<double> getMapExtend( );


private:
	double height;
	double width;
	QPixmap pix_;
	mapnik::Map * map_;
	DM::System * sys_;

	//Workaround for Qt moc and boost error
	mapnik_private * d;
	double zoom_level;
	QPoint pan;

	QPoint startPos;


	//Rules are evaluated when created, therefore can't the original rule string gets lost -> bad when saving so stoared in map
	QMap<QString, style_struct> styles_structs;

	/** return index of layer, if not found return value is -1*/
	int getLayerIndex(std::string layer_name);

	int EPSG;
	int EPSGTo;
};




#endif // MAPNIKRENDERER_H
