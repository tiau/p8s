#!/bin/bash

gs=600
nt=60

echo "Benchmarking..."

x=""
for i in `seq $nt` ; do
	n="$(./time.sh ./p8 -m3453 -g$gs 2>&1 | grep user | sed 's/^.*\ *0m\([0-9\.]*\)s$/\1/')"
	x="$x $n"
done

s="$(stats $x)"
sd="$(echo "$s" | grep "^StdDev:" | awk '{ print $2 }')"
mean="$(echo "$s" | grep "^Mean:" | awk '{ print $2 }')"
n="$(echo "$s" | grep "^n:" | awk '{ print $2 }')"
sum="$(echo "$s" | grep "^Sum:" | awk '{ print $2 }')"
gps="$(echo "scale=2;($n*$gs)/$sum" | bc)"
echo
echo "$s" | grep -v Slope | grep -v Skewness | grep -v Median | grep -v Mode
echo "$gps games per second"
