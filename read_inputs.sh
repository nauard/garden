#!/bin/bash

#	read_inputs.sh
# reads input commands addressed to world prg


INPUT_FILE=`grep INP_CMD textInterface.h | cut -d'"' -f2`
OUTPUT_FILE=`grep OUT_CMD textInterface.h | cut -d'"' -f2`

clear
# clears file input
> $INPUT_FILE
WORLD_PROCESS=toto

while [ -n "$WORLD_PROCESS" ]
do
    echo "enter your command :"
    echo "(exit to quit reader  -  exitall to quit reader & watcher)"
    # read input at keyboard
    read X

    # test if exit is requested
    if [ "$X" == "exitall" ]; then
        echo $X > $OUTPUT_FILE
    fi
    if [ "$X" == "exit" -o "$X" == "exitall" ]; then
        exit 0;
    fi

    # write it down into command file
    echo $X > $INPUT_FILE
    # wait for world prg to take it into account
    while [ -f $INPUT_FILE ]
    do
        sleep 1
    done
    #sleep 3
    # clears command entered - to not let world reinterpret it a second time
    #> $INPUT_FILE

    # clears display
    clear
    WORLD_PROCESS=`ps ax | grep world | grep -v grep`
done