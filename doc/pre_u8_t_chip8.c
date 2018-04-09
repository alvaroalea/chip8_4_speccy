/* Emulador de Chip-8 para ZX Spectrum    *
 * Desarrollado por Alvaro Alea Fernandez *
 * Distribuido con licencia GPL V2        */

#include <stdlib.h>
#include <stdio.h>
#include <spectrum.h>
#include <graphics.h>
#include <sound.h>

// realmente estos valores deberian ser de memoria y no a parte
// tampoco tengo en memoria la bios-128 para introducir programas
u8_t mem[0x1000]; 
u8_t reg[0x10];   // mem 0x0 a mem 0xF
int stack[0x10];  // mem 0x16 a mem 0x36
u8_t sp,stimer,dtimer; //sp deberia ser tb un int
int  pc,i;
int  display[64*32]; 
#define FONT_OFFSET 0x38
// si esta linea no existe (o esta comentada) el display se actualiza en cuanto cambia
// si existe se actualiza estilo TV, 60 veces por sec.
//#define UPDATE_BY_FRAMES

void updatedisplay(int x, int y, int w, int h)
{
	int c,d,x2,y2;
//	printf("actualizo x=%x ,y=%x de %x x %x\n",x,y,w,h);
	x2=(x+w-1) & 0x3F; // como los sprites son cicliclos, es posible
	if (x2<x)          // tener que dibujar 2 o 4 cuatro trocitos en
	{                  // la pantalla, si eso pasa, dibujo toda una
		x=0;w=64;  // tira que abarque los 2 trozos
	}		   // o la pantalla entera.	
	y2=(y+h-1) & 0x1F; 
	if (y2<y) 
	{
		y=0;h=32;
	}
#define BIG_PIXEL		
	for (d=y;d<(y+h);d++)
		for (c=x;c<(x+w);c++)
		{
			if (display[d*64+c]==1)
			{
#ifndef BIG_PIXEL
	plot(c,d);
#else
				plot(c*2,d*2);
				plot(c*2+1,d*2);
				plot(c*2,d*2+1);
				plot(c*2+1,d*2+1);
#endif
			}
			else
			{
#ifndef BIG_PIXEL
				unplot(c,d);
#else
				unplot(c*2,d*2);
				unplot(c*2+1,d*2);
				unplot(c*2,d*2+1);
				unplot(c*2+1,d*2+1);
#endif
			}
		}
}

void cls(void)
{
	int c;
	for (c=0;c<(64*32);c++) display[c]=0;
#ifndef UPDATE_BY_FRAMES
	updatedisplay(0,0,64,32);
#endif
//	printf("vamos bien\n");
}

char drawsprite(char x, char y, char size)
{
	char ret=0;
	char c,d,dat,pr,px,po;
	int dir;
	for (c=0;c<size;c++)
	{
		dat=mem[i+c];
		for (d=0;d<8;d++)
		{
			dir= ((y+c)%32)*64 + ((x+d)%64); //probably mas rapido con rots y mask
//			dir= ((y+c) & 0x1F << 6 ) + ((x+d) & 0x3F);	
			pr=display[dir];
			px=( dat & 0x80) >> 7;
			dat=dat << 1;
			po= pr ^ px;
			if ((pr==1) && (po==0)) 
				ret=1;
			display[dir]=po;
		}
	}
#ifndef UPDATE_BY_FRAMES
	updatedisplay(x,y,8,size);
#endif
	return (ret);	
}

char iskeypressed(char key)
{
	char ret=0;
	char c;
//	printf("k=%x\n",key);
	c=(char)getk();
	if (c=='0' && key==0) ret=1; 	
	if (c=='1' && key==1) ret=1; 
	if (c=='2' && key==2) ret=1; 
	if (c=='3' && key==3) ret=1; 
	if (c=='4' && key==4) ret=1; 
	if (c=='5' && key==5) ret=1; 
	if (c=='6' && key==6) ret=1; 
	if (c=='3' && key==7) ret=1; 
	if (c=='8' && key==8) ret=1; 
	if (c=='9' && key==9) ret=1; 
	if (c=='a' && key==0xA) ret=1; 
	if (c=='b' && key==0xB) ret=1; 
	if (c=='c' && key==0xC) ret=1; 
	if (c=='d' && key==0xD) ret=1; 
	if (c=='e' && key==0xE) ret=1; 
	if (c=='f' && key==0xF) ret=1; 
//	if (c!=0) printf("BING %x=%x\n",key,c);
	return (ret);
}

