#!/usr/bin/python3

import csv
import sys
from random import randint

dna1 = {}
with open(sys.argv[1], 'r') as dna:
    dnareader = csv.reader(dna, delimiter='\t')
    for r in dnareader:
        dna1[r[0]] = int(r[1])

with open(sys.argv[2], 'r') as dna:
    dnareader = csv.reader(dna, delimiter='\t')
    for r in dnareader:
        if randint(0,1) == 0:
            print('%s\t%i' % (r[0], int(r[1])))
        else:
            print('%s\t%i' % (r[0], dna1[r[0]]))
