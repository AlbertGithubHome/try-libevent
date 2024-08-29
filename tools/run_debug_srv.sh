#!/bin/bash

export LC_ALL="C"

ulimit -c unlimited

cd ../bin/debug
./server
