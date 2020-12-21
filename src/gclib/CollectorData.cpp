#include "CollectorData.hpp"


namespace gclib {


    //returns the one and only instance
    CollectorData& CollectorData::instance() {
        static CollectorData cd;
        return cd;
    }


} //namespace gclib
