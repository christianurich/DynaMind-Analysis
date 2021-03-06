#include "mapnikrenderer.h"
//STD
#include <stdio.h>
#include <math.h>

//QT
#include <QPainter>
#include <QColor>
#include <QRgb>


//DM
#include <dmlogger.h>
#include <systemmapnikwrapper.h>
#include <dmrasterdata.h>

//Mapnik
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/color.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/raster_colorizer.hpp>


#include <mapnikstylereader.h>

using namespace mapnik;

struct mapnik_private {
	QMap<QString,  boost::shared_ptr<SystemMapnikWrapper> > datasources_;
};

MapnikRenderer::MapnikRenderer(DM::System * sys, int EPSG, int EPSGTo) :
	sys_(sys),
	d(new mapnik_private),
	zoom_level(1.),
	pan(0,0),
	startPos(0,0),
	height(256),
	width(256),
	EPSG(EPSG),
	EPSGTo(EPSGTo)
{

	std::string mapnik_dir("/usr/local/Cellar/mapnik/2.2.0/lib");
	DM::Logger(DM::Debug) << " looking for 'shape.input' plugin in... " << mapnik_dir << "/mapnik/input/";
	//datasource_cache::instance().register_datasource(mapnik_dir + "/mapnik/input/");
	DM::Logger(DM::Debug) << " looking for DejaVuSans font in... " << mapnik_dir << "/mapnik/fonts/DejaVuSans.ttf";
	freetype_engine::register_font(mapnik_dir + "/mapnik/fonts/DejaVuSans.ttf");

	//	pan = QPoint(this->height()/2, this->width()/2);
	map_ = 0;
	this->init_mapnik();
}

QString MapnikRenderer::getFilterForStyle(QString style)
{
	return this->styles_structs[style].filter;
}

MapnikRenderer::~MapnikRenderer()
{
	delete map_;
	delete d;
}

void MapnikRenderer::init_mapnik() {
	if (!sys_)
		return;

	if (map_)
		delete map_;

	map_ = new Map(this->width,this->height);


	DM::Logger(DM::Debug) << this->width;
	map_->set_background(color(0,0,0,0));

}

void MapnikRenderer::setPan(int x, int y)
{
	this->pan = QPoint(x,y);
}

void MapnikRenderer::setZoomLevel(double level)
{
	this->zoom_level = level;
}

void MapnikRenderer::drawMap()
{
	if (!sys_)
		return;
	try {
		map_->set_height(this->height);
		map_->set_width(this->width);
		map_->zoom_all();
		map_->pan(pan.x(), pan.y());
		map_->zoom(this->zoom_level);
		image_32 buf(map_->width(),map_->height());
		agg_renderer<image_32> ren(*map_,buf);
		ren.apply();

		QImage image((uchar*)buf.raw_data(),map_->width(),map_->height(),QImage::Format_ARGB32);
		//pix_ = QPixmap::fromImage(image.rgbSwapped());
	}
	catch ( const mapnik::config_error & ex )
	{
		DM::Logger(DM::Error) << "### Configuration error: " << ex.what();
	}
	catch ( const std::exception & ex )
	{
		DM::Logger(DM::Error) <<  "### std::exception: " << ex.what();
	}
	catch ( ... )
	{
		DM::Logger(DM::Error) <<  "### Unknown exception.";
	}
}

void MapnikRenderer::setSystem(DM::System * sys)
{
	this->sys_ = sys;
	this->init_mapnik();
	this->drawMap();
}

