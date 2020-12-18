#! /usr/bin/bash
rm -rf build
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cd build
make -j8
make test
cd ..
popd
