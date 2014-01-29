#ifndef TILEMILL_H
#define TILEMILL_H

#include <dmmodule.h>
#include <dmlogger.h>

//gdal
#include <ogrsf_frmts.h>
#include <gdal_priv.h>

using namespace DM;

class DM_HELPER_DLL_EXPORT TileMill : public DM::Module
{
    DM_DECLARE_NODE(TileMill)
    private:
		std::map<std::string, std::string> exportMaps;
		std::map<std::string, std::string> styles;
    DM::View vData;
	int maxZoomLevel;
	int global_counter;
	int EPSGCode;
	std::string folderName;

	int currrentZoomLevel;
	double centre_x;
	double centre_y;
	bool transformok;

	OGRCoordinateTransformation * poCT;

    public:
        TileMill();  //Constructor where data and objects for the module are defined
        void init();
        void run(); //Function executed from the Simulation
		bool createInputDialog();
		void initTransForm(int EPSG, int EPGSTo);
		bool transform(double *x, double *y);


};


#endif // TILEMILL_H
