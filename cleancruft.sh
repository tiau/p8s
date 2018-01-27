#!/bin/bash

minscore=5500
if [ -n "$1" ] ; then
	if [ -n "`echo "$1" | grep [^0-9]`" ] ; then
		echo "Give me a score below which to clean, e.g. $minscore"
		exit 1
	fi
	minscore=$1
fi

echo "Gathering cruft (score < $minscore) to clean..."

for i in `./readscores.sh | sed 's/\ /-/'` ; do
	score="`echo "$i" | awk -F- '{ print $1 }'`"
	if [ $score -lt $minscore ] ; then
		fn="`echo "$i" | awk -F- '{ print $2 }'`"
		rm $fn
		echo "Cleaning $fn at score $score"
	fi
done
