#ifndef GCLIB_GCCUSTOMBLOCKHEADERVTABLE_HPP
#define GCLIB_GCCUSTOMBLOCKHEADERVTABLE_HPP


#include "GCIBlockHeaderVTable.hpp"


/**
 * Class that allows the customization of the individual functions of a block header vtable.
 * @param Scan type of the scan function.
 * @param Finalize type of the finalize function.
 * @param Free type of the free function.
 */
template <class Scan, class Finalize, class Free> class GCCustomBlockHeaderVTable : public GCIBlockHeaderVTable {
public:
    /**
     * Constructor.
     * @param scan function.
     * @param finalize finalize function.
     * @param free free function.
     */
    GCCustomBlockHeaderVTable(Scan&& scan, Finalize&& finalize, Free&& free)
        : m_scan(std::move(scan))
        , m_finalize(std::move(finalize))
        , m_free(std::move(free))
    {
    }

    /**
     * Scans for member pointers by invoking the scan function.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
        m_scan(start);
    }

    /**
     * Finalizes the objects by invoking the finalize function.
     * @param start memory start.
     * @param end memory end.
     */
    void finalize(void* start, void* end) noexcept final {
        m_finalize(start);
    }

    /**
     * Frees memory either by invoking the free function.
     * @param mem pointer to memory to free.
     */
    void free(void* mem) noexcept final {
        m_free(mem);
    }

private:
    Scan m_scan;
    Finalize m_finalize;
    Free m_free;
};


/**
 * Helper function for creating a custom block header vtable.
 * @param scan the scan function.
 * @param finalize the finalize function.
 * @param free the free function.
 * @return custom block header vtable.
 */
template <class Scan, class Finalize, class Free> 
GCCustomBlockHeaderVTable<Scan, Finalize, Free> GCMakeBlockHeaderVTable(Scan&& scan, Finalize&& finalize, Free&& free) {
    return { std::move(scan), std::move(finalize), std::move(free) };
}


#endif //GCLIB_GCCUSTOMBLOCKHEADERVTABLE_HPP
