/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                MSDOS.c                                 **/
/**                                                                        **/
/** This file contains the MS-DOS implementation                           **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include "CHIP8.h"

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static volatile byte timer_count;               /* 55Hz counter             */
static volatile byte keyb_status[256];          /* If 1, key is pressed     */
static void interrupt (*old_int8) (void);       /* Old timer interrupt      */
static void interrupt (*old_int9) (void);       /* Old keyboard interrupt   */
static int  sync=1;                             /* If 0, do not sync        */
                                                /* emulation                */
static byte uperiod=1;                          /* number of interrupts per */
                                                /* screen update            */

/****************************************************************************/
/* Turn sound on                                                            */
/****************************************************************************/
void chip8_sound_on (void)
{
 sound (500);
}

/****************************************************************************/
/* Turn sound off                                                           */
/****************************************************************************/
void chip8_sound_off (void)
{
 nosound ();
}

/****************************************************************************/
/* Update the display                                                       */
/****************************************************************************/
static void update_display (void)
{
 int i,j;
 byte far *p=MK_FP(0xb800,1448);                /* Get a pointer to VRAM    */
 byte *q=chip8_display;
 byte c;
 for (i=0;i<32;++i)                             /* 32 rows                  */
 {
  for (j=0;j<64;++j)                            /* 64 columns               */
  {
   c=*q^0x55;                                   /* background=cyan,         */
                                                /* foreground=magenta       */
   p[0]=p[0x2000]=p[80]=p[0x2000+80]=c;         /* draw a 4x4 box           */
   p++;                                         /* update pointers          */
   q++;
  }
  p+=16+80;
 }
}

/****************************************************************************/
/* Update CHIP8 keyboard status                                             */
/****************************************************************************/
static void check_keys (void)
{
 struct keyinfo_struct
 {
  byte code,key;
 };
 static struct keyinfo_struct keyinfo[]=
 {
  { 0x02,0x01 }, { 0x03,0x02 }, { 0x04,0x03 }, { 0x05,0x0c },
  { 0x10,0x04 }, { 0x11,0x05 }, { 0x12,0x06 }, { 0x13,0x0d },
  { 0x1e,0x07 }, { 0x1f,0x08 }, { 0x20,0x09 }, { 0x21,0x0e },
  { 0x2c,0x0a }, { 0x2d,0x00 }, { 0x2e,0x0b }, { 0x2f,0x0f },
                            
  { 0x47,0x01 }, { 0x48,0x02 }, { 0x49,0x03 }, { 0xb5,0x0c },
  { 0x4b,0x04 }, { 0x4c,0x05 }, { 0x4d,0x06 }, { 0x37,0x0d },
  { 0x4f,0x07 }, { 0x50,0x08 }, { 0x51,0x09 }, { 0x4a,0x0e },
  { 0x52,0x0a }, { 0x53,0x00 }, { 0x9c,0x0b }, { 0x4e,0x0f },

  { 0x00,0x00 }
 };
 byte i;
 /* start with all keys up */
 memset (chip8_keys,0,sizeof(chip8_keys));
 /* now check which are pressed */
 for (i=0;keyinfo[i].code;++i)
  if (keyb_status[keyinfo[i].code])
   chip8_keys[keyinfo[i].key]=1;
 /* check if ESC or F10 is being held down */
 if (keyb_status[0x01] || keyb_status[0x44]) chip8_running=0;
 /* what about F1 ? */
 if (keyb_status[0x3b]) chip8_reset();
}

/****************************************************************************/
/* Return time passed in 55th seconds                                       */
/****************************************************************************/
static byte get_timer_count (void)
{
 return timer_count;
}

/****************************************************************************/
/* Update keyboard and display, sync emulation with hardware timer          */
/****************************************************************************/
void chip8_interrupt (void)
{
 static int ucount=1;
 if (!--ucount)
 {
  ucount=uperiod;
  update_display ();
 }
 check_keys ();
 if (sync)
 {
  while (get_timer_count()==0);
  timer_count=0;
 }
}

