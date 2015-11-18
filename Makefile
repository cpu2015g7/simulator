CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

sim: sim.c fmul.c finv.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f sim
