/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                 MSX1.c                                 **/
/**                                                                        **/
/** This file contains the MSX-1 and Coleco ADAM implementation            **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include "CHIP8.h"
#include "VDP.h"

#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <stdlib.h>

#ifdef MSX                                      /* Define the VDP I/O port  */
#define _VDP_CTRL_PORT  99h                     /* addresses                */
#define _VDP_DATA_PORT  98h
#else
#define _VDP_CTRL_PORT  0bfh
#define _VDP_DATA_PORT  0beh
#endif

#define CHRTAB          0x1800                  /* name table               */
#define CHRGEN          0x0000                  /* pattern generator        */
#define COLTAB          0x2000                  /* colour table             */
#define SPRTAB          0x3800                  /* sprite pattern generator */
#define SPRGEN          0x1d00                  /* sprite attribute table   */

#define _CHRTAB         1800h                   /* same for inline assembly */
#define _CHRGEN         0000h                   /* routines                 */
#define _COLTAB         2000h
#define _SPRTAB         3800h
#define _SPRGEN         1d00h

static byte picture[16384];                     /* background picture data  */
                                                /* old VRAM contents are    */
                                                /* here as well             */
static byte sync=1;                             /* if 0, do not sync to VDP */
                                                /* clock                    */
static byte uperiod=1;                          /* number of interrupts per */
                                                /* screen update            */

/****************************************************************************/
/* Turn sound on                                                            */
/****************************************************************************/
void chip8_sound_on (void)
{
#ifdef MSX
 outp (0xa0,7);
 outp (0xa1,0x3e);
#else
 outp (0xff,0x90);
#endif
}

/****************************************************************************/
/* Turn sound off                                                           */
/****************************************************************************/
void chip8_sound_off (void)
{
#ifdef MSX
 outp (0xa0,7);
 outp (0xa1,0x3f);
#else
 outp (0xff,0x9f);
#endif
}

/****************************************************************************/
/* Initialise the sound chip                                                */
/****************************************************************************/
static void init_sound (void)
{
 chip8_sound_off ();
#ifdef MSX
 outp (0xa0,0);                                 /* set frequency divider to */
 outp (0xa1,0);
 outp (0xa0,1);
 outp (0xa1,1);
 outp (0xa0,8);                                 /* maximum volume           */
 outp (0xa1,0x0f);
#else
 outp (0xff,0x80);                              /* set frequency divider    */
 outp (0xff,0x12);
#endif
}

/****************************************************************************/
/* Update the display                                                       */
/****************************************************************************/
static void update_display (void)
{
#asm
        global _chip8_display
        ld      c,_VDP_CTRL_PORT
        ld      de,_SPRTAB+4000h                ; set write address to
        out     (c),e                           ; sprite attribute table
        out     (c),d
        dec     c
        ld      hl,_chip8_display
        ld      d,8                             ; 8 columns, 8 pixels wide
1:      ld      e,16                            ; 16 pixels high
2:      ld      a,(hl)                          ; get first pixel
        inc     hl                              ; increment pointer
        and     80h                             ; mask out unused bits
        ld      b,a                             ; save it
        ld      a,(hl)                          ; get second pixel
        inc     hl                              ; increment pointer
        and     40h                             ; mask out unused bits
        or      b                               ; OR with previous pixel
        ld      b,a                             ; save it
        ld      a,(hl)                          ; ...
        inc     hl
        and     20h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     10h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     08h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     04h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     02h
        or      b
        ld      b,a
        ld      a,(hl)
        and     01h
        or      b                               ; ...
        out     (c),a                           ; update VRAM
        push    de
        ld      de,57                           ; update pointer to next row
        add     hl,de
        pop     de
        dec     e
        jp      nz,2b                           ; update next row
        push    de
        ld      de,-16*64+8                     ; update next column
        add     hl,de
        pop     de
        dec     d
        jp      nz,1b

        ld      hl,_chip8_display               ; now update lower half of
        ld      de,64*16                        ; display
        add     hl,de
        ld      d,8
1:      ld      e,16
2:      ld      a,(hl)
        inc     hl
        and     80h
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     40h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     20h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     10h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     08h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     04h
        or      b
        ld      b,a
        ld      a,(hl)
        inc     hl
        and     02h
        or      b
        ld      b,a
        ld      a,(hl)
        and     01h
        or      b
        out     (c),a
        push    de
        ld      de,57
        add     hl,de
        pop     de
        dec     e
        jp      nz,2b
        push    de
        ld      de,-16*64+8
        add     hl,de
        pop     de
        dec     d
        jp      nz,1b
#endasm
}

