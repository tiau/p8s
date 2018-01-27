#!/bin/bash

./randodna.py $1 > temp.dna
if [ $? -ne 0 ] ; then
	exit 1
fi
hash="`md5sum temp.dna | awk '{ print $1 }'`"
mv temp.dna dna/$hash.dna
