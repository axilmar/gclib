#include "GCCollector.hpp"


//Returns the one and only collector instance.
GCCollector& GCCollector::instance() {
    static GCCollector collector;
    return collector;
}