/****************************************************************************/
/* The library version of rand() is terribly slow on a Z80 and we don't     */
/* really need good precision                                               */
/****************************************************************************/
#asm
        psect   bss
old_rnd:defb    0
        psect   text
        global  _rand
_rand:  ld      a,r                             ; get refresh register
        ld      b,a                             ; save it
        ld      a,(old_rnd)                     ; get previous random value
        add     a,b                             ; add refresh register
        ld      (old_rnd),a                     ; save it
        ld      l,a                             ; return value in hl. upper
        ld      h,a                             ; byte should be unused
        ret
#endasm

/****************************************************************************/
/* Update CHIP8 keyboard status                                             */
/****************************************************************************/
static void check_keys (void)
{
 byte i;
#ifdef MSX
 static byte joy_data[16]=
 { 0,3,9,0, 5,2,8,0, 7,4,10,0, 0,0,0,0 };
 static byte cursor_data[16]=
 { 0,5,3,2, 9,8,0,0, 7,0,4,0, 10,0,0,0 };
 for (i=0;i<16;++i) chip8_keys[i]=0;
 outp (0xaa,7);
 if ((inp(0xa9)&4)==0)
 {
  chip8_running=0;
  return;
 }
 outp (0xaa,6);
 if ((inp(0xa9)&0x20)==0)
  chip8_reset ();
 outp (0xa0,15);
 outp (0xa1,0x03);
 outp (0xa0,14);
 i=inp (0xa2);
 outp (0xa0,15);
 outp (0xa1,0x4c);
 outp (0xa0,14);
 i&=inp (0xa2);
 if ((i&0x30)!=0x30) chip8_keys[5]=1;
 i=joy_data[(~i)&0x0f];
 if (i) chip8_keys[i-1]=1;
 outp (0xaa,0);
 i=inp(0xa9);
 if ((i&0x03)!=0x03) chip8_keys[1]=1;
 if ((i&0x04)==0) chip8_keys[2]=1;
 if ((i&0x08)==0) chip8_keys[3]=1;
 if ((i&0x10)==0) chip8_keys[12]=1;
 outp(0xaa,1);
 i=inp(0xa9);
 if ((i&0x80)==0) chip8_keys[14]=1;
 if ((i&0x20)==0) chip8_keys[13]=1;
 if ((i&0x10)==0) chip8_keys[12]=1;
 if ((i&0x08)==0) chip8_keys[3]=1;
 if ((i&0x04)==0) chip8_keys[2]=1;
 outp(0xaa,2);
 i=inp(0xa9);
 if ((i&0x40)==0) chip8_keys[7]=1;
 if ((i&0x08)==0) chip8_keys[15]=1;
 if ((i&0x04)==0) chip8_keys[11]=1;
 outp(0xaa,3);
 i=inp(0xa9);
 if ((i&0x80)==0) chip8_keys[7]=1;
 if ((i&0x40)==0) chip8_keys[4]=1;
 if ((i&0x08)==0) chip8_keys[14]=1;
 if ((i&0x04)==0) chip8_keys[6]=1;
 if ((i&0x02)==0) chip8_keys[9]=1;
 if ((i&0x01)==0) chip8_keys[11]=1;
 outp(0xaa,4);
 i=inp(0xa9);
 if ((i&0x80)==0) chip8_keys[13]=1;
 if ((i&0x40)==0) chip8_keys[4]=1;
 if ((i&0x20)==0) chip8_keys[6]=1;
 if ((i&0x10)==0) chip8_keys[5]=1;
 if ((i&0x08)==0) chip8_keys[10]=1;
 if ((i&0x04)==0) chip8_keys[0]=1;
 if ((i&0x02)==0) chip8_keys[9]=1;
 if ((i&0x01)==0) chip8_keys[8]=1;
 outp(0xaa,5);
 i=inp(0xa9);
 if ((i&0x80)==0) chip8_keys[10]=1;
 if ((i&0x20)==0) chip8_keys[0]=1;
 if ((i&0x10)==0) chip8_keys[5]=1;
 if ((i&0x08)==0) chip8_keys[15]=1;
 if ((i&0x01)==0) chip8_keys[8]=1;
 outp(0xaa,8);
 i=inp(0xa9);
 if ((i&0x01)==0) chip8_keys[5]=1;
 i=cursor_data[((~i)>>4)&0x0f];
 if (i) chip8_keys[i-1]=1;
#else
 static byte keypad_table[16]=
 { 0,9,5,6,0,8,12,3,0,11,1,10,4,2,7,0 };
 static byte joypad_table[16]=
 { 0,3,7,4,9,0,10,0,5,2,0,0,8,0,0,0 };
 static byte keyboard_table[256]=
 {
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* 00 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* 10 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,13,12, 4,16, 0,     /* 20 */
   3, 2, 3, 4,13, 0, 0, 0,  0, 2, 0,15, 0, 0, 0, 0,     /* 30 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* 40 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,14, 0, 0, 0, 0,     /* 50 */
   0, 8, 0,12,10, 7,15, 0,  0, 5, 8, 9,10, 1,11, 6,     /* 60 */
   7, 5,14, 9, 0, 0,16, 6,  1, 0,11, 0, 0, 0, 0, 0,     /* 70 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* 80 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* 90 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* a0 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* b0 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* c0 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* d0 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,     /* e0 */
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0      /* f0 */
 };
 static byte joypad,keypad_1,keypad_2;
 for (i=0;i<16;++i) chip8_keys[i]=0;
 if ((i=bdos(6,0xff))!=0)
 {
#asm
  di
#endasm
  if (i==27)
  {
   chip8_running=0;
   return;
  }
  if (i==24) chip8_reset();
  i=keyboard_table[i];
  if (i) chip8_keys[i-1]=1;
 }
 outp (0xc0,0);
 joypad=~(inp(0xfc)&inp(0xff));
 outp (0x80,0);
 keypad_1=inp(0xfc)^0x40;
 keypad_2=inp(0xff)^0x40;
 outp (0xc0,0);
 joypad|=(keypad_1|keypad_2)&0x40;
 if ((joypad|keypad_1|keypad_2)&0x40)
  chip8_keys[5]=1;
 joypad&=15;
 keypad_1&=15;
 keypad_2&=15;
 if (keypad_1==4 || keypad_1==8)
  chip8_keys[5]=1;
 else
 {
  i=keypad_table[keypad_1];
  if (i) chip8_keys[i-1]=1;
 }
 if (keypad_2==4 || keypad_2==8)
  chip8_keys[5]=1;
 else
 {
  i=keypad_table[keypad_2];
  if (i) chip8_keys[i-1]=1;
 }
 i=joypad_table[joypad];
 if (i) chip8_keys[i-1]=1;
#endif
}

