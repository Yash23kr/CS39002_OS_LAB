all: mmu.c master.c sched.c process.c
	gcc -o mmu mmu.c
	gcc -o master master.c
	gcc -o sched sched.c 
	gcc -o process process.c
run: all
	./master 10 20 30
clean:
	rm -f mmu master sched process *.txt