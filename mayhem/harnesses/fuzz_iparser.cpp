// amrex::IParser: integer math-expression parser and evaluator.
#include "fuzz_common.H"
#include <AMReX_IParser.H>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    std::string s(reinterpret_cast<const char*>(data), size);
    try {
        amrex::IParser p(s);
        p.setConstant("n", 4);
        p.registerVariables({"i", "j"});
        (void)p.depth();
        (void)p.maxStackSize();
        auto e = p.compileHost<2>();
        volatile long long r = e(3, -7);
        (void)r;
    } catch (std::exception const&) {
    }
    return 0;
}
