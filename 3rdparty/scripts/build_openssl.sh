#!/bin/bash
CUR_PWD=${pwd}

cd ../openssl

./config shared zlib-dynamic || exit 1

make -j4 || exit 1

cp -rf ./*.so ../../bin/debug/
cp -rf ./*.so ../../bin/release/
cp -rf ./*.so.* ../../bin/debug/
cp -rf ./*.so.* ../../bin/release/

cp -rf ./include/openssl/opensslconf.h .

make clean

cp ../../bin/debug/libssl.so* .
cp ../../bin/debug/libcrypto.so* .

mv -f ./opensslconf.h ./include/openssl

echo Compiling openssl end...

cd ${CUR_PWD}
