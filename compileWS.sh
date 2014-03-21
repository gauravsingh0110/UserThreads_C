#!/bin/sh
if test $# -gt 0
then
#make
gcc $1 ./Library/ThreadManipulationWithStat.o ./Library/ThreadQueueWithStat.o -o OutputWithStat
#echo "$0 $1"
fi

