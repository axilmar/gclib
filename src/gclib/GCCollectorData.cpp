#include "GCCollectorData.hpp"


//Returns the one and only collector instance.
GCCollectorData& GCCollectorData::instance() {
    static GCCollectorData collectorData;
    return collectorData;
}
