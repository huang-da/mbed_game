#!/bin/bash

PATH="../gcc/bin/:"$PATH

if mm $@
then
    echo "Code size: (text=code, data=static vars, bss=static ram)"
    arm-none-eabi-size build/elf

    if echo $* | grep -- "-ne" > /dev/null
    then
	echo "No upload."
    else
	echo "Uploading program..."
	cp build/$1.bin /f/prog.bin

	sleep 1
	cd tools
	mm reset -d 0
    fi
fi

