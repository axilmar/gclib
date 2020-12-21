#ifndef GCLIB_GLOBALDATA_HPP
#define GCLIB_GLOBALDATA_HPP


#include "gclib/DList.hpp"
#include "gclib/Mutex.hpp"
#include "ThreadData.hpp"


namespace gclib {


    //main struct with global data
    struct GlobalData {
        //global mutex
        Mutex mutex;

        //active thread data
        DList<ThreadData> activeThreadData;

        //terminated thread data
        DList<ThreadData> terminatedThreadData;

        //returns the one and only instance
        static GlobalData& instance();
    };


} //namespace gclib


#endif //GCLIB_GLOBALDATA_HPP
