/* Emulador de Chip-8 para ZX Spectrum    *
 * Desarrollado por Alvaro Alea Fernandez *
 * Distribuido con licencia GPL V2        */

/* Actualmente pido discrecion y no distribucion   *
 * de este codigo a cualquiera que lo vea          *
 * de igual manera pido que me cedan el (C) de     *
 * cualquier cambio, para evitar posible problemas *
 * con posteriores cambios de licencia             *
 * si no estas deacuerdo, no lo toques             */

/* La razon es sencilla: va a existir una version  *
 * comercial y otra GPL                            */

/* Este archivo contiene las Funcines que *
 * Dependen de la arquitectura            *
 * en este caso: Para ZX Spectrum         */


#include <spectrum.h>
#include <graphics.h>
#include <sound.h>
#include "speccy.h"
#include "chip8.h"

void updatedisplay(u8_t x,u8_t y, u8_t w, u8_t h)
{
	u8_t *scr;
	u8_t p,c,d;
	int cp;
	if ( ((y+h-1) & 0x1F) <x)          
		{ x=0; w=64;  }		   
	if ( ((y+h-1) & 0x1F) <y) 
		{ y=0; h=32; }
	x &= 0xFE;		
	for (d=y;d<(y+h);d++)
		for (c=x/2,cp=d*64+x;cp<(d*64+x+w);c++,cp++)
		{
			if (display[cp++]!=0)
				p = 0xE0;
			else 
				p = 0x00;
			if (display[cp]!=0)
				p |= 0x0E;
			scr=(u8_t *)( 0x4000+c+((d &0x10)<<7)+((d &0x01)<<10)+((d &0x0E)<<4) );
			*scr=p;
			scr+=0x100; *scr=p;
			scr+=0x100; *scr=p;
			//scr+=0x100; *scr=p;
		}
}

void updatedisplaymini(u8_t x,u8_t y, u8_t w, u8_t h)
{
	int c,d,x2,y2;
	x2=(x+w-1) & 0x3F; 
	if (x2<x)        
	{               
		x=0;w=64; 
	}		  	
	y2=(y+h-1) & 0x1F; 
	if (y2<y) 
	{
		y=0;h=32;
	}
	for (d=y;d<(y+h);d++)
		for (c=x;c<(x+w);c++)
			if (display[d*64+c]==1)
				plot(c,d);
			else
				unplot(c,d);
}

/* Esta rutina devuelve 0 si no se pulsa ninguna tecla             *
 * si se pulsa alguna, devuelve en los 3 bit de arriba la semifila *
 * y en los 5 de abajo la tecla/s                                  *
 * Probablemente hubiese sido mas facil hacerla entera en ASM      *
 * pero no esta el horno pa bollos y estoy DESESPERADO             *
 * Para mas info consultar Curso de C/M de MH pag 346              */

u8_t r;
u8_t mygetk2(void)
{
// Semifila de V a mayusculas
#asm
	ld a,1
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) return (r); 
// Semifila de G a A
#asm
	ld a,2
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0x20 ;return (r); }
// Semifila de T a Q
#asm
	ld a,4
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0x40 ;return (r); }
// Semifila de 5 a 1
#asm
	ld a,8
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0x60 ;return (r); }
// Semifila de 6 a 0
#asm
	ld a,16
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0x80 ;return (r); }
// Semifila de Y a P
#asm
	ld a,32
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |=0xA0 ;return (r); }
// Semifila de H a Enter
#asm
	ld a,64
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0xC0 ;return (r); }
// Semifila de B a espacio
#asm
	ld a,128
	cpl
	in a,(254)
	cpl
	and 31
	ld (_r),a
#endasm
if (r!=0) { r |= 0xE0 ;return (r); }
return (0);	
}

u8_t mygetk(void)
{
	switch (mygetk2())
	{
		case 0x00:
			return 0x00;
		case 0x61:
			return ('1');
		case 0x62:
			return ('2');
		case 0x64:
			return ('3');
		case 0x68:
			return ('4');
		case 0x70:
			return ('5');
		case 0x90:
			return ('6');
		case 0x88:
			return ('7');
		case 0x84:
			return ('8');
		case 0x82:
			return ('9');
		case 0x81:
			return ('0');
		
		case 0x41:
			return ('Q');
		case 0x42:
			return ('W');
		case 0x44:
			return ('E');
		case 0x48:
			return ('R');
		case 0xA2:
			return ('O');
		case 0xA1:
			return ('P');
		
		case 0x21:
			return ('A');
		case 0x22:
			return ('S');
		case 0x24:
			return ('D');
		case 0x28:
			return ('F');
		case 0xC2:
			return ('L');
		
		case 0x02:
			return ('Z');
		case 0x04:
			return ('X');
		case 0x08:
			return ('C');
		case 0x10:
			return ('V');
		case 0xF0:
			return ('B');
		case 0xE4:
			return ('M');
		
		default:
			//printf("tecla =%x ",mygetk2());
			return (' ');
	}
}