void MapnikRenderer::addDefaultLayer(layer & lyr, QString dm_layer)
{
	DM::View * view = this->sys_->getViewDefinition(dm_layer.toStdString());
	if (view->getType()  == DM::RASTERDATA) {
		float val_max;
		float val_min;
		std::map<std::string, DM::Component*> components = this->sys_->getAllComponentsInView(*view);
		mforeach(DM::Component* c, components) {
			if(c->getType() == DM::RASTERDATA) {
				DM::RasterData * rdata = (DM::RasterData*)c;
				val_min = rdata->getMinValue();
				val_max = rdata->getMaxValue();
			}
		}

		//Default symbolizer Edge
		feature_type_style raster_style;
		rule raster_style_on("");

		raster_symbolizer rs;
		raster_colorizer_ptr rc =   boost::make_shared<raster_colorizer>(COLORIZER_LINEAR);
		//rc->set_default_color(color(0,0,0));
		colorizer_stop cs1(val_min,COLORIZER_LINEAR,color(0,0,255) );
		colorizer_stop cs2((val_max - val_min)/2. + val_min,COLORIZER_LINEAR,color(0,255,0) );
		colorizer_stop cs3(val_max,COLORIZER_LINEAR,color(255,0,0) );
		rc->add_stop(cs1);
		rc->add_stop(cs2);
		rc->add_stop(cs3);
		rs.set_colorizer(rc);

		raster_style_on.append(rs);
		raster_style.add_rule(raster_style_on);
		map_->insert_style("default_raster",raster_style);
		lyr.add_style("default_raster");

		DM::Logger(DM::Debug) <<  val_min << "\t"<< rc->get_color(val_min).red() << "\t" << rc->get_color(val_min).green() << "\t" << rc->get_color(val_min).blue();
		DM::Logger(DM::Debug) <<  val_max << "\t"<< rc->get_color(val_max).red() << "\t" << rc->get_color(val_max).green() << "\t" << rc->get_color(val_max).blue();


		return;
	}

	if (this->sys_->getViewDefinition(dm_layer.toStdString())->getType()  == DM::NODE) {

		//Default symbolizer Edge
		feature_type_style node_style;
		rule node_style_on("");

		markers_symbolizer ms;
		ms.set_fill(color(0, 0, 0));
		ms.set_width(parse_expression("4"));
		ms.set_height(parse_expression("4"));

		node_style_on.append(ms);
		node_style.add_rule(node_style_on);
		map_->insert_style("default_node",node_style);
		lyr.add_style("default_node");
		return;
	}

	//Default symbolizer Edge
	feature_type_style edges_style;
	rule edge_rule_on("");
	edge_rule_on.append(stroke(color(0, 0, 0), 1));
	edges_style.add_rule(edge_rule_on);
	map_->insert_style("default_edge",edges_style);
	lyr.add_style("default_edge");

	if (this->sys_->getViewDefinition(dm_layer.toStdString())->getType()  == DM::EDGE) return;

	//Default symbolizer Polygon
	feature_type_style polygon_style;
	rule polygon_rule_on("");
	polygon_rule_on.append(polygon_symbolizer(color(211, 211, 211)));
	polygon_style.add_rule(polygon_rule_on);
	map_->insert_style("default_face",polygon_style);
	lyr.add_style("default_face");


}

void MapnikRenderer::addLayer(QString dm_layer, bool withdefault)
{
	try {
		parameters p;
		p["type"]="dm";
		p["view_name"]= dm_layer.toStdString();
		stringstream s_view_type;
		s_view_type << this->sys_->getViewDefinition(dm_layer.toStdString())->getType();
		p["view_type"]= s_view_type.str();
		boost::shared_ptr<SystemMapnikWrapper> ds(new SystemMapnikWrapper(p, true, sys_, this->EPSG, this->EPSGTo));


		d->datasources_[dm_layer] = ds;

		//Add default styles
		layer lyr(dm_layer.toStdString());
		lyr.set_datasource(ds);

		if (withdefault) addDefaultLayer(lyr, dm_layer);

		map_->addLayer(lyr);
		this->pan = QPoint(128,128);
		this->drawMap();
	}
	catch ( const mapnik::config_error & ex )
	{
		DM::Logger(DM::Error) << "### Configuration error: " << ex.what();
	}
	catch ( const std::exception & ex )
	{
		DM::Logger(DM::Error) <<  "### std::exception: " << ex.what();
	}
	catch ( ... )
	{
		DM::Logger(DM::Error) <<  "### Unknown exception.";
	}

	this->drawMap();
}

