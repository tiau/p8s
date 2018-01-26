#!/bin/bash

./mergedna.py $1 $2 > temp.dna
hash="`md5sum temp.dna | awk '{ print $1 }'`"
mv temp.dna dna/$hash.dna
