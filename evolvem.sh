#!/bin/bash

games=400
scoretobeat=150

maxpop=120
minpop=30

envfactor=1
gen=1
bottlenecks=0

if [ -n "`ls -l dna/ | grep total.0`" ] ; then
	echo "Didn't find any dna, doing initial seeding..."
	mkdir dna 2>/dev/null
	for i in `seq $maxpop` ; do
		./newrandodna.sh start.dna
	done
fi
mkdir -p dna dead scores/dna 2>/dev/null

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
	local nl="`cat scores/$dna 2>/dev/null | grep [0-9] | wc -l`"
	if [ `./randof $(seq 0 $nl)` -eq 0 -a $nl -le 24 ] ; then
		./c.sh $dna p8-$jf
		local out="`./p8-$jf -m26 -g$games | grep Player.0.won`"
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
		parallel newscore ::: $all
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
	alive=`ls -l dna/ | wc -l`
	line
	echo ">>> Gen: $gen, alive: $alive, scoretobeat: $scoretobeat, best: $best, env: $envfactor, bottlenecks: $bottlenecks"
	line

	if [ $alive -ge $maxpop ] ; then
		let "envfactor++"
	fi

	scores=""
	all=""
	export -f newscore
	export games
	parallel newscore ::: dna/*
	for i in dna/* ; do
		score="`echo "($(cat scores/$i | tr '\n' '+' | sed 's/+$//'))/$(cat scores/$i | wc -l)" | bc`"
		if [ -z "$score" ] ; then
			echo "Looks like you're trying to quit (or something went wrong on $i)"
			rm -f p8-*
			exit 1
		fi
		if [ $score -ge $scoretobeat ] ; then
			scores="$scores $score"
			if [ $score -ge $best ] ; then
				best=$score
				echo "$i *OK* ${score}"
			else
				echo "$i  OK  $score"
			fi
			continue
		fi
		echo "$i died $score"
		if [ $score -lt $((best-300)) ] ; then
			rm "$i"
		else
			mv "$i" dead/
		fi
	done

	line
	mutateandmerge
	population="`ls -l dna/ | wc -l`"
	while [ $population -lt $minpop ] ; do
		mutateandmerge
		if [ $envfactor -gt 1 ] ; then
			let "envfactor--"
		fi
		let "scoretobeat-=1"
		let "bottlenecks++"
		population="`ls -l dna/ | wc -l`"
		if [ $population -le 1 ] ; then
			echo "WARNING: Everyone died! Resurrecting top $maxpop dnas"
			mv `./readscores.sh | tail -n$maxpop | awk '{ print $2 }' | xargs` dna/
			scores=$((best-1))
			let "best-=1"
		fi
	done

	scoretobeat=$((envfactor+`echo "$scores" | tr ' ' '\n' | grep [0-9] | sort | head -1`))
	if [ $((scoretobeat+5)) -ge $best ] ; then
		let "scoretobeat-=5"
	fi
	let "gen++"
done