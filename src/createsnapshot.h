#ifndef CREATESNAPSHOT_H
#define CREATESNAPSHOT_H

#include <dmmodule.h>
#include <dmlogger.h>

//gdal
#include <ogrsf_frmts.h>
#include <gdal_priv.h>

using namespace DM;

class DM_HELPER_DLL_EXPORT CreateSnapshot : public DM::Module
{
	DM_DECLARE_NODE(CreateSnapshot)
	private:
		std::map<std::string, std::string> exportMaps;
		std::map<std::string, std::string> styles;
	DM::View vData;
	int global_counter;
	int EPSGCode;
	std::string fileName;

	int height;
	int width;
	bool transformok;

	OGRCoordinateTransformation * poCT;

	public:
		CreateSnapshot();  //Constructor where data and objects for the module are defined
		void init();
		void run(); //Function executed from the Simulation
		bool createInputDialog();
		void initTransForm(int EPSG, int EPGSTo);
		bool transform(double *x, double *y);


};
#endif // CREATESNAPSHOT_H
