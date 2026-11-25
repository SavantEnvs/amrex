// BoxArray::readFrom (checkpoint/plotfile BoxArray text format) followed by the BoxArray/BoxList
// set operations applications run on what they read back.
#include "fuzz_common.H"
#include <AMReX_BoxArray.H>
#include <AMReX_BoxList.H>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    std::istringstream is(std::string(reinterpret_cast<const char*>(data), size));
    try {
        amrex::BoxArray ba;
        ba.readFrom(is);
        // Follow-on set operations are quadratic in the box count and linear in the volume;
        // only run them on BoxArrays of a size a checkpoint for a small run would hold.
        if (ba.size() > 64) { return 0; }
        // A BoxArray holds boxes of a single IndexType (BoxArray::ixType(), AMReX_BoxArray.H); the
        // follow-on operations (minimalBox, complementIn, ...) are only defined for such arrays.
        for (int i = 1; i < ba.size(); ++i) {
            if (ba[i].ixType() != ba[0].ixType()) { return 0; }
        }
        amrex::Box mb = ba.minimalBox();
        if (mb.ok() && mb.numPts() > (amrex::Long(1) << 24)) { return 0; }
        for (int i = 0; i < ba.size(); ++i) {
            if (ba[i].ok() && ba[i].numPts() > (amrex::Long(1) << 24)) { return 0; }
        }
        (void)ba.ok();
        (void)ba.isDisjoint();
        (void)ba.numPts();
        amrex::BoxList bl(ba);
        bl.simplify();
        amrex::BoxList bl2(ba);
        bl2.maxSize(4);
        bl2.simplify();
        if (mb.ok()) {
            (void)ba.contains(mb);
            amrex::BoxArray c = amrex::complementIn(mb, ba);
            (void)ba.intersections(mb);
            (void)c.size();
        }
        amrex::BoxArray b2 = ba;
        b2.removeOverlap();
        amrex::BoxArray b3 = ba;
        b3.maxSize(8);
        amrex::BoxArray b4 = ba;
        b4.coarsen(2);
        (void)b4.coarsenable(2);
        std::ostringstream os;
        ba.writeOn(os);
    } catch (std::exception const&) {
    }
    return 0;
}
