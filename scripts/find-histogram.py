#!/usr/bin/env python

import sys

if (len(sys.argv) != 3):
	print "syntax: %s <filename> <bucketsize>" % (sys.argv[0])
	sys.exit()


lines = open(sys.argv[1]).read().split("\n")
bsize = int(sys.argv[2]) # bucket size

# max bucket value (= num of buckets)
mmax = 0
for i in range(0, len(lines)):
	if (len(lines[i]) > 0):
		m = int(lines[i]) / bsize
		mmax = max(mmax,m)

# count terms per bucket
a = [0]*(mmax+1)
for i in range(0, len(lines)):
	if (len(lines[i]) > 1):
		m = int(lines[i]) / bsize
		a[m] += 1


# print buckets
for i in range(0, len(a)):
	if (a[i] > 0):
		print "%d %d" % (i*bsize, a[i])
