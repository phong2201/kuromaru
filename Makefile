all: programA programB
programA: programA.c
	gcc -o programA programA.c -pthread
programB: programB.c
	gcc -o programB programB.c -pthread
clean:
	rm -rf programA programB

