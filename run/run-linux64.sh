#!/bin/sh

cd "$(dirname $0)"
LD_LIBRARY_PATH="$PWD/lib" ./lib/ld-linux-x86-64.so.2 ./bin/landscape-demo $@
