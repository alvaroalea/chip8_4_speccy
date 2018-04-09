# Makefile for Vision-8 X-Windows

# Following #defines can be used:
# MITSHM		Use MIT SHM extensions
# DEBUG			Compile with builtin debugger
# HAVE_CLOCK		clock() is available on this machine

CFLAGS = -Wall -O2 -DMITSHM -DHAVE_CLOCK

OBJECTS = X.o CHIP8.o C8Debug.o

all: v8

clean:
	rm -f *.o *~

# Make sure the X11 library path is set correctly
v8: $(OBJECTS)
	$(CC) -L/usr/X11/lib -o v8 $(OBJECTS) -lXext -lX11

X.o: X.c CHIP8.h
CHIP8.o: CHIP8.c CHIP8.h
C8Debug.o: C8Debug.c CHIP8.h
