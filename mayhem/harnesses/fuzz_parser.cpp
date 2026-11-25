// amrex::Parser: math-expression parser, AST optimizer and bytecode compiler/evaluator.
#include "fuzz_common.H"
#include <AMReX_Parser.H>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
    fuzz::init();
    std::string s(reinterpret_cast<const char*>(data), size);
    try {
        amrex::Parser p(s);
        p.setConstant("a", 1.5);
        p.registerVariables({"x", "y", "z"});
        (void)p.depth();
        (void)p.maxStackSize();
        (void)p.symbols();
        auto e = p.compileHost<3>();
        volatile double r = e(1.0, 2.0, 3.0);
        (void)r;
    } catch (std::exception const&) {
    }
    return 0;
}
