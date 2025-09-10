CC = gcc
CFLAGS =-ggdb -Imujs -Wall -Wswitch-enum
LDFLAGS =-lm

mujs/build/debug/libmujs.o:
	make -C mujs

jsc: main.c mujs/build/debug/libmujs.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf jsc
