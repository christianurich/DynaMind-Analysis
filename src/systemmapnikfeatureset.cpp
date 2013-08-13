// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "systemmapnikfeatureset.h"

#include <dm.h>

void SystemMapnikFeatureset::initTransformation()
{
	if (EPSG == 0 || EPSGTo == 0) {
		transformok = false;
	}
	if (EPSG == EPSGTo) {
		transformok = false;
	}

	OGRSpatialReference *oSourceSRS, *oTargetSRS;
	oSourceSRS = new OGRSpatialReference();
	oSourceSRS->importFromEPSG(EPSG);
	oTargetSRS = new OGRSpatialReference();
	oTargetSRS->importFromEPSG(EPSGTo);
	poCT = OGRCreateCoordinateTransformation( oSourceSRS, oTargetSRS );

	if(poCT == NULL)
	{
		transformok = false;
		DM::Logger(DM::Error) << "Unknown transformation to EPSG:" << this->EPSGTo;
	}

}

SystemMapnikFeatureset::SystemMapnikFeatureset(mapnik::box2d<double> const& box, std::string const& encoding, DM::System * sys, const DM::View & v, int EPSG, int EPSGTo)
	: box_(box),
	  feature_id_(0),
	  tr_(new mapnik::transcoder(encoding)),
	  sys(sys),
	  view(v),
	  ctx_(boost::make_shared<mapnik::context_type>() ),
	  EPSG(EPSG),
	  EPSGTo(EPSGTo),
	  transformok(true)

{
	initTransformation();

	feature_uuids = sys->getUUIDs(view);

	//Get View embended in the system
	DM::View * vptr = sys->getViewDefinition(v.getName());
	if (!vptr) {
		DM::Logger(DM::Warning) << "View doesn't exist yet";
		return;
	}
	DM::Component * cmp = sys->getViewDefinition(v.getName())->getDummyComponent();
	std::map<std::string, DM::Attribute*> attrs_map =  cmp->getAllAttributes();

	for (std::map<std::string, DM::Attribute*>::const_iterator it = attrs_map.begin();
		 it != attrs_map.end();
		 ++it) {
		attribute_list.push_back(it->first);
	}
}

SystemMapnikFeatureset::~SystemMapnikFeatureset() { }

mapnik::feature_ptr SystemMapnikFeatureset::next()
{

	foreach(std::string attr_name, attribute_list) {
		ctx_->push(attr_name);
	}


	if (feature_id_ < feature_uuids.size())
	{
		std::string uuid = this->feature_uuids[feature_id_];
		++feature_id_;
		mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

		DM::Component * cmp = 0;

		if (!sys->getComponent(uuid)) {
			DM::Logger(DM::Warning) << "Component doesn t exist";
			return mapnik::feature_ptr();
		}

		switch (view.getType()) {
		case DM::NODE:
			cmp = (DM::Component *)  sys->getNode(uuid);
			draw_node( (DM::Node*) cmp, feature);
			break;
		case DM::FACE:
			cmp = (DM::Component *)  sys->getFace(uuid);
			draw_faces( (DM::Face*) cmp, feature);
			break;
		case DM::EDGE:
			cmp = (DM::Component *) sys->getEdge(uuid);
			draw_edges((DM::Edge*) cmp, feature);
			break;
		}
		this->add_attribute(cmp, feature);
		// return the feature!
		return feature;
	}

	// otherwise return an empty feature
	return mapnik::feature_ptr();
}

void SystemMapnikFeatureset::draw_faces(DM::Face * f, mapnik::feature_ptr feature)
{
	std::vector<DM::Node*> nodes = f->getNodePointers();
	mapnik::geometry_type * polygon = new mapnik::geometry_type(mapnik::Polygon);
	double x = nodes[0]->getX();
	double y = nodes[0]->getY();
	transform(&x,&y);
	polygon->move_to(x,y);
	for (uint i = 1; i < nodes.size(); i++) {
		x = nodes[i]->getX();
		y = nodes[i]->getY();
		transform(&x,&y);
		polygon->line_to(x, y);
	}
	x = nodes[0]->getX();
	y = nodes[0]->getY();
	transform(&x,&y);
	polygon->line_to(x,y);
	polygon->close_path();

	foreach (DM::Face * f,  f->getHolePointers()) {
		nodes = f->getNodePointers();
		x = nodes[0]->getX();
		y = nodes[0]->getY();
		transform(&x,&y);
		polygon->move_to(x, y);
		for (uint i = 1; i < nodes.size(); i++) {
			x = nodes[i]->getX();
			y = nodes[i]->getY();
			transform(&x,&y);
			polygon->line_to(x, y);
		}
		x = nodes[0]->getX();
		y = nodes[0]->getY();
		transform(&x,&y);
		polygon->line_to(x,y);
		polygon->close_path();
	}

	feature->add_geometry(polygon);

}


void SystemMapnikFeatureset::draw_edges(DM::Edge *e, mapnik::feature_ptr feature)
{
	mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);
	double x = e->getStartNode()->getX();
	double y = e->getStartNode()->getY();

	transform(&x,&y);
	line->move_to(x,y);

	x = e->getEndNode()->getX();
	y = e->getEndNode()->getY();
	transform(&x,&y);
	line->line_to(x,y);
	feature->add_geometry(line);

}

void SystemMapnikFeatureset::draw_node(DM::Node *n, mapnik::feature_ptr feature)
{
	mapnik::geometry_type * node = new mapnik::geometry_type(mapnik::Point);
	double x = n->getX();
	double y = n->getY();

	transform(&x,&y);
	node->move_to(x,y);
	feature->add_geometry(node);

}

void SystemMapnikFeatureset::add_attribute(DM::Component *cmp, mapnik::feature_ptr feature)
{
	foreach(std::string attr_name, attribute_list) {
		feature->put(attr_name, cmp->getAttribute(attr_name)->getDouble());
	}
}

bool SystemMapnikFeatureset::transform(double *x, double *y)
{
	if(!transformok)
		return false;

	if( poCT == NULL || !poCT->Transform( 1, x, y ) )
		return false;

	return true;
}