u8_t mygetkey(void){ u8_t c; while ( (c=mygetk()) == 0 ); return (c); }

u8_t iskeypressed(u8_t k)
{
	if (mygetk()==key[k]) 
		return(1);
	else
		return (0);
}
			
u8_t waitanykey(void)
{
	u8_t c;
	u8_t ret=100;
//	printf("K\n");
	while (ret==100)
	{
        	c=mygetkey();
//		c='5';
		if (c==key[0] ) ret=0; 
		else if (c==key[1] ) ret=1; 
		else if (c==key[2] ) ret=2; 
		else if (c==key[3] ) ret=3; 
		else if (c==key[4] ) ret=4; 
		else if (c==key[5] ) ret=5; 
		else if (c==key[6] ) ret=6; 
		else if (c==key[7] ) ret=7; 
		else if (c==key[8] ) ret=8; 
		else if (c==key[9] ) ret=9; 
		else if (c==key[0xA] ) ret=0xA; 
		else if (c==key[0xB] ) ret=0xB; 
		else if (c==key[0xC] ) ret=0xC; 
		else if (c==key[0xD] ) ret=0xD;
		else if (c==key[0xE] ) ret=0xE; 
		else if (c==key[0xD] ) ret=0xF; 
	}
	return (ret);
}

u8_t menu(void)
{
	u8_t salir=0,c;
	debug=0;
	while (salir==0)
	{
   		printf("\x0C\x01\x20ZX Chip8 V0.2 \x01\x40\n"
		       "(C) 2005-2013 Alvaro Alea Fernandez\n"
   		       "Todos los derechos reservados.\n\n");
		printf("\x01\x20Menu Principal:\x01\x40\n\n");
		printf("Pulsa 1 para cargar una Rom\n");
		printf("Pulsa 2 para hacer un Reset\n");
		printf("Pulsa 3 para entrar en el Editor interno\n");
		printf("Pulsa 4 para continuar en modo DEBUG\n");
		//printf("Pulsa 5 para redefinir el Teclado\n");
		printf("\nPulsa 0 para continuar a la Emulacion.\n");
		switch (getkey())
		{
			case '1':
				loadrom();
				reset();
				break;
			case '2':
				reset();
				break;
			case '3':
				pc=0x200;
				sp=0;
				cls();
				salir=1;
			case '4':
				debug=1;
				salir=1;
				break;
			case '5':
				redefineteclas();
				break;
			case '0':
				salir=1;
		}
	}
	return (1);
}


void loadrom(void)
{
	int c;
	static u8_t font[16*5]=
   	{
	   0xf0,0x90,0x90,0x90,0xf0, 0x20,0x60,0x20,0x20,0x70,
	   0xf0,0x10,0xf0,0x80,0xf0 ,0xf0,0x10,0xf0,0x10,0xf0,
	   0x90,0x90,0xf0,0x10,0x10 ,0xf0,0x80,0xf0,0x10,0xf0,
	   0xf0,0x80,0xf0,0x90,0xf0 ,0xf0,0x10,0x20,0x40,0x40,
	   0xf0,0x90,0xf0,0x90,0xf0 ,0xf0,0x90,0xf0,0x10,0xf0,
	   0xf0,0x90,0xf0,0x90,0x90 ,0xe0,0x90,0xe0,0x90,0xe0,
	   0xf0,0x80,0x80,0x80,0xf0 ,0xe0,0x90,0x90,0x90,0xe0,
	   0xf0,0x80,0xf0,0x80,0xf0 ,0xf0,0x80,0xf0,0x80,0x80,
   	};
	struct zxtapehdr cabezera;

	// iniciamos el contenido de la ROM.
	for(c=0;c<0x800;c++)
		mem[c]=0;
   	for (c=0;c<(16*5);c++)
   		mem[FONT_OFFSET+c]=font[c];
		
   	// cargamos la ROM
   	printf("\nCargando Rom...\n");
   	tape_load_block(&cabezera,17,255); //la cabezera de las narices.
   	// mem[0x20A]=0; //tonteria para que el printf salga bien.
	// cabezera.name[0xA]=0;
     printf("Encontrado:%s\n",cabezera.name);
     printf("Direccion:%u\n",cabezera.address);
     printf("Longuitud:%u\n",cabezera.length);
     printf("Offset:%u\n",cabezera.offset);
     for(c=1024;c!=0;c--);
	tape_load_block((unsigned int)mem+0x200,cabezera.length,0);
/*
    // agregar LET org=NOT PI tras el clear y luego
    // de echo no deberia hacer falta
    // 100 LOAD "" CODE org : STOP
 	printf("ORG=%u\n",mem);
 	zx_setint("org",(unsigned int)mem+0x200);
     zx_goto(100);	
*/
     for(c=1024;c!=0;c--);
} 

