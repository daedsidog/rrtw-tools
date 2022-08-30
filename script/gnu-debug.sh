_cwd=$(pwd)
_script_dir=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
cd ${_script_dir}
_cxx_extra=$(cat gnu-debug-flags.txt)
cd ..
cmake -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAG} ${_cxx_extra}" \
      -DDCC_LIB_DIR=ext/lib/gnu \
      -B build/gnu/debug
cmake --build build/gnu/debug -j 8
cd ${_cwd}
