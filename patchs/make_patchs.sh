#!/bin/bash

LIST_VARIANTS=`ls -d v_* | grep -v merged`

for i in $LIST_VARIANTS
do
    diff -Nur v_merged $i > patch.$i
    rm -rf $i
done
