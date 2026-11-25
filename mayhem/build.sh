#!/usr/bin/env bash
# mayhem/build.sh — build AMReX (static, 3D, no MPI/Fortran/linear solvers) with the sanitizers,
# link the fuzz harnesses in mayhem/harnesses against it, and build a set of upstream Tests/ programs
# with normal flags for mayhem/test.sh. Re-runnable and offline: no network access, CMake re-configures
# the existing (gitignored) mayhem/build/ trees in place.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS
cd "$SRC"

B="$SRC/mayhem/build"     # gitignored by upstream's `build/` pattern
COMMON=(-DAMReX_SPACEDIM=3 -DAMReX_MPI=OFF -DAMReX_OMP=OFF -DAMReX_FORTRAN=OFF
        -DAMReX_FORTRAN_INTERFACES=OFF -DAMReX_LINEAR_SOLVERS=OFF -DAMReX_PARTICLES=ON
        -DAMReX_EB=OFF -DAMReX_BUILD_SHARED_LIBS=OFF -DAMReX_PRECISION=DOUBLE)

# 1) Sanitized, instrumented library (installed into a private prefix for the harness compiles).
cmake -S "$SRC" -B "$B/fuzz" "${COMMON[@]}" -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX="$B/fuzz-install" \
      -DCMAKE_C_FLAGS="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS -O1" \
      -DCMAKE_CXX_FLAGS="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS -O1" \
      -DCMAKE_C_FLAGS_DEBUG= -DCMAKE_CXX_FLAGS_DEBUG=
cmake --build "$B/fuzz" -j"$MAYHEM_JOBS"
cmake --install "$B/fuzz" >/dev/null

INC="$B/fuzz-install/include"
LIB=$(ls "$B"/fuzz-install/lib*/libamrex_3d.a)
CXXF=(-std=c++20 $SANITIZER_FLAGS $DEBUG_FLAGS -O1 -I"$INC" -I"$SRC/mayhem/harnesses")
$CXX ${SANITIZER_FLAGS} $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$B/lsan_off.o"
$CC ${SANITIZER_FLAGS} $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$B/standalone_main.o"
for h in "$SRC"/mayhem/harnesses/fuzz_*.cpp; do
  n=$(basename "$h" .cpp)
  $CXX "${CXXF[@]}" -c "$h" -o "$B/$n.o"
  $CXX ${SANITIZER_FLAGS} $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$B/$n.o" "$B/lsan_off.o" "$LIB" -o "$SRC/$n"
  $CXX ${SANITIZER_FLAGS} $DEBUG_FLAGS "$B/standalone_main.o" "$B/$n.o" "$B/lsan_off.o" "$LIB" -o "$SRC/$n-standalone"
done

# 2) Upstream unit tests (Tests/Parser, Parser2, ParmParse, TOML, Enum) with normal flags.
cmake -S "$SRC" -B "$B/tests" "${COMMON[@]}" -DCMAKE_BUILD_TYPE=Release -DAMReX_ENABLE_TESTS=ON \
      -DCMAKE_C_FLAGS="$COVERAGE_FLAGS" -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS"
cmake --build "$B/tests" -j"$MAYHEM_JOBS" --target \
      Test_Parser_3d Test_Parser2_3d Test_ParmParse_3d Test_TOML_3d Test_Enum_3d
