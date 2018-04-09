#!/bin/makefile ....
chip8: chip8.c
	zcc +zx -W -Wall -O3 -o chip8.bin chip8.c -lndos
	bin2tap chip8.bin chip8.tap
	tapmaker roms/PONG chip8.tap
	tapmaker roms/TICTAC chip8.tap
	tapmaker roms/BRIX chip8.tap
	tapmaker roms/MISSILE chip8.tap
	-rm zcc_opt.def

clean:
	-rm chip8.bin
	-rm chip8.tap
	-rm zcc_opt.def

all: clean chip8

#.phony clean all
	