void MapnikRenderer::editStyleDefintionGUI(QString layer_name)
{
	std::vector<layer> layers = map_->layers();

	QStringList styleNames;
	foreach (layer l, layers) {
		if (l.name() == layer_name.toStdString()) {
			std::vector<std::string> styles = l.styles();
			foreach (std::string s, styles)
				styleNames << QString::fromStdString(s);
			break;
		}
	}
	//Get View embended in the system
	if (!sys_->getViewDefinition(layer_name.toStdString())) return;
	DM::View *vptr = sys_->getViewDefinition(layer_name.toStdString());
	//DM::Component * cmp = sys_->getViewDefinition(layer_name.toStdString())->getDummyComponent();
	//std::map<std::string, DM::Attribute*> attrs_map =  cmp->getAllAttributes();

	QStringList attribute_list;
	foreach ( std::string s, vptr->getAllAttributes()) {
		attribute_list.append(QString::fromStdString(s));
	}

	//GUIStyleDefinition * gsd = new GUIStyleDefinition(layer_name, styleNames, attribute_list, this);
	//connect (gsd, SIGNAL(removeStyle(QString,QString)), this, SLOT(removeStyleDefinition(QString,QString)));
	//connect (gsd, SIGNAL(newStyle(style_struct)), this, SLOT(addNewStyle(style_struct)));
	//gsd->show();
}

void MapnikRenderer::removeStyleDefinition(QString layer_name, QString stylename)
{
	int index_layer = this->getLayerIndex(layer_name.toStdString());

	layer currentL = map_->getLayer(index_layer);

	layer new_layer(layer_name.toStdString());
	foreach (std::string cs, currentL.styles()) {
		if (cs == stylename.toStdString()) continue;
		new_layer.add_style(cs);
	}

	new_layer.set_datasource(currentL.datasource());
	map_->removeLayer(index_layer);
	map_->addLayer(new_layer);

	DM::Logger(DM::Debug) << "Remove Layer";

	this->drawMap();

	//	emit removedStyle(layer_name, stylename);
}

void MapnikRenderer::addNewStyle(style_struct ss)
{
	try {
		feature_type_style new_style;
		rule new_rule_on("");
		if (ss.symbolizer == "MarkerSymbolizer") {
			markers_symbolizer ms;
			ms.set_fill(color(ss.color.red(), ss.color.green(), ss.color.blue()));

			ms.set_width(parse_expression(QString::number(ss.linewidth).toStdString()));
			ms.set_height(parse_expression(QString::number(ss.linewidth).toStdString()));
			new_rule_on.append(ms);
		}
		if (ss.symbolizer == "PolygonSymbolizer") {
			new_rule_on.append(polygon_symbolizer(color(ss.color.red(), ss.color.green(), ss.color.blue())));
		}
		if (ss.symbolizer == "LineSymbolizer") {
			new_rule_on.append(stroke(color(ss.color.red(), ss.color.green(), ss.color.blue()), ss.linewidth));
		}
		if (ss.symbolizer == "BuildingSymbolizer"){
			new_rule_on.append(building_symbolizer(color(ss.color.red(), ss.color.green(), ss.color.blue()),parse_expression(ss.buildingHeight.toStdString())));
		}
		if (!ss.filter.isEmpty()) new_rule_on.set_filter(parse_expression(ss.filter.toStdString()));

		new_style.add_rule(new_rule_on);
		//new_style.set_opacity(ss.opacity);

		bool added_style = map_->insert_style(ss.name.toStdString(),new_style);

		if (!added_style) std::cout << "Error" << std::endl;

		//update layer;

		int index_layer = this->getLayerIndex(ss.layer.toStdString());
		if (index_layer == -1){
			DM::Logger(DM::Error) << ss.layer.toStdString() << " doesn't exist";
			return;
		}
		layer l = map_->getLayer(index_layer);
		l.add_style(ss.name.toStdString());
		map_->removeLayer(index_layer);
		map_->addLayer(l);


		this->styles_structs[ss.name] = ss;
	}
	catch ( const mapnik::config_error & ex )
	{
		DM::Logger(DM::Error) << "### Configuration error: " << ex.what();
	}
	catch ( const std::exception & ex )
	{
		DM::Logger(DM::Error) <<  "### std::exception: " << ex.what();
	}
	catch ( ... )
	{
		DM::Logger(DM::Error) <<  "### Unknown exception.";
	}

	this->drawMap();

	//	emit new_style_added(ss.layer, ss.name);
}

void MapnikRenderer::saveToPicture(unsigned width, unsigned height, QString filename)
{
	if (!sys_)
		return;
	//map_->zoom_to_box();

	map_->set_width(width);
	map_->set_height(height);
	map_->zoom_all();
	map_->zoom(1.1);

	image_32 buf(map_->width(),map_->height());
	agg_renderer<image_32> ren(*map_,buf);
	ren.apply();

	save_to_file(buf,filename.toStdString(),"png");
}

