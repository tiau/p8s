#!/bin/bash

if [ -z "$1" ] ; then
	echo -e "Looking in live dna\n"
	target="dna/*"
elif ! [ -d "$1" ] ; then
	echo "Looking at top $1 scorers"
	target="`./readscores.sh | tail -n$1 | awk '{ print $2 }' | tr '\n' ' '`"
else
	echo -e "Looking in $1\n"
	target="$1/*"
	f=$1
fi

for a in `cat start.dna | awk '{ print $1 }'` ; do
	echo -ne "$a\t"
	for i in $target ; do
		grep $a $i 2>/dev/null | awk '{ print $2 }'
	done | sort -n | uniq | tr '\n' ' '
	echo
done
