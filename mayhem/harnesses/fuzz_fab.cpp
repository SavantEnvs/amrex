// FArrayBox::readFrom: the FAB on-disk format (header + RealDescriptor + data conversion).
#include "fuzz_common.H"
#include <AMReX_FArrayBox.H>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    const std::string s(reinterpret_cast<const char*>(data), size);
    try {
        int nc = 0;
        {
            std::istringstream is(s);
            amrex::Box b = amrex::FArrayBox::skipFAB(is, nc);
            // readFrom allocates box.numPts()*ncomp Reals from the header; bound it so the
            // fuzzer explores the reader rather than the allocator.
            if (!b.ok() || nc <= 0 || b.numPts() > (amrex::Long(1) << 20) / nc) { return 0; }
        }
        {
            std::istringstream is(s);
            amrex::FArrayBox fab;
            fab.readFrom(is);
            volatile double v = fab.nComp() > 0 ? fab.max<amrex::RunOn::Host>(0) : 0.0;
            (void)v;
        }
        {
            std::istringstream is(s);
            amrex::FArrayBox fab;
            fab.readFrom(is, 0);
        }
    } catch (std::exception const&) {
    }
    return 0;
}
