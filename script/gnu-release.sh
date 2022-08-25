_cwd=$(pwd)
_script_dir=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
cd ${_script_dir}
_cxx_extra=$(gnu-release-flags.txt)
cd ..
cmake -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} ${_cxx_extra}" \
      -DDCC_BUILD_TESTS=OFF \
      -DDCC_BUILD_DEMOS=ON \
      -DDCC_BUILD_UTILS=OFF \
      -B build/gnu/release
cmake --build build/gnu/release -j 8
cd ${_cwd}
_script_dir=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
