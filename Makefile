all : ./Library/ThreadManipulation.o ./Library/ThreadQueue.o ./Library/ThreadManipulationWithStat.o ./Library/ThreadQueueWithStat.o

ThreadManipulation.o : ./Library/ThreadManipulation.c ./Library/ThreadManipulation.h
			gcc -c ./Library/ThreadManipulation.c

ThreadQueue.o : ./Library/ThreadQueue.c ./Library/ThreadQueue.h
			gcc -c ./Library/ThreadQueue.c

ThreadManipulationWithStat.o : ./Library/hreadManipulationWithStat.c ./Library/ThreadManipulationWithStat.h
			gcc -c ./Library/ThreadManipulationWithStat.c

ThreadQueueWithStat.o : ./Library/ThreadQueueWithStat.c ./Library/ThreadQueueWithStat.h
			gcc -c ./Library/ThreadQueueWithStat.c

clean :
	rm -rf ./Library/*.o Output OutputWithStat
