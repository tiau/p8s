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
	dna=$1
	./c.sh $dna
	out="`./p8 -m14 -g$games | grep Player.0.won`"
	score="`echo "$out" | awk '{ print $4 }'`"
	echo "$score" >> scores/$dna
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
		if [ "`ps ax | grep evolve.sh | wc -l`" -gt 3 ] ; then
			echo "Can't generate new scores while evolving"
			exit 1
		fi
		echo "$Generating $(($#-1)) new scores"
		for i in ${*} ; do
			if [ -f "$i" ] ; then
				echo "Updating score for $i"
				home="`echo "$i" | sed 's/^\(.*\/\)[^\/]*$/\1/'`"
				jf="`echo "$i" | sed 's/^.*\/\([^\/]*\)$/\1/'`"
				mv "$i" dna/ 2>/dev/null
				newscore dna/$jf
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
	for i in dna/* ; do
		if [ -f scores/$i ] ; then
			if [ `randof 1 2 3 4 5` -eq 1 ] ; then
				newscore $i
			fi
		else
			newscore $i
		fi
		score="`echo "($(cat scores/$i | tr '\n' '+' | sed 's/+$//'))/$(cat scores/$i | wc -l)" | bc`"
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
