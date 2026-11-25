// ParmParse inputs/TOML file reader + typed queries (ParmParse::addfile, the path every AMReX
// application reads its inputs file through).
#include "fuzz_common.H"
#include <AMReX_Enum.H>
#include <sstream>
#include <vector>

AMREX_ENUM(FuzzColor, red, green, blue);

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    fuzz::reset_parmparse();
    std::string path = fuzz::write_scratch("inputs", reinterpret_cast<const char*>(data), size);
    try {
        amrex::ParmParse::addfile(path);
        std::ostringstream os;
        amrex::ParmParse::dumpTable(os);
        amrex::ParmParse::prettyPrintTable(os);
        for (const char* prefix : {"", "a", "b", "fuzz", "geometry", "amr", "table"}) {
            amrex::ParmParse pp(prefix);
            for (const char* key : {"i", "x", "s", "b", "v", "m", "e", "c", "n_cell", "prob_lo", "a", "b.c"}) {
                try { int i; pp.query(key, i); } catch (std::exception const&) {}
                try { double d; pp.query(key, d); } catch (std::exception const&) {}
                try { bool b; pp.query(key, b); } catch (std::exception const&) {}
                try { std::string s; pp.query(key, s); } catch (std::exception const&) {}
                try { std::vector<int> v; pp.queryarr(key, v); } catch (std::exception const&) {}
                try { std::vector<double> v; pp.queryarr(key, v); } catch (std::exception const&) {}
                try { std::vector<std::string> v; pp.queryarr(key, v); } catch (std::exception const&) {}
                try { std::vector<std::vector<int>> v; pp.queryarr(key, v); } catch (std::exception const&) {}
                try { double d; pp.queryWithParser(key, d); } catch (std::exception const&) {}
                try { int i; pp.queryWithParser(key, i); } catch (std::exception const&) {}
                try { std::vector<double> v; pp.queryarrWithParser(key, 3, v); } catch (std::exception const&) {}
                try { FuzzColor c; pp.query_enum_sloppy(key, c, "_"); } catch (std::exception const&) {}
                try { (void)pp.countval(key); (void)pp.contains(key); } catch (std::exception const&) {}
            }
        }
        (void)amrex::ParmParse::getEntries();
    } catch (std::exception const&) {
    }
    return 0;
}
