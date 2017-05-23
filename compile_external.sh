#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CPUS="$( grep -c ^processor /proc/cpuinfo )"
cd $DIR/external/common_backend
sh ./compile_external.sh
cd $DIR/external/libsodium
./configure
make -j$CPUS
cd $DIR/external/uWebSockets
make
cd $DIR