#!/bin/sh
if test $# -gt 0
then
#make
gcc  -o Output $1 ./Library/ThreadManipulation.o ./Library/ThreadQueue.o
#echo "$0 $1"
fi

