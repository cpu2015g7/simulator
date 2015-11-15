CC = gcc
CFLAGS = -Wall -O3 -lm

sim: sim.c fmul.c finv.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm sim