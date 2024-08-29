#!/bin/bash
CUR_PWD=${pwd}

cd ../libevent

sed -in 's/^#define.*EVBUFFER_MAX_READ.*/#define EVBUFFER_MAX_READ 65536/g' buffer.c

autoreconf -f -i || exit 1

./configure CPPFLAGS="-I../openssl/include -fPIC" OPENSSL_LIBADD="-L../openssl -lssl -lcrypto" --disable-static || exit 1

make clean

make -j4 || exit 1

cp -rf ./.libs/*.so ../../bin/debug/
cp -rf ./.libs/*.so ../../bin/release/
cp -rf ./.libs/*.so.* ../../bin/debug/
cp -rf ./.libs/*.so.* ../../bin/release/

make clean

echo Compiling libevent end...

cd ${CUR_PWD}