void pita (void)
{
//	   bit_frequency(0.01,440);
	   bit_beep(400,10);
}

void border(u8_t n)
{
/* otra forma   pop de  	; direccion de retorno
 * 		pop hl  	; parametro de la funcion
 * 		ld a,(hl)
 * 		push hl 	; dejamos la pila como esta.
 * 		push de
 * 		dec a   	; el manual de CM no dice nada de que
 * 		call 8859	; se llame con a-1 pero bueno...
 * 		ret
 */
#asm
	ld      hl,2    ;const
        add     hl,sp
        inc     (hl)
        ld      a,(hl)
	dec 	a
	call 8859	; BORD-1 
	ret
#endasm
//zx_border(n);
}

void initsystem(void)
{
	u8_t *scr;
	int c;
	for (c=0;c<768;c++)
	{
		scr=(u8_t *)(0x5800+c);
		*scr=PAPER_BLACK+INK_WHITE+BRIGHT;
	}
	zx_border(0);
	
}

u8_t printstate(u8_t showall)
{
	static int old_i,old_sp;
	u8_t b,c,d;

	if (showall==1) 
		old_i=old_sp=-1;
	
	// imprimir SP y los 16 valores del stack con un signo en su posicion.
	if (sp!=old_sp)
	{
		old_sp=sp;
		printf("\x16\x20\x44Stack");
		for(c=0;c<0xF;c++)
			printf("\x16%c\x44%s%x : %x  ",c+34,(c==sp?">":" "),
				c,stack[c]);
	}
		
	// imprimir I y los 16 siguientes valores, tb como grafico.
	if (i!=old_i)
	{
		old_i=i;
		printf("\x16\x20\x4E I=%x   ",i);
		for(c=0;c<0xF;c++)
		{
			b=mem[i+c];
			printf("\x16%c\x4E %x : %x  \x16%c\x58",c+34,i+c,b,c+34);
			for(d=0;d<8;d++)
			{
				printf("%s",((b&0x80)==0?"_":"X"));
				b=b<<1;
			}
		}
	}

	// imprimir pc y los 16 valores siguientes.
	printf("\x16\x2A\x20PC=%x   ",pc);
	for(c=0;c<12;c++)
	{
		printf("\x16%c\x20%x=%x   \x16%c\x29",0x2C+c,pc+(2*c),(mem[pc+(2*c)]<<8) + mem[pc+(2*c)+1],0x2c+c);
		decode((mem[pc+(2*c)]<<8) + mem[pc+(2*c)+1]);
	}		
	
	
	// imprimir los 16 valores de v
	printf("\x16\x32\x42v[0]=%x ",reg[0]);
	printf("\x16\x33\x42v[1]=%x ",reg[1]);
	printf("\x16\x34\x42v[2]=%x ",reg[2]);
	printf("\x16\x35\x42v[3]=%x ",reg[3]);
	printf("\x16\x36\x42v[4]=%x ",reg[4]);
	printf("\x16\x37\x42v[5]=%x ",reg[5]);

	printf("\x16\x32\x4bv[6]=%x ",reg[6]);
	printf("\x16\x33\x4bv[7]=%x ",reg[7]);
	printf("\x16\x34\x4bv[8]=%x ",reg[8]);
	printf("\x16\x35\x4bv[9]=%x ",reg[9]);
	printf("\x16\x36\x4bv[A]=%x ",reg[0xA]);
	printf("\x16\x37\x4bv[B]=%x ",reg[0xB]);
	
	printf("\x16\x32\x54v[C]=%x ",reg[0xC]);
	printf("\x16\x33\x54v[D]=%x ",reg[0xD]);
	printf("\x16\x34\x54v[E]=%x ",reg[0xE]);
	printf("\x16\x35\x54v[F]=%x ",reg[0xF]);
	// imprimir el timer
	printf("\x16\x36\x54Dtimer=%d  ",dtimer);
	// imprimir el stimer
	printf("\x16\x37\x54Stimer=%d  ",stimer);
	
}

void redefineteclas(void)
{

}
