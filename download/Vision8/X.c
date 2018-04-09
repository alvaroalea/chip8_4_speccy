/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                  X.c                                   **/
/**                                                                        **/
/** This file contains the X-Windows implementation                        **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/** AmiWin adaption copyright (C) Lars Malmborg 1997                       **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include "CHIP8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _AMIGA
#include <sys/time.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#ifdef _AMIGA
char *AmigaVersion="$VER: Vision-8 AmiWin 1.0 "__AMIGADATE__;

#include <pragmas/xlib_pragmas.h>
extern struct Library *XLibBase;

#include <pragmas/xleshape_pragmas.h>
extern struct Library *XLEShapeBase;

#include <pragmas/xmu2_pragmas.h>
extern struct Library *Xmu2Base;

#include <pragmas/xt_pragmas.h>
extern struct Library *XtBase;

#include <pragmas/xaw_pragmas.h>
extern struct Library *XawBase;
#endif

#define WIDTH   256
#define HEIGHT  128

/* Windows title */
static char Title[]="Vision-8 Unix/X 1.0";

static byte *DisplayBuf;          /* Screen buffer                          */
static Display *Dsp;		  /* Default display                        */
static Window Wnd;		  /* Our window				    */
static Colormap DefaultCMap;	  /* The display's default colours          */
static XImage *Img;		  /* Our image                              */
static GC DefaultGC;		  /* The default graphics context	    */
static int White,Black;		  /* White and black colour values	    */
#ifdef MITSHM			  /* SHM extensions			    */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
XShmSegmentInfo SHMInfo;
int UseSHM=1;
#endif

static int bpp;                   /* Bits per pixel of the display device   */
static int uperiod=1;             /* Number of interrupts per screen update */
static int sync=1;                /* If 0, do not sync emulation            */
static long timer;                /* Timer value at previous interrupt      */
static byte keybstatus[512];      /* If 1, specified key is being held down */

/****************************************************************************/
/* Return the time elapsed in microseconds                                  */
/****************************************************************************/
static long ReadTimer (void)
{
#ifdef HAVE_CLOCK
 return (long)((float)clock()*1000000.0/(float)CLOCKS_PER_SEC);
#else
 /* If clock() is unavailable, just return a large number */
 static long a=0;
 a+=1000000;
 return a;
#endif
}

/****************************************************************************/
/* Turn sound on                                                            */
/****************************************************************************/
void chip8_sound_on (void)
{
}

/****************************************************************************/
/* Turn sound off                                                           */
/****************************************************************************/
void chip8_sound_off (void)
{
}

/****************************************************************************/
/* Table to convert X-Windows key values to CHIP8 equivalents               */
/****************************************************************************/
static byte keyboard_table[512]=
{
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* 00 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* 10 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,13,12, 4,16, 0,      /* 20 */
  3, 2, 3, 4,13, 0, 0, 0,  0, 2, 0,15, 0, 0, 0, 0,      /* 30 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* 40 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,14, 0, 0, 0, 0,      /* 50 */
  0, 8, 0,12,10, 7,15, 0,  0, 5, 8, 9,10, 1,11, 6,      /* 60 */
  7, 5,14, 9, 0, 0,16, 6,  1, 0,11, 0, 0, 0, 0, 0,      /* 70 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* 80 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* 90 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* a0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* b0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* c0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* d0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,      /* e0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0       /* f0 */
};

/****************************************************************************/
/* Parse keyboard events and update CHIP8 keyboard status                   */
/****************************************************************************/
static void check_keys (void)
{
 XEvent E;
 unsigned long i;
 int j;
 while (XCheckWindowEvent(Dsp,Wnd,KeyPressMask|KeyReleaseMask,&E))
 {
  i=XLookupKeysym ((XKeyEvent*)&E,0);
  if ((i&0xFFFFFF00)==0 || (i&0xFFFFFF00)==0xFF00)
  {
   if (E.type==KeyPress)
   {
    if (i==XK_Escape || i==XK_F10) chip8_running=0;
    if (i==XK_F1) chip8_reset ();
    keybstatus[i&511]=1;
   }
   else
    keybstatus[i&511]=0;
  }
 }
 memset (chip8_keys,0,sizeof(chip8_keys));
 for (i=0;i<512;++i)
 {
  if (keybstatus[i])
  {
   j=keyboard_table[i];
   if (j) chip8_keys[j-1]=1;
  }
 }
}

