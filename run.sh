#!/bin/sh
export LD_PRELOAD=./libETH.so
./et.x86 $*
unset LD_PRELOAD
exit 0
