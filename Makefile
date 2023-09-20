build:
	gcc Queue.h process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc Queue.h LinkedList.h PriorityQueue.h scheduler.c -o scheduler.out -lm
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
