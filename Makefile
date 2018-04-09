#!/bin/makefile ....
CFLAGS=+zx -v -W -Wall -O0 -Ddeunavez -zorg=32768
LFLAGS=-lndos

bin: emu.bin cinta

# Cuando sepa separarlos se compilara algo asi:
chip8.bin : chip8.c chip8.c
	zcc $(CFLAGS) -c -o speccy.bin speccy.c 
	-rm zcc_opt.def

speccy.bin : speccy.c speccy.h
	zcc $(CFLAGS) -c -o chip8.bin chip8.c  
	-rm zcc_opt.def

emu.binbin : chip8.bin speccy.bin
	zcc $(LFLAGS) -o emu.bin chip8.bin speccy.bin 

# Pero por ahora lo hacemos de una vez.
emu.bin : chip8.c chip8.h speccy.c speccy.h
	zcc $(CFLAGS) -o chip8.bin chip8.c $(LFLAGS)

cinta:
	#bin2tap emu.bin chip8.tap
	z88dk-appmake +zx -b chip8.bin -o chip8.tap --org 32768
	# Estas estas delante por que son las que fallan.
	tapmaker roms/MERLIN chip8.tap
	tapmaker roms/BLINKY chip8.tap
	# Todas las Rom Chip8 que tengo	
	tapmaker roms/15PUZZLE chip8.tap
	tapmaker roms/AIRPLANE chip8.tap
	tapmaker roms/BLINKY chip8.tap
	tapmaker roms/BLITZ chip8.tap
	tapmaker roms/BREAKOUT chip8.tap
	tapmaker roms/BRIX chip8.tap
	tapmaker roms/C8PIC chip8.tap
	tapmaker roms/CAVE chip8.tap
	tapmaker roms/CONNECT4 chip8.tap
	tapmaker roms/GUESS chip8.tap
	tapmaker roms/HIDDEN chip8.tap
	tapmaker roms/IBM chip8.tap
	tapmaker roms/INVADERS chip8.tap
	tapmaker roms/KALEID chip8.tap
	tapmaker roms/MAZE chip8.tap
	tapmaker roms/MERLIN chip8.tap
	tapmaker roms/MISSILE chip8.tap
	tapmaker roms/PONG chip8.tap
	tapmaker roms/PONG2 chip8.tap
	tapmaker roms/PONG3 chip8.tap
	tapmaker roms/PUZZLE chip8.tap
	tapmaker roms/PUZZLE2 chip8.tap
	tapmaker roms/SYZYGY chip8.tap
	tapmaker roms/TANK chip8.tap
	tapmaker roms/TETRIS chip8.tap
	tapmaker roms/TICTAC chip8.tap
	tapmaker roms/UFO chip8.tap
	tapmaker roms/VBRIX chip8.tap
	tapmaker roms/VERS chip8.tap
	tapmaker roms/WIPEOFF chip8.tap

clean:
	-rm *.bin
	-rm *.opt
	-rm *.err
	-rm chip8.tap
	-rm zcc_opt.def

all: clean bin 

#.phony clean all
	

