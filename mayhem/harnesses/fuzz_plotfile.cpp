// PlotFileData reader (AMReX_PlotFileDataImpl.cpp): how fcompare/fextract-style tools open an
// untrusted plotfile directory. The input is split at the first "\n%%CELL_H%%\n" line into the
// plotfile's top-level `Header` and its level-0 MultiFab header `Level_0/Cell_H`; both are written
// under a fresh <scratch>/plt directory for every input (nothing survives between inputs), then
// amrex::PlotFileData opens the directory exactly as a user tool would.
#include "fuzz_common.H"
#include <AMReX_PlotFileUtil.H>
#include <filesystem>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    namespace fs = std::filesystem;
    const std::string in(reinterpret_cast<const char*>(data), size);
    static const std::string delim = "\n%%CELL_H%%\n";
    const auto pos = in.find(delim);
    const std::string header = in.substr(0, pos);
    const std::string cell_h = (pos == std::string::npos) ? std::string() : in.substr(pos + delim.size());

    const fs::path plt = fs::path(fuzz::scratch_dir()) / "plt";
    std::error_code ec;
    fs::remove_all(plt, ec);
    fs::create_directories(plt / "Level_0");
    fuzz::write_scratch("plt/Header", header.data(), header.size());
    if (pos != std::string::npos) {
        fuzz::write_scratch("plt/Level_0/Cell_H", cell_h.data(), cell_h.size());
    }

    try {
        amrex::PlotFileData pf(plt.string());
        (void)pf.nComp();
        (void)pf.finestLevel();
        (void)pf.varNames();
        if (pf.nComp() > 0) {
            (void)pf.boxArray(0).size();
            (void)pf.probDomain(0);
            (void)pf.nGrowVect(0);
        }
    } catch (std::exception const&) {
    }
    return 0;
}
