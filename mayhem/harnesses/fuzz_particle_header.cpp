// ParticleHeader::parse + parse_grid_table: the particle plotfile/checkpoint "Header" file.
#include "fuzz_common.H"
#include <AMReX_ParticleHeader.H>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    std::istringstream is(std::string(reinterpret_cast<const char*>(data), size));
    try {
        amrex::ParticleHeader h;
        h.parse(is);
        h.parse_grid_table(is);
        (void)h.real_comp_names.size();
    } catch (std::exception const&) {
    }
    return 0;
}
