#!/bin/bash

echo Building 3rdparty dependencies...

FORCE_BUILD_3RDPARTY=$1

mkdir -p ./lib/
mkdir -p ../bin/debug/
mkdir -p ../bin/release/

cd scripts

./build_openssl.sh ${FORCE_BUILD_3RDPARTY} || exit 1

./build_libevent.sh ${FORCE_BUILD_3RDPARTY} || exit 1

echo Building 3rdparty dependencies finish...

pwd
