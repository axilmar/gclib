#include "GlobalData.hpp"


namespace gclib {


    //returns the one and only instance
    GlobalData& GlobalData::instance() {
        static GlobalData cd;
        return cd;
    }


} //namespace gclib
