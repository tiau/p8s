#!/bin/bash

if [ -n "`ls -l dna/ | grep total.0`" ] ; then
	echo "Didn't find any dna, doing initial seeding..."
	mkdir dna 2>/dev/null
	for i in `seq 30` ; do
		./newrandodna.sh start.dna
	done
fi
mkdir -p scores/dna 2>/dev/null

games=10000
scoretobeat=5100
best=$scoretobeat

maxpop=60
minpop=30

envfactor=10
gen=1

function mutateandmerge() {
	echo "Mutating and merging dnas"
	for i in dna/* ; do
		if [ `./randof 1 2` = 1 ] ; then
			./newrandodna.sh $i
		else
			./newmergedna.sh $i `./randof dna/*`
		fi
		if [ "`ls -l dna/ | wc -l`" -gt $maxpop ] ; then
			break
		fi
	done
}

if [ -n "$1" ] ; then
	if [ -n "`echo "$1" | grep -i m`" ] ; then
		mutateandmerge
		echo "Grew population, exiting now"
		exit 0
	fi
	echo "Setting scoretobeat to $1"
	scoretobeat=$1
	best=$1
fi

while [ 1 ] ; do
	echo -------------------------------------------------------------
	echo ">>> This is gen: $gen, scoretobeat: $scoretobeat, best: $best"
	echo -------------------------------------------------------------

	scores=""
	for i in dna/* ; do
		if [ -f scores/$i ] ; then
			score="`cat scores/$i`"
		else
			./c.sh $i
			out="`./p8 -m14 -g$games | grep Player.0.won`"
			score="`echo "$out" | awk '{ print $4 }'`"
		fi
		echo "$score" > scores/$i
		if [ $score -ge $scoretobeat ] ; then
			scores="$scores $score"
			if [ $score -gt $best ] ; then
				best=$score
				echo "$i is BEST, score: $score"
			else
				echo "$i is okay, score: $score"
			fi
			continue
		fi
		echo "$i  DIED  , score: $score"
		rm "$i"
	done

	echo -------------------------------------------------------------
	mutateandmerge
	while [ "`ls -l dna/ | wc -l`" -lt $minpop ] ; do
		echo "WARNING: Population bottleneck!"
		mutateandmerge
		if [ $envfactor -gt 0 ] ; then
			let "envfactor--"
		fi
	done

	scoretobeat=$((envfactor+"`echo "$scores" | tr ' ' '\n' | grep [0-9] | sort | head -1`"))
	let "gen++"
done
