#!/bin/bash

if [ -z "$2" ] ; then
	o=p8
else
	o=$2
fi

cc `cat $1 | awk '{ printf("-D"$1"="$2" "); }'` -march=native  -O3 -fomit-frame-pointer -ffast-math -DNDEBUG -fivopts -funsafe-loop-optimizations engine.c.o io.c.o mhash.c.o movegen.c.o p8.c.o plist.c.o poker.c.o ai/human.c.o ai/first.c ai/random.c ai/shedder.c.o ai/monte.c.o ai/judge.c.o ai/stacked.c.o ai/draw.c.o ai/cheat.c.o ai/mmcheat.c.o -std=gnu11 -D_GNU_SOURCE -lpthread -lrt -lm -s -o $o