int MapnikRenderer::minZoomLevel() {
	if (!sys_)
		return -1;
	map_->zoom_all();
	mapnik::box2d<double> window_extend = map_->get_current_extent();


	double length = window_extend[2] - window_extend[0];

	double length_map = 360;

	double difference =  length_map / length;

	return log2(difference);


}

mapnik::box2d<double> MapnikRenderer::getMapExtend()
{
	map_->zoom_all();
	return  map_->get_current_extent();
}



void MapnikRenderer::renderGrid(unsigned dx, unsigned dy, QString filename)
{

	double x1_extend = -20037508.342789244;
	double y1_extend = -20037508.342789244;
	double x2_extend = 20037508.342789244;
	double y2_extend = 20037508.342789244;

	double total_length_x = x2_extend - x1_extend;
	double total_length_y = y2_extend - y1_extend;

	if (!sys_)
		return;
	map_->zoom_all();


	map_->set_width(256);
	map_->set_height(256);
	mapnik::box2d<double> window;
	double x1 =  this->zoom_level * dx * total_length_x +  x1_extend;// +  window_extend[0];
	double y1 =  total_length_y - this->zoom_level * dy * total_length_x + y1_extend;// +  window_extend[1];
	double x2 = this->zoom_level * (dx + 1) * total_length_x +  x1_extend;// +  window_extend[0];
	double y2 =  total_length_y - this->zoom_level * (dy+1) * total_length_x + y1_extend ;// +  window_extend[1];

	window.init(x1,y1,x2,y2);
	DM::Logger(DM::Debug) << window[0] << "/" << window[1];
	DM::Logger(DM::Debug) << window[2] << "/" << window[3];
	map_->zoom_to_box(window);
	image_32 buf(map_->width(),map_->height());
	agg_renderer<image_32> ren(*map_,buf);
	ren.apply();

	save_to_file(buf,filename.toStdString(),"png");
}

void MapnikRenderer::loadStyle(QString filename)
{
	MapnikStyleReader(filename, this);
}

void MapnikRenderer::increaseZoomLevel(double factor)
{
	this->zoom_level /= (1.+(0.2*factor));
	this->drawMap();
	//update();
}

void MapnikRenderer::decreaseZoomLevel(double factor)
{
	this->zoom_level *= (1.+(0.2*factor));
	this->drawMap();
	//update();
}

void MapnikRenderer::changeSystem(DM::System *sys)
{
	int counter_l = map_->layer_count();

	std::vector<layer> new_layer;
	for (int i = 0; i < counter_l; i++) {
		parameters p;
		layer lyr(map_->getLayer(i));
		p["type"]="dm";
		p["view_name"] = lyr.name();
		//Bug fix int was causing a ugly carsh
		stringstream s_view_type;
		s_view_type << this->sys_->getViewDefinition(lyr.name())->getType();
		p["view_type"]= s_view_type.str();
		boost::shared_ptr<SystemMapnikWrapper> ds(new SystemMapnikWrapper(p, true, sys));

		lyr.set_datasource(ds);
		new_layer.push_back(lyr);
	}

	for (int i = counter_l; i > 0; i--)  map_->removeLayer(i-1);

	for (int i = 0; i < counter_l; i++) map_->addLayer(new_layer[i]);

	this->drawMap();
	//update();


}


