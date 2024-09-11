#!/bin/bash
CUR_PWD=${pwd}

cd ../libuv

mkdir -p build

(cd build && cmake .. -DBUILD_TESTING=ON) # generate project with tests
cmake --build build                       # add `-j <n>` with cmake >= 3.12

# cp -rf ./*.so ../../bin/debug/
# cp -rf ./*.so ../../bin/release/
# cp -rf ./*.so.* ../../bin/debug/
# cp -rf ./*.so.* ../../bin/release/

echo Compiling libuv end...

cd ${CUR_PWD}
