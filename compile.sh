#!/bin/bash

PATH="../gcc/bin/:"$PATH

if mm $@
then
    echo "Uploading program..."
    cp build/$1.bin /f/prog.bin

    sleep 1
    cd tools
    mm reset -d 0
fi

