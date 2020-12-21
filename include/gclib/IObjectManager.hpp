#ifndef GCLIB_IOBJECTMANAGER_HPP
#define GCLIB_IOBJECTMANAGER_HPP


namespace gclib {


    class IObjectManager {
    public:
        virtual void finalize(void* const begin, void* const end) = 0;
    };


} //namespace gclib


#endif //GCLIB_IOBJECTMANAGER_HPP
