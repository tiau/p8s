#!/bin/bash

games=10000
scoretobeat=5100

maxpop=100
minpop=50

envfactor=20
gen=1
bottlenecks=0

if [ -n "`ls -l dna/ | grep total.0`" ] ; then
	echo "Didn't find any dna, doing initial seeding..."
	mkdir dna 2>/dev/null
	for i in `seq $maxpop` ; do
		./newrandodna.sh start.dna
	done
fi
mkdir -p scores/dna 2>/dev/null
mkdir dead 2>/dev/null

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

function newscore() {
	local dna=$1
	local jf="`echo "$dna" | sed 's/^.*\/\([^\/]*\)$/\1/'`"
	local nl="`cat scores/$dna 2>/dev/null | wc -l`"
	if [ $nl -eq 0 -o `./randof 1 2 3` -eq 1 -a $nl -le 9 ] ; then
		./c.sh $dna p8-$jf
		local out="`./p8-$jf -m14 -g$games | grep Player.0.won`"
		local score="`echo "$out" | awk '{ print $4 }'`"
		echo "$score" >> scores/$dna
		local out="`./p8-$jf -m14 -g$games | grep Player.0.won`"
		local score="`echo "$out" | awk '{ print $4 }'`"
		echo "$score" >> scores/$dna
		rm p8-$jf
	fi
}

function line {
	echo -------------------------------------------------------------------------------
}

if [ -n "$1" ] ; then
	if [ -n "`echo "$1" | grep -i m`" ] ; then
		mutateandmerge
		echo "Grew population, exiting now"
		exit 0
	elif [ -n "`echo "$1" | grep -i newscore`" ] ; then
		echo "Generating $(($#-1)) new scores"
		all=""
		for i in ${*} ; do
			if [ -f "$i" ] ; then
				echo "Updating score for $i"
				home="`echo "$i" | sed 's/^\(.*\/\)[^\/]*$/\1/'`"
				jf="`echo "$i" | sed 's/^.*\/\([^\/]*\)$/\1/'`"
				mv "$i" dna/ 2>/dev/null
				all="$all dna/$jf"
			fi
		done
		export -f newscore
		export games
		parallel --will-cite newscore ::: $all
		for i in ${*} ; do
			if [ -f "$i" ] ; then
				mv dna/$jf $home 2>/dev/null
			fi
		done
		exit 0
	elif [ -n "`echo "$1" | grep [^0-9]`" ] ; then
		echo "Bad args"
		exit 1
	fi
	echo "Setting scoretobeat to $1"
	scoretobeat=$1
	if [ -n "$2" ] ; then
		echo "Setting envfactor to $2"
		envfactor=$2
	fi
fi
best=$scoretobeat

while [ 1 ] ; do

	line
	echo ">>> Gen: $gen, alive: `ls -l dna/ | wc -l`, scoretobeat: $scoretobeat, best: $best, env: $envfactor, bottlenecks: $bottlenecks"
	line

	scores=""
	all=""
	export -f newscore
	export games
	parallel --will-cite newscore ::: dna/*
	for i in dna/* ; do
		score="`echo "($(cat scores/$i | tr '\n' '+' | sed 's/+$//'))/$(cat scores/$i | wc -l)" | bc`"
		if [ -z "$score" ] ; then
			echo "Looks like you're trying to quit (or something went wrong)"
			rm -f p8-*
			exit 1
		fi
		if [ $score -ge $scoretobeat ] ; then
			scores="$scores $score"
			if [ $score -ge $best ] ; then
				best=$score
				echo "$i BEST: $score"
			else
				echo "$i Okay: $score"
			fi
			continue
		fi
		echo "$i died: $score"
		mv "$i" dead/
	done

	line
	mutateandmerge
	while [ "`ls -l dna/ | wc -l`" -lt $minpop ] ; do
		echo "Population bottleneck!"
		mutateandmerge
		if [ $envfactor -gt 1 ] ; then
			let "envfactor--"
		fi
		let "bottlenecks++"
	done

	scoretobeat=$((envfactor+"`echo "$scores" | tr ' ' '\n' | grep [0-9] | sort | head -1`"))
	if [ $scoretobeat -ge $best ] ; then
		scoretobeat=$((best-10))
		echo ">>> Environment harshness is outpacing fitness! <<<"
	fi
	let "gen++"
done