/****************************************************************************/
/* Update keyboard and display, sync emulation with VDP clock               */
/****************************************************************************/
void chip8_interrupt (void)
{
 static int ucount=1;
 if (!--ucount)
 {
  update_display ();
  check_keys ();
  ucount=uperiod;
 }
 if (sync) while ((vdp_read_status()&0x80)==0);
 else vdp_read_status();
}

/****************************************************************************/
/* Initialise the VDP                                                       */
/****************************************************************************/
static void init_vdp (void)
{
 static byte sprite_table[128]=
 {
  63,64,0,15,
  63,96,4,15,
  63,128,8,15,
  63,160,12,15,
  95,64,16,15,
  95,96,20,15,
  95,128,24,15,
  95,160,28,15,
  208
 };
 word i,j;
 /* turn display off and set registers */
 vdp_set_register (1,0x83);
#ifdef MSX
 vdp_set_register (8,0x08);
#endif
 vdp_set_register (0,0x02);
 vdp_set_register (2,0x06);
 vdp_set_register (3,0xff);
 vdp_set_register (4,0x03);
 vdp_set_register (5,0x3a);
 vdp_set_register (6,0x07);
 vdp_set_register (7,0xf0);
 /* write VRAM and get old contents */
 memcpy (picture+SPRGEN,sprite_table,sizeof(sprite_table));
 for (i=0;i<768;++i) picture[CHRTAB+i]=(byte)i;
 for (i=0,j=0;i<128;++i,j+=128)
 {
  vdp_read_video_ram (j,sprite_table,128);
  vdp_write_video_ram (j,picture+j,128);
  memcpy (picture+j,sprite_table,128);
 }
 vdp_read_status ();
 vdp_set_register (1,0xc3);
}

/****************************************************************************/
/* Reset the VDP to its original status                                     */
/****************************************************************************/
static void reset_vdp (void)
{
 static byte vdp_regs[]=
 {
#ifdef MSX
  0x00,0x70,0x00,0x80,0x01,0x36,0x07,0xf4,
  0x08,0x02,0x00,0x00,0x00,0x00,0x00,0x00
#else
  0x00,0xd0,0x0d,0x00,0x07,0x00,0x00,0x4f
#endif
 };
 byte i;
 vdp_set_register (1,0x83);
 vdp_write_video_ram (0,picture,16384);
 for (i=0;i<sizeof(vdp_regs);++i)
  if (i!=9) vdp_set_register (i,vdp_regs[i]);
 vdp_read_status ();
}

