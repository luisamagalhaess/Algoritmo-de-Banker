banker:
	gcc -Wall -Wextra -Wpedantic -std=c11 banker.c -o banker

run:
	./banker 10 5 7

clean:
	rm -f banker

.PHONY: banker run clean