CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

sim: sim.c fmul.c finv.c f2i.c i2f.c if.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f sim