/****************************************************************************/
/* New timer interrupt routine                                              */
/****************************************************************************/
static void interrupt new_int8 (void)
{
 static int icount=3;
 ++timer_count;                                 /* increment counter        */
 if (!--icount)                                 /* call old routine 18,2    */
 { icount=3; (*old_int8)(); }                   /* times per second         */
 else
 { outportb (0x20,0x20); }                      /* acknowledge interrupt    */
}

/****************************************************************************/
/* New keyboard interrupt routine                                           */
/****************************************************************************/
static void interrupt new_int9 (void)
{
 unsigned code;
 static int extkey;
 code=inportb (0x60);                           /* get scancode             */
 if (code<0xE0)                                 /* ignore codes >0xE0       */
 {
  keyb_status[(code&0x7f)+extkey]=((code&0x80)==0);
  extkey=0;
 }
 else if (code==0xE0) extkey=128;
 code=inportb (0x61);                           /* acknowledge interrupt    */
 outportb (0x61,code | 0x80);
 outportb (0x61,code);
 outportb (0x20,0x20);
}

/****************************************************************************/
/* Parse command line options and start emulation                           */
/****************************************************************************/
int main (int argc,char *argv[])
{
 FILE *f;
 union REGS r;
 byte old_video_mode;
 int i,j,k,misparm;
 char *options[] = { "h","up","ip","s",NULL };
 char *program=NULL;
 chip8_iperiod=15;
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
             return 1;
   }
  else
  {    
   for (j=0;options[j];j++)
    if (!strcmp(argv[i]+1,options[j])) break;
   switch (j)
   {
    case 0:
     printf ("Usage: v8 [options] <filename>\n"
             "Available options are:\n"
             " -h          - Print this help page\n"
             " -up <value> - Select number of interrupts per screen update [1]\n"
             " -ip <value> - Select number of opcodes per interrupt [15]\n"
             " -s <value>  - Select synchronisation mode [1]\n"
             "               0 - Do not sync emulation\n"
             "               1 - Sync emulation to 55Hz clock\n");
             return 0;
    case 1:  i++;
             if (i<argc) uperiod=atoi(argv[i]);
             else misparm=1;
             break;
    case 2:  i++;
             if (i<argc) chip8_iperiod=atoi(argv[i]);
             else misparm=1;
             break;
    case 3:  i++;
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
 printf ("%d bytes loaded\n",i);
 printf ("Press any key to start emulation...");
 getch ();
 memset (&r,0,sizeof(r));                       /* get current video mode   */
 r.h.ah=0x0f;
 int86 (0x10,&r,&r);
 old_video_mode=r.h.al;
 memset (&r,0,sizeof(r));                       /* set 320x200x4 (CGA) mode */
 r.x.ax=0x04;
 int86 (0x10,&r,&r);
 old_int8=getvect (8);                          /* get interrupt vectors    */
 old_int9=getvect (9);
 setvect (8,new_int8);                          /* and set our own          */
 setvect (9,new_int9);
 disable ();                                    /* set timer frequency to   */
 outportb (0x43,0x36);                          /* 55Hz                     */
 outportb (0x40,0x55);
 outportb (0x40,0x55);
 enable ();
 chip8 ();                                      /* start emulation          */
 setvect (8,old_int8);                          /* restore interrupt        */
 setvect (9,old_int9);                          /* vectors                  */
 memset (&r,0,sizeof(r));
 r.x.ax=old_video_mode;                         /* restore original video   */
 int86 (0x10,&r,&r);                            /* mode                     */
 nosound ();                                    /* turn sound off           */
 disable ();                                    /* restore timer frequency  */
 outportb (0x43,0x36);
 outportb (0x40,0);
 outportb (0x40,0);
 enable ();
 return 0;                                      /* all done                 */
}

