#!/usr/bin/python3

import csv
import sys
from random import randint

with open(sys.argv[1], 'r') as dna:
	dnareader = csv.reader(dna, delimiter='\t')
	for r in dnareader:
		print('%s\t%i' % (r[0], int(r[1]) + randint(-3,3)))
