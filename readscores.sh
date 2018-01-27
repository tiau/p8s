#!/bin/bash

for i in dna/* dead/* ; do
	sn="`echo "$i" | awk -F/ '{ print $2 }'`"
	if [ -f "scores/dna/$sn" ] ; then
		echo "($(cat scores/dna/$sn | tr '\n' '+' | sed 's/+$//'))/$(cat scores/dna/$sn | wc -l)" | bc | tr '\n' ' '
		if [ -z "$1" ] ; then
			echo "$i"
		else
			echo -n "$i "
			cat scores/dna/$sn | wc -l
		fi
	fi
done | sort -n
