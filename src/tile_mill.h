#ifndef TILEMILL_H
#define TILEMILL_H

#include "dmmodule.h"

using namespace DM;

class DM_HELPER_DLL_EXPORT TileMill : public DM::Module
{
    DM_DECLARE_NODE(TileMill)
    private:
        std::string viewName;
    DM::View vData;
	int maxZoomLevel;

    public:
        TileMill();  //Constructor where data and objects for the module are defined
        void init();
        void run(); //Function executed from the Simulation

};


#endif // TILEMILL_H
