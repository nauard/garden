#!/bin/bash

#	watch_output.sh
# displays current output of world prg

clear
WORLD_PROCESS=toto
FILE=`grep OUT_CMD textInterface.h | cut -d'"' -f2`

while [ -n "$WORLD_PROCESS" ]
do
    if [ -f $FILE ]; then
        # test if exit has been requested
        X=`cat $FILE`
        if [ "$X" == "exitall" ]; then
            exit 0;
        fi

	# clears display
	clear
        echo "last output is :"
	# prints current output
	cat $FILE
	# remove it - to not be displayed a second time
	rm $FILE
    fi
    sleep 2
    WORLD_PROCESS=`ps ax | grep world | grep -v grep`
done
