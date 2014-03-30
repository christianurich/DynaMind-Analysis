#ifndef EXPORTGEOJSON_H
#define EXPORTGEOJSON_H

#include <dmmodule.h>

using namespace DM;

class DM_HELPER_DLL_EXPORT ExportGeoJSON : public DM::Module
{
	DM_DECLARE_NODE(ExportGeoJSON)
private:
	std::string ViewName;
	DM::View vName;
	int EPSGCode;
	std::string FileName;
	std::string jsVaraibleName;
	bool asVariable;


public:
	ExportGeoJSON();
	void run();

};

#endif // EXPORTGEOJSON_H
