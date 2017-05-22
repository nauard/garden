#!/bin/bash

make cleanall
cd ..

# make a backup
if [ -d garden_backup ]; then
    echo "Warning : a backup already exist"
    exit 1
fi
cp -R garden garden_backup

# get the remote(s) archive(s)
recall "garden*tgz"
ARCHIVE=`ls garden*tgz | grep -v backup`
NB_ARCHIVES=`echo $ARCHIVE | wc -w`
if [ $NB_ARCHIVES -gt 1 ]; then
    echo "Warning : more than 1 archive : "$ARCHIVE
    exit 2
fi

tar zxvf $ARCHIVE && rm $ARCHIVE
diff -r garden_backup garden
