#!/bin/bash

PATH="../gcc/bin/:"$PATH

mm $@
cp build/$1.bin /f/prog.bin

sleep 1
cd tools
mm reset -d 0

