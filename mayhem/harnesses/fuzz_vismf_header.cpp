// VisMF::Header stream reader: the MultiFab "_H" header of every plotfile and checkpoint.
#include "fuzz_common.H"
#include <AMReX_VisMF.H>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    std::istringstream is(std::string(reinterpret_cast<const char*>(data), size));
    try {
        amrex::VisMF::Header h;
        is >> h;
        (void)h.m_ba.size();
    } catch (std::exception const&) {
    }
    return 0;
}