/****************************************************************************/
/* Compare ASCII-$ string with ASCII-zero one                               */
/****************************************************************************/
static int __strcmp (char *s1,char *s2)
{
 char s[20],i;
 /* Convert ASCII-$ string to ASCII-zero one */
 for (i=0;i<sizeof(s)-1 && s1[i]!='$';++i) s[i]=s1[i];
 s[i]='\0';
 /* Convert upper case characters to lower case ones */
 for (i=0;s2[i];++i)
  if (s2[i]>='A' && s2[i]<='Z') s2[i]+='a'-'A';
 /* Compare the strings */
 return strcmp (s,s2);
}

/****************************************************************************/
/* Parse command line options and start emulation                           */
/****************************************************************************/
int main (int argc,char *argv[])
{
 FILE *f;
 unsigned i,j,k,misparm;
 /* Hi-Tech C can't handle initialised arrays of pointers */
 char *options="h$bg$up$ip$s$";
 char *p;
 char *program=NULL,*bg="v8.bg";
 chip8_iperiod=16;
 printf ("Vision-8: Portable CHIP8 emulator\n"
         "Copyright (C) 1997  Marcel de Kogel\n");
 for (i=1,k=0;i<argc;++i)
 {
  misparm=0;
  if (*argv[i]!='-')
   switch (k++)
   {
    case 0:  program=argv[i];
             break;
    default: printf("Excessive filename '%s'\n",argv[i]);
             return 0;
   }
  else
  {    
   for (p=options,j=0;*p;p=strchr(p,'$')+1,j++)
    if (!__strcmp(p,argv[i]+1)) break;
   switch (j)
   {
    case 0:
     printf ("Usage: v8 [options] <filename>\n"
             "Available options are:\n"
             " -h          - Print this help page\n"
             " -bg <file>  - Select file to load as\n"
             "               background image [v8.bg]\n"
             " -up <value> - Select number of\n"
             "               interrupts per screen\n"
             "               update [1]\n"
             " -ip <value> - Select number of opcodes\n"
             "               per interrupt [16]\n"
             " -s <value>  - Select synchronisation\n"
             "               mode [1]\n"
             "               0 - Do not sync\n"
             "                   emulation\n"
             "               1 - Sync emulation to\n"
             "                   VDP clock\n");
             return 0;
    case 1:  i++;
             if (i<argc) bg=argv[i];
             else misparm=1;
             break;
    case 2:  i++;
             if (i<argc) uperiod=atoi(argv[i]);
             else misparm=1;
             break;
    case 3:  i++;
             if (i<argc) chip8_iperiod=atoi(argv[i]);
             else misparm=1;
             break;
    case 4:  i++;
             if (i<argc) sync=atoi(argv[i]);
             else misparm=1;
             break;
    default: printf ("Wrong option '%s'\n",argv[i]);
             return 1;
   }
   if (misparm)
   {
    printf("%s: Missing parameter\n",argv[i-1]);
    return 1;
   }
  }
 }
 if (!program)
 {
  puts ("No program name given\n");
  return 1;
 }
 printf ("Opening CHIP8 program %s... ",program);
 f=fopen (program,"rb");
 if (!f)
 {
  puts ("FAILED");
  return 1;
 }
 printf ("OK\n Reading... ");
 i=fread (chip8_mem+0x200,1,4096-0x200,f);
 fclose (f);
 if (i==0)
 {
  puts ("File is empty");
  return 1;
 }
 printf ("%d bytes loaded\nOpening background picture %s... ",i,bg);
 f=fopen (bg,"rb");
 i=0;
 if (f)
 {
  printf ("OK\n Reading... ");
  if (fread(picture+CHRGEN,1,6144,f)==6144)
   if (fread(picture+COLTAB,1,6144,f)==6144)
    i=1;
  fclose (f);
 }
 if (!i)
 {
  puts ("FAILED");
  memset (picture,0,16384);
 }
 else puts ("OK");
 printf ("Press any key to start emulation...");
 getch ();
 printf ("\n");
#asm
 di
#endasm
 init_vdp ();
 init_sound ();
 chip8 ();
 chip8_sound_off ();
 reset_vdp ();
#asm
 ei
#endasm
 return 0;
}
