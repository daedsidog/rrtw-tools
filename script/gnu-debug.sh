_cwd=$(pwd)
_script_dir=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
cd ${_script_dir}
_cxx_extra=$(cat gnu-debug-flags.txt)
cd ..
source ext/dcc/script/gnu-debug.sh
cmake -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAG} ${_cxx_extra}" \
      -DDCC_LIB_DIR=ext/dcc/build/gnu/debug \
      -DSTATIC_LIB_SUFFIX=a \
      -B build/gnu/debug
cmake --build build/gnu/debug -j 8
cd ${_cwd}