/****************************************************************************/
/* Update the display                                                       */
/****************************************************************************/
#define UPDATE_DISPLAY \
 { \
  PIXEL *p; \
  int i,j; \
  byte *q; \
  p=(PIXEL *) DisplayBuf; \
  q=chip8_display; \
  for (i=0;i<32;++i,p+=WIDTH*3) \
   for (j=0;j<64;++j,p+=4,q++) \
   { \
    p[WIDTH*0+0]=p[WIDTH*0+1]=p[WIDTH*0+2]=p[WIDTH*0+3]= \
    p[WIDTH*1+0]=p[WIDTH*1+1]=p[WIDTH*1+2]=p[WIDTH*1+3]= \
    p[WIDTH*2+0]=p[WIDTH*2+1]=p[WIDTH*2+2]=p[WIDTH*2+3]= \
    p[WIDTH*3+0]=p[WIDTH*3+1]=p[WIDTH*3+2]=p[WIDTH*3+3]=(*q)? White:Black; \
   } \
 }

static void update_display (void)
{
 switch (bpp)
 {
  case 8:
   #define PIXEL byte
   UPDATE_DISPLAY
   #undef PIXEL
   break;
  case 16:
   #define PIXEL unsigned short
   UPDATE_DISPLAY
   #undef PIXEL
   break;
  case 32:
   #define PIXEL unsigned
   UPDATE_DISPLAY
   #undef PIXEL
   break;
 }
#ifdef MITSHM
 if (UseSHM) XShmPutImage (Dsp,Wnd,DefaultGC,Img,0,0,0,0,WIDTH,HEIGHT,False);
 else
#endif
 XPutImage (Dsp,Wnd,DefaultGC,Img,0,0,0,0,WIDTH,HEIGHT);
 XFlush (Dsp);
}

/****************************************************************************/
/* Update keyboard and display, sync emulation with VDP clock               */
/****************************************************************************/
void chip8_interrupt (void)
{
 static int ucount=1;
 long newtimer;
 if (!--ucount)
 {
  ucount=uperiod;
  update_display ();
 }
 check_keys ();
 if (sync)
 {
  newtimer=ReadTimer ();
  timer+=20000;
  if ((newtimer-timer)<0)
  {
   do
    newtimer=ReadTimer ();
   while ((newtimer-timer)<0);
  }
  else timer=newtimer;
 }
}

/****************************************************************************/
/** Deallocate all resources taken by InitMachine()                        **/
/****************************************************************************/
static void TrashMachine(void)
{
 if (Dsp && Wnd)
 {
#ifdef MITSHM
  if (UseSHM)
  {
   XShmDetach (Dsp,&SHMInfo);
   if (SHMInfo.shmaddr) shmdt (SHMInfo.shmaddr);
   if (SHMInfo.shmid>=0) shmctl (SHMInfo.shmid,IPC_RMID,0);
  }
  else
#endif
  if (Img) XDestroyImage (Img);
 }
 if (Dsp) XCloseDisplay (Dsp);
}