char mygetkey(void){ char r; while ((r=getk())==0); return (r); }
	
			
char waitanykey()
{
	char c;
	char ret=100;
//	printf("K\n");
	while (ret==100)
	{
//        	c=(char)getkey();
        	c=mygetkey();
//		c='5';
		if (c=='0' ) ret=0; 
		if (c=='1' ) ret=1; 
		if (c=='2' ) ret=2; 
		if (c=='3' ) ret=3; 
		if (c=='4' ) ret=4; 
		if (c=='5' ) ret=5; 
		if (c=='6' ) ret=6; 
		if (c=='7' ) ret=7; 
		if (c=='8' ) ret=8; 
		if (c=='9' ) ret=9; 
		if (c=='A' ) ret=0xA; 
		if (c=='B' ) ret=0xB; 
		if (c=='C' ) ret=0xC; 
		if (c=='D' ) ret=0xD; 
		if (c=='E' ) ret=0xE; 
		if (c=='F' ) ret=0xF;
	}
	return (ret);
}

void reset(void)
{
	cls();
	pc=0x200;
	sp=0;  // ¿necesario?
	return (0);
}

void loadrom(void)
{
	int c;
	static char font[16*5]=
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
   	clg();
   	printf("ZX Chip8 V0.1 (C) 2005 Alvaro Alea Fernandez\n");
   	printf("Distribuido con licencia GPL V2\n\n");
//   	printf("Char=%x int=%x\n",sizeof(char),sizeof(int));
   	// cargamos la memoria con el fontmap
   	for (c=0;c<(16*5);c++)
   		mem[FONT_OFFSET+c]=font[c];
   
   	// cargamos la ROM
   	printf("Cargando Rom...[Pulse Una Tecla]\n");
   	getkey();
   	tape_load_block(&mem[0x200],17,255); //la cabezera de las narices.
   	tape_load_block(&mem[0x200],(mem[0x20C]<<8)+( mem[0x20B] & 0xFF) ,0);
   	clg(); 
   	reset();
}

int do_cpu(void); // Prototipo: la funcion mas tarde.

int main(void)
{
   int c,d,quit=1;
   loadrom();
   while (quit)
   {
	   for (c=0;c<20;c++) // calculado a ojo que el spectrum hace 20 inst en un frame (60hz)
	   {
		   if (do_cpu())
			quit=1;
	   if (stimer>0)
//		   bit_frequency(0.01,440);
		   bit_beep(400,10);
	   if (getk()=='l') loadrom();
	   if (getk()=='r') reset();
#ifdef UPDATE_BY_FRAMES
	   updatedisplay(0,0,64,32);
#endif
	   }
   	   if (dtimer>0)     // hay formas mejores de hacer esto, p.e. con interrupciones.
		   dtimer--;
	   if (stimer>0)
	   	   stimer--;
  //	   printf("ciclo\n");
   }
   return(0);
}
		   

