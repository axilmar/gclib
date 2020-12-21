#ifndef GCLIB_COLLECTORDATA_HPP
#define GCLIB_COLLECTORDATA_HPP


#include "gclib/DList.hpp"
#include "gclib/Mutex.hpp"
#include "ThreadData.hpp"


namespace gclib {


    //main struct with global data
    struct CollectorData {
        //global mutex
        Mutex mutex;

        //active thread data
        DList<ThreadData> activeThreadData;

        //terminated thread data
        DList<ThreadData> terminatedThreadData;

        //returns the one and only instance
        static CollectorData& instance();
    };


} //namespace gclib


#endif //GCLIB_COLLECTORDATA_HPP