/****************************************************************************/
/** Allocate resources needed by X-Windows-dependent code                  **/
/****************************************************************************/
static int InitMachine(void)
{
 Screen *Scr;
 XSizeHints Hints;
 XWMHints WMHints;
 printf ("Initialising Unix/X drivers...\n");
 printf ("  Opening display... ");
 Dsp=XOpenDisplay (NULL);
 if (!Dsp)
 {
  printf ("FAILED\n");
  return 0;
 }
 Scr=DefaultScreenOfDisplay (Dsp);
 White=WhitePixelOfScreen (Scr);
 Black=BlackPixelOfScreen (Scr);
 DefaultGC=DefaultGCOfScreen (Scr);
 DefaultCMap=DefaultColormapOfScreen (Scr);
 bpp=DefaultDepthOfScreen (Scr);
 if (bpp!=8 && bpp!=16 && bpp!=32)
 {
  printf ("FAILED - Only 8,16 and 32 bpp displays are supported\n");
  return 0;
 }
 if (bpp==32 && sizeof(unsigned)!=4)
 {
  printf ("FAILED - 32 bpp displays are only supported on 32 bit machines\n");
  return 0;
 }
 printf ("OK\n  Opening window... ");
 Wnd=XCreateSimpleWindow (Dsp,RootWindowOfScreen(Scr),
                          0,0,WIDTH,HEIGHT,0,White,Black);
 if (!Wnd)
 {
  printf ("FAILED\n");
  return 0;
 }
 Hints.flags=PSize|PMinSize|PMaxSize;
 Hints.min_width=Hints.max_width=Hints.base_width=WIDTH;
 Hints.min_height=Hints.max_height=Hints.base_height=HEIGHT;
 WMHints.input=True;
 WMHints.flags=InputHint;
 XSetWMHints (Dsp,Wnd,&WMHints);
 XSetWMNormalHints (Dsp,Wnd,&Hints);
 XStoreName (Dsp,Wnd,Title);
 XSelectInput (Dsp,Wnd,FocusChangeMask|ExposureMask|KeyPressMask|KeyReleaseMask);
 XMapRaised (Dsp,Wnd);
 XClearWindow (Dsp,Wnd);
 printf ("OK\n");
#ifdef MITSHM
 if (UseSHM)
 {
  printf ("  Using shared memory:\n    Creating image... ");
  Img=XShmCreateImage (Dsp,DefaultVisualOfScreen(Scr),bpp,
                       ZPixmap,NULL,&SHMInfo,WIDTH,HEIGHT);
  if (!Img)
  {
   printf ("FAILED\n");
   return 0;
  }
  printf ("OK\n    Getting SHM info... ");
  SHMInfo.shmid=shmget (IPC_PRIVATE,Img->bytes_per_line*Img->height,
  			IPC_CREAT|0777);
  if (SHMInfo.shmid<0)
  {
   printf ("FAILED\n");
   return 0;
  }
  printf ("OK\n    Allocating SHM... ");
  Img->data=SHMInfo.shmaddr=shmat(SHMInfo.shmid,0,0);
  DisplayBuf=Img->data;
  if (!DisplayBuf)
  {
   printf ("FAILED\n");
   return 0;
  }
  SHMInfo.readOnly=False;
  printf ("OK\n    Attaching SHM... ");
  if (!XShmAttach(Dsp,&SHMInfo))
  {
   printf ("FAILED\n");
   return 0;
  }
 }
 else
#endif
 {
  printf ("  Allocating screen buffer... ");
  DisplayBuf=malloc(bpp*WIDTH*HEIGHT/8);
  if (!DisplayBuf)
  {
   printf ("FAILED\n");
   return 0;
  }
  printf ("OK\n  Creating image... ");
  Img=XCreateImage (Dsp,DefaultVisualOfScreen(Scr),bpp,ZPixmap,
  		    0,DisplayBuf,WIDTH,HEIGHT,8,0);
  if (!Img)
  {
   printf ("FAILED\n");
   return 0;
  }
 }
 printf ("OK\n");
 timer=ReadTimer ();
 return 1;
}

/****************************************************************************/
/* Parse command line options and start emulation                           */
/****************************************************************************/
int main (int argc,char *argv[])
{
 FILE *f;
 int i,j,k,misparm;
 char *options[] = { "h","trap","up","ip","s",NULL };
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
#ifdef DEBUG
             " -trap <pc>  - Trap execution when PC reaches address <pc> [000]\n"
#endif
             " -up <value> - Select number of interrupts per screen update [1]\n"
             " -ip <value> - Select number of opcodes per interrupt [15]\n"
             " -s <value>  - Select synchronisation mode [1]\n"
             "               0 - Do not sync emulation\n"
             "               1 - Sync emulation to 50Hz clock\n");
             return 0;
#ifdef DEBUG
    case 1:  i++;
             if (i<argc) chip8_trap=strtoul(argv[i],NULL,16);
             else misparm=1;
             break;
#endif
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
 printf ("%d bytes loaded\n",i);
 if (!InitMachine())                            /* Initialise X-Windows     */
  return 1;                                     /* driver                   */
 chip8 ();                                      /* start emulation          */
 TrashMachine ();                               /* Deallocate resources     */
 return 0;                                      /* all done                 */
}