/* El nucleo del la CPU del Chip8 */
/* Ejecuta una unica instruccion  */
/* 34 instrucciones de 2bytes     */
int do_cpu(void)   
{
	int inst,in2,r,c,zz;
//	int  in1,x,y;
	char in1,x,y;
//	static int debug=0;	
	inst=((mem[pc++] & 0xFF)<<8) + (mem[pc++] & 0xFF );
	in1=(inst >>12) & 0xF;
	x = (inst >> 8) & 0xF;
	y = (inst >> 4) & 0xF;
	zz = inst & 0x00FF;
/*
//if (getk()=='p') debug=1;
//if (getk()=='o') debug=0;
	
if (debug==1)
{
//	printf ("%c[%u;%uH",27,17,0);
	printf ("pc=%x(%x,%x) inst=%x (in1=%x, x=%x, y=%x, zz=%x)\n",pc-2,mem[pc-2],mem[pc-1],inst,in1,x,y,zz);	
	printf ("A 0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x  i=%x\n",reg[0],reg[1],reg[2],reg[3],reg[4],reg[5],reg[6],reg[7],i); 
	printf ("A 8=%x 9=%x A=%x B=%x C=%x D=%x E=%x F=%x sp=%x\n",reg[8],reg[9],reg[10],reg[11],reg[12],reg[13],reg[14],reg[15],sp); 
	getkey();
}
*/
	switch (in1)
	{
		case 0x2: // call xyz
			//printf("estoy en %x y llamo...(guardo en %d)\n",pc,sp);
			stack[sp]=pc;
			sp= ++sp & 0x0F;  // el stack es de solo 16 niveles.
			pc= inst & 0xFFF; 
			
			break;           // :-) 
		case 0x1: // jmp xyz
			pc=inst & 0x0FFF;
			break;
		case 0x3: // skeq :  salta si r(x) igual a yz 
			if (reg[x] == zz) 
				pc+=2;
			break;
		case 0x4: //skne : salta si r(x) distinto de yz
			if (reg[x] != zz) 
				pc+=2;
			break;
		case 0x5: //skeq : salta si r(x) igual a r(y)
			if (reg[x] == reg[y])
				pc+=2; 
			break;
		case 0x6:  // ld 
		        reg[x]=zz;
			break;
		case 0x7:
		        reg[x]=reg[x]+zz;
			break;
		case 0x9:
			if (reg[x] != reg[y]) 
				pc+=2;
			break;
		case 0xa: // ld i,
			i=inst & 0xFFF;
			break;
		case 0xb: // jmi xxx
			pc=(inst & 0xFFF) + reg[0];
			break;
		case 0xc: // ldrnd
			reg[x]= rand() % zz;
			break;
		case 0xd: // draw sprite 
			reg[0xF]=drawsprite( reg[x], reg[y], inst & 0xF);
			break;
		case 0x0: // instruccion extendida
			switch (zz)
			{
				case 0xE0: 
					cls();
					break;
				case 0xEE: // ret
					sp = --sp & 0xF;
					pc=stack[sp] & 0xFFF;
					//printf("vuelvo a %x... (pillo de %d)\n",pc,sp);
					break;
			}
			break;
		case 0x8: // instruccion extendida
			in2= inst & 0xF;
			switch (in2)
			{
				case 0x0: // ld
					reg[x] = reg[y];
					break;
				case 0x1: // or
					reg[x] |= reg[y];
					break;
				case 0x2: // and
					reg[x] &= reg[y];
					break;
				case 0x3: // xor
					reg[x] ^= reg[y];
					break;
				case 0x4: // addc
					r=reg[x] + reg[y];
					reg[x] = r & 0xFF;
					if ((r & 0xF00) !=0) 
						reg[0xF]=1;
					break;
				case 0x5: // cmp 
					r=reg[x] - reg[y];
			 		if (r < 0) 
						reg[0xF]=1;
					break;
				case 0x6: // srl
					reg[0xf]=reg[x] & 0x1;
					reg[x] = reg[x] >> 1;
					break;
				case 0x7: // subc 
					r=reg[x] - reg[y];
					reg[x] = r & 0xFF;
					if (r < 0) 
						reg[0xF]=1;
					break;
				case 0xe: // srr
					reg[0xF]=( reg[x] & 0x70) >> 8;
					reg[x] = (reg[x] << 1) & 0xFF;
					break;
			}
			break;
		case 0xe: // instruccion extendida
			if  (zz == 0x9E ) // skipifkey
			{
				if ( iskeypressed(reg[x]) )
					pc+=2;
			}
			else if (zz == 0xA1) // skipifnokey
			{
				if ( ! iskeypressed(reg[x]) )
					pc+=2;
			}
			break;
		case 0xf: // instruccion extendida
			switch (zz)
			{
				case 0x07: // getdelay
					reg[x]=dtimer;
					break;
				case 0x0A: // waitkey 
					reg[x]=waitanykey();
					break;
				case 0x15: // setdelay
					dtimer=reg[x];
					break;
				case 0x18: // setsound
					stimer=reg[x];
					break;
				case 0x1E: // add i,
					i+=reg[x];
					break;
				case 0x29: // font i  
					i= FONT_OFFSET + (reg[x]*5);
					break;
				case 0x33: // bcd
					mem[i]=reg[x]/100;
					mem[i+1]=(reg[x]/10) % 10;
					mem[i+2]=(reg[x]) % 10;
//					printf("%x=%x-%x-%x\n",reg[x],reg[x]/100,(reg[x]/10) % 10,(reg[x]) % 10);
					break;
				case 0x55: // str
					for (c=0;c<=x;c++)
						mem[i++]=reg[c];
					break;
				case 0x65: // ldr
					for (c=0;c<=x;c++)
						reg[c]=mem[i++]; 
					break;
			}
			break;
	}
/*
if (debug==1)
{
	printf ("pc=%x(%x,%x) inst=%x (in1=%x, x=%x, y=%x, zz=%x)\n",pc-2,mem[pc-2],mem[pc-1],inst,in1,x,y,zz);	
	printf ("D 0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x  i=%x\n",reg[0],reg[1],reg[2],reg[3],reg[4],reg[5],reg[6],reg[7],i); 
	printf ("D 8=%x 9=%x A=%x B=%x C=%x D=%x E=%x F=%x sp=%x\n",reg[8],reg[9],reg[10],reg[11],reg[12],reg[13],reg[14],reg[15],sp); 
	getkey();
}
*/
	return (0);
}
	
