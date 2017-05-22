#!/bin/bash

HOST=`hostname`
TARGET="on_"$HOST

make cleanall
cd ..
ARCHIVE="garden_"$TARGET".tgz"
if [ -f $ARCHIVE ]; then
    mv $ARCHIVE $ARCHIVE".old"
fi
tar czvf $ARCHIVE garden
store $ARCHIVE