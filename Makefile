CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lcrypt

permutepass: permutepass.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@ 

clean:
	rm -f permutepass
