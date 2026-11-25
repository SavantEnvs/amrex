// Checkpoint stream readers: Geometry, RealBox, CoordSys, Orientation, DistributionMapping.
// The first byte selects the reader, the rest is the stream.
#include "fuzz_common.H"
#include <AMReX_Geometry.H>
#include <AMReX_RealBox.H>
#include <AMReX_CoordSys.H>
#include <AMReX_Orientation.H>
#include <AMReX_DistributionMapping.H>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    if (size < 1) { return -1; }
    fuzz::init();
    std::istringstream is(std::string(reinterpret_cast<const char*>(data + 1), size - 1));
    try {
        switch (data[0] % 5) {
        case 0: { amrex::Geometry g; is >> g; (void)g.CellSize(); break; }
        case 1: { amrex::RealBox rb; is >> rb; (void)rb.volume(); break; }
        case 2: { amrex::CoordSys c; is >> c; break; }
        case 3: { amrex::Orientation o; is >> o; break; }
        default: { amrex::DistributionMapping dm; dm.readFrom(is); (void)dm.size(); break; }
        }
    } catch (std::exception const&) {
    }
    return 0;
}
