#!/bin/bash

for i in *.c ai/[^f]*.c ; do gcc -march=native  -O3 -fomit-frame-pointer -ffast-math -DNDEBUG -flto -fwhole-program -fivopts -funsafe-loop-optimizations -c $i -o "$i.o" ; done
