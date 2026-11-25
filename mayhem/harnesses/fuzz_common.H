// Shared setup for the AMReX fuzz harnesses (mayhem/harnesses/*.cpp).
//
// amrex::Initialize runs once per process (kept on purpose: it only sets up read-only runtime
// state). `amrex.throw_exception=1` turns amrex::Error/Abort and failed AMREX_ALWAYS_ASSERTs into
// std::runtime_error (the library's documented error path for invalid input), and
// `amrex.signal_handling=0` keeps AMReX from installing its own signal handlers so real crashes
// reach the sanitizer.
#pragma once
#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fuzz {

inline std::string& scratch_dir () { static std::string d; return d; }

inline void init ()
{
    static bool done = false;
    if (done) { return; }
    done = true;
    static char arg0[] = "amrex_fuzz";
    static char* argv_storage[] = {arg0, nullptr};
    int argc = 1;
    char** argv = argv_storage;
    static std::ofstream devnull("/dev/null");
    using namespace amrex; // MPI_COMM_WORLD is amrex::MPI_COMM_WORLD in no-MPI builds
    amrex::Initialize(argc, argv, true, MPI_COMM_WORLD, [] () {
        amrex::ParmParse pp("amrex");
        pp.add("throw_exception", 1);
        pp.add("signal_handling", 0);
        pp.add("verbose", 0);
    }, devnull, devnull);
    const char* t = std::getenv("TMPDIR");
    std::string tmpl = std::string((t && *t) ? t : "/tmp") + "/amrex_fuzz_XXXXXX";
    if (mkdtemp(tmpl.data()) == nullptr) { std::perror("mkdtemp"); std::abort(); }
    scratch_dir() = tmpl;
}

// Empty the global ParmParse table so each input starts from the same state.
inline void reset_parmparse () { amrex::ParmParse::Finalize(); }

inline std::string write_scratch (const std::string& name, const char* data, size_t size)
{
    std::string path = scratch_dir() + "/" + name;
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ofs.write(data, static_cast<std::streamsize>(size));
    return path;
}

} // namespace fuzz
