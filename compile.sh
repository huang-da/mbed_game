#!/bin/bash

PATH="../gcc/bin/:"$PATH

mm $@
rm /f/*.bin
cp build/$1.bin /f/

sleep 1
cd tools
mm reset -d 0

