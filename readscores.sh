#!/bin/bash

for i in dna/* dead/* ; do
	sn="`echo "$i" | awk -F/ '{ print $2 }'`"
	if [ -f "scores/dna/$sn" ] ; then
		echo "`cat scores/dna/$sn` $i"
	fi
done | sort -n
