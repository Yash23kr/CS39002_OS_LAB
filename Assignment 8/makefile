all: mmu.c master.c sched.c process.c
	gcc -o mmu mmu.c -g -fsanitize=address
	gcc -o master master.c -fsanitize=address -g
	gcc -o sched sched.c -fsanitize=address -g
	gcc -o process process.c -fsanitize=address -g

clean:
	rm -f mmu master sched process