std::string MapnikRenderer::save_style_to_file()
{

	std::stringstream out;
	//Write Map
	{
		boost::optional<color> bg = map_->background();

		out << "<Map ";
		if (!bg) {
			out << "background-color=\""
				<< "rgb("
				<< static_cast<unsigned int>((*bg).red()) << ","
				<< static_cast<unsigned int>((*bg).green()) << ","
				<< static_cast<unsigned int>((*bg).blue()) << ")\" ";
		}
		out << "srs=\""
			<< map_->srs()
			<< "\">\n";
	}

	//Write Styles
	typedef std::map<std::string,feature_type_style> style_map;
	for (style_map::const_iterator it = map_->styles().begin(); it != map_->styles().end(); it++) {
		feature_type_style sft = it->second;
		out << "\t";
		out  << "<Style name =\""
			 << it->first
			 << "\" "
			 << "opacity=\""
			 << sft.get_opacity() <<"\">"
			 << "\n";

		foreach (rule r, sft.get_rules()) {
			out << "\t\t";
			out  << "<Rule>\n";
			foreach (symbolizer s, r.get_symbolizers()) {
				out << "\t\t\t";
				if  ((s.type())  == typeid(markers_symbolizer))  {
					markers_symbolizer ms (boost::get<markers_symbolizer>(s));
					color c = ms.get_fill().get();
					out << "<MarkerSymbolizer fill=\"";
					out << "rgb("
						<< static_cast<unsigned int>(c.red()) << ","
						<< static_cast<unsigned int>(c.green()) << ","
						<< static_cast<unsigned int>(c.blue()) << ")\" ";
					out << "fill-width=\""
						<< this->styles_structs[QString::fromStdString(it->first)].linewidth << "\" ";
					out << "fill-height=\""
						<< this->styles_structs[QString::fromStdString(it->first)].linewidth;
					out << "\"/>\n";
				}
				if  ((s.type())  == typeid(line_symbolizer))  {
					line_symbolizer ls (boost::get<line_symbolizer>(s));
					color c = ls.get_stroke().get_color();
					out << "<LineSymbolizer stroke=\"";
					out << "rgb("
						<< static_cast<unsigned int>(c.red()) << ","
						<< static_cast<unsigned int>(c.green()) << ","
						<< static_cast<unsigned int>(c.blue()) << ")\" ";
					out << "stroke-width=\""
						<< ls.get_stroke().get_width();
					out << "\"/>\n";
				}
				if  ((s.type())  == typeid(polygon_symbolizer))  {
					polygon_symbolizer ps (boost::get<polygon_symbolizer>(s));
					color c = ps.get_fill();
					out << "<PolygonSymbolizer fill=\"";
					out << "rgb("
						<< static_cast<unsigned int>(c.red())  << ","
						<< static_cast<unsigned int>(c.green())  << ","
						<< static_cast<unsigned int>(c.blue())  << ")\""
						<< "/>\n";
				}
				if  ((s.type())  == typeid(building_symbolizer))  {
					building_symbolizer ps (boost::get<building_symbolizer>(s));
					color c = ps.get_fill();
					out << "<BuildingSymbolizer fill=\"";
					out << "rgb("
						<< c.red() << ","
						<< c.green() << ","
						<< c.blue() << ")\" ";
					out << "height=\""
						<< this->styles_structs[QString::fromStdString(it->first)].buildingHeight.toStdString()
						<< "\"/>\n";
				}
				out << "\t\t";
				out  << "</Rule>\n";
			}

			//Replace <> in filter
			QString filter = this->getFilterForStyle(QString::fromStdString(it->first));
			filter.replace(">", "&gt;");
			filter.replace("<", "&lt;");
			out << "<Filter value=\""
				<< filter.toStdString()
				<< "\"/>\n";

			out << "\t";
			out  << "</Style>\n";

		}

	}


	//Write Layers
	out << "\t";
	{
		foreach (layer l, map_->layers()) {
			out << "<Layer name=\""
				<< l.name() << "\""
				<< " srs=\""
				<< l.srs()
				<< "\">\n";
			foreach (std::string n, l.styles()) {
				out << "\t\t";
				out << "<Stylename";
				out << " name=\"style\">";
				out << n;
				out << "</Stylename>\n";
			}

			boost::shared_ptr<SystemMapnikWrapper> d = boost::static_pointer_cast<SystemMapnikWrapper>(l.datasource());
			out << "\t\t";
			out << "<Datasource>";
			out << "\n\t\t\t";
			out << "<Parameter name=\"type\">";
			out << d->getSourceType();
			out << "</Parameter>";
			out << "\n\t\t\t";
			out << "<Parameter name=\"view_name\">";
			out << d->getViewName();
			out << "</Parameter>";
			out << "\n\t\t\t";
			out << "<Parameter name=\"viewtype\">";
			out << d->getViewType();
			out << "</Parameter>\n";
			out << "\t\t";
			out << "</Datasource>\n";
		}
		out << "\t";
		out << "</Layer>\n";
	}

	out << "</Map>\n";

	return out.str();
}


int MapnikRenderer::getLayerIndex(string layer_name)
{
	std::vector<layer> layers = map_->layers();
	for (uint i = 0; i < layers.size(); i++) {
		if (layers[i].name() == layer_name) return i;
	}
	return -1;
}
