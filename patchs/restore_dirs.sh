#!/bin/bash

LIST_VARIANTS=`ls -d patch.*`

for i in $LIST_VARIANTS
do
    NAME=`echo $i | sed 's/patch\.//'`
    cp -R v_merged $NAME
    cd $NAME
    patch -p1 < ../$i
    cd ..
    rm $i
done
