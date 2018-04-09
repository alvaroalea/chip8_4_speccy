/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                Amiga.c                                 **/
/**                                                                        **/
/** This file contains the Amiga implementation                            **/
/**                                                                        **/
/** Copyright (C) Lars Malmborg (glue@df.lth.se) 1997                      **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Amiga includes */
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/asl.h>
#include <dos/rdargs.h>
#include <exec/memory.h>
#include <devices/audio.h>

#include "CHIP8.h"

/* Amiga RawKeys */
#define KEY_ESC (0x45)
#define KEY_F1 (0x50)
#define KEY_HELP (0x5F)

#define KEY_1 (0x01)
#define KEY_2 (0x02)
#define KEY_3 (0x03)
#define KEY_4 (0x04)
#define KEY_Q (0x10)
#define KEY_W (0x11)
#define KEY_E (0x12)
#define KEY_R (0x13)
#define KEY_A (0x20)
#define KEY_S (0x21)
#define KEY_D (0x22)
#define KEY_F (0x23)
#define KEY_Z (0x31)
#define KEY_X (0x32)
#define KEY_C (0x33)
#define KEY_V (0x34)

#define KEY_9 (0x09)
#define KEY_0 (0x0A)
#define KEY_MINUS (0x0B)
#define KEY_EQUAL (0x0C)
#define KEY_I (0x17)
#define KEY_O (0x18)
#define KEY_P (0x19)
#define KEY_LBRACK (0x1A)
#define KEY_J (0x26)
#define KEY_K (0x27)
#define KEY_L (0x28)
#define KEY_SEMICOL (0x29)
#define KEY_N (0x36)
#define KEY_M (0x37)
#define KEY_COMMA (0x38)
#define KEY_PERIOD (0x39)

#define WIDTH   256
#define HEIGHT  128

#define FREQUENCY 440		/* Frequency of the tone desired */
#define DURATION 3			/* Duration in seconds */
#define CLOCK 3546895		/* Clock constant, 3546895 for PAL */
#define SAMPLES 2				/* Number of sample bytes */
#define SAMPLECYCLES 1	/* Number of cycles in the sample */

#define PERIOD CLOCK*SAMPLECYCLES/(SAMPLES*FREQUENCY)
#define CYCLES 0				/* Infinite loop (FREQUENCY*DURATION/SAMPLECYCLES) */

char *Title = "Vision-8 Amiga 1.0";

char *AmigaVersion="$VER: Vision-8 Amiga 1.0 "__AMIGADATE__;

char ReqText[]=	"Vision-8: Portable CHIP8 emulator\n"
								"© 1997 by Marcel de Kogel\n"
								"Amiga port 1.0 by Lars Malmborg\n";

struct EasyStruct HelpReq =
{
	sizeof(struct EasyStruct),
	0,
	"Vision-8 Help",
	ReqText,
	"OK"
};

char *HelpText=	"Vision-8: Portable CHIP8 emulator\n"
								"© 1997 by Marcel de Kogel\n"
								"Amiga port 1.0 by Lars Malmborg\n"
								"\n"
								"Usage: v8 <filename> [options]\n"
								"Available options are:\n"
								" Verbose               - Print verbose information\n"
#ifdef DEBUG
								" Trap <pc> (hex)       - Trap execution when PC reaches address <pc> [000]\n"
#endif
								" UpdatePeriod <value>  - Select number of interrupts per screen update [1]\n"
								" InstrPeriod <value>   - Select number of opcodes per interrupt [15]\n"
								" NoSync                - Disable synchronisation of emulation\n"
								" ScaleUp               - Double window size\n"
								" Borderless            - Use a borderless window\n"
								" PublicScreen <screen> - Open windows on a (public) screen\n"
								" NoBuzzer              - Disable sound emulation\n"
								" Help                  - Print this help page\n";


struct Screen *scr=NULL;
struct Window *win=NULL;
struct RastPort *rp=NULL;

struct RastPort temprp;

LONG White=-1,Black=-1; /* White and black palette numbers */

BOOL ScaleUp;
BOOL Verbose;
BOOL Borderless;
STRPTR PubScreen;
int UpdatePeriod=1; /* Number of interrupts per screen update */
BOOL Sync=TRUE; /* If 0, do not sync emulation with TOF */

BOOL Sound=TRUE;

STRPTR ReqFile(void);

UBYTE whichannel[]={1,2,4,8};

struct IOAudio AudioIO; /* Pointer to the I/O block for I/O commands */
struct MsgPort *AudioMP=NULL; /* Pointer to a port so the device can reply */
BYTE device=-1;

__chip static BYTE waveptr[SAMPLES]={127,-127};

/****************************************************************************/
/* Turn sound on                                                            */
/****************************************************************************/
void chip8_sound_on (void)
{
	if(Sound)
	{
		AudioIO.ioa_Request.io_Message.mn_ReplyPort = AudioMP;
		AudioIO.ioa_Request.io_Command              = CMD_WRITE;
		AudioIO.ioa_Request.io_Flags                = ADIOF_PERVOL;
		AudioIO.ioa_Data                            = (BYTE *)&waveptr;
		AudioIO.ioa_Length                          = SAMPLES;
		AudioIO.ioa_Period                          = PERIOD;
		AudioIO.ioa_Volume                          = 64;
		AudioIO.ioa_Cycles                          = CYCLES;

		BeginIO((struct IORequest *) &AudioIO );
	}
}

/****************************************************************************/
/* Turn sound off                                                           */
/****************************************************************************/
void chip8_sound_off (void)
{
	if(Sound)
	{
		AudioIO.ioa_Request.io_Message.mn_ReplyPort = AudioMP;
		AudioIO.ioa_Request.io_Command              = CMD_FLUSH;
		AudioIO.ioa_Request.io_Flags                = NULL;

		BeginIO((struct IORequest *) &AudioIO );
	}
}

/****************************************************************************/
/* Defines which key values represents CHIP8 equivalents                    */
/****************************************************************************/
/*
 * Most of the original CHIP8 games are programmed for a keyboard with the
 * following layout:
 * 1 2 3 C
 * 4 5 6 D
 * 7 8 9 E
 * A 0 B F
 * This is emulated as follows (Physical location of keys is for US keymap):
 * 1 2 3 4
 * q w e r
 * a s d f
 * z x c v
 * and:
 * 9 0 - =
 * i o p [
 * j k l ;
 * n m , .
 */
#define C8_0 13
#define C8_1 0
#define C8_2 1
#define C8_3 2
#define C8_4 4
#define C8_5 5
#define C8_6 6
#define C8_7 8
#define C8_8 9
#define C8_9 10
#define C8_A 12
#define C8_B 14
#define C8_C 3
#define C8_D 7
#define C8_E 11
#define C8_F 15

/****************************************************************************/
/* Update the display                                                       */
/****************************************************************************/
static void UpdateDisplay(void)
{
	UBYTE drawarray[2*WIDTH];

	register UBYTE *p;
	register byte *q;
	int i,j;

	register UBYTE b;
	register UBYTE w;

	b=Black;
	w=White;
  q=chip8_display;

	if(ScaleUp)
	{
		for (i=0;i<32;i++)
		{
			p=(UBYTE *) drawarray;
			for (j=0;j<64;j++)
			{
				p[0]=p[1]=p[2]=p[3]=p[4]=p[5]=p[6]=p[7]=(*q)?w:b;
				q++;
				p+=8;
			}
			WritePixelLine8(rp,win->BorderLeft,win->BorderTop+i*8+0,
				2*WIDTH,(UBYTE*)drawarray,&temprp);
			ClipBlit(rp,win->BorderLeft,win->BorderTop+i*8+0,
				rp,win->BorderLeft,win->BorderTop+i*8+1,
				2*WIDTH,1,0x0c0);
			ClipBlit(rp,win->BorderLeft,win->BorderTop+i*8+0,
				rp,win->BorderLeft,win->BorderTop+i*8+2,
				2*WIDTH,2,0x0c0);
			ClipBlit(rp,win->BorderLeft,win->BorderTop+i*8+0,
				rp,win->BorderLeft,win->BorderTop+i*8+4,
				2*WIDTH,4,0x0c0);
		}
	}
	else
	{
		for (i=0;i<32;i++)
		{
			p=(UBYTE *) drawarray;
			for (j=0;j<64;j++)
			{
				p[0]=p[1]=p[2]=p[3]=(*q)?w:b;
				q++;
				p+=4;
			}
			WritePixelLine8(rp,win->BorderLeft,win->BorderTop+i*4+0,
				WIDTH,(UBYTE*)drawarray,&temprp);
			ClipBlit(rp,win->BorderLeft,win->BorderTop+i*4+0,
				rp,win->BorderLeft,win->BorderTop+i*4+1,
				WIDTH,1,0x0c0);
			ClipBlit(rp,win->BorderLeft,win->BorderTop+i*4+0,
				rp,win->BorderLeft,win->BorderTop+i*4+2,
				WIDTH,2,0x0c0);
		}
	}
}

byte chip8_keys_left[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
byte chip8_keys_right[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/****************************************************************************/
/* Update keyboard and display, sync emulation with TOF clock               */
/****************************************************************************/
void chip8_interrupt(void)
{
	static LONG UpdateCount=1;
	struct IntuiMessage *imsg;
	ULONG imsg_Class;
	UWORD Code;
	int i;

	while(imsg = (struct IntuiMessage*)GetMsg(win->UserPort))
	{
		imsg_Class = imsg->Class;
		Code  = imsg->Code;
		ReplyMsg((struct Message*)imsg);
		switch(imsg_Class)
		{
			case IDCMP_CLOSEWINDOW:
				chip8_running=0;
				break;
			case IDCMP_RAWKEY:
				if(Code & 0x80)
				{
					switch(Code & 0x7f)
					{
						case KEY_ESC:
							chip8_running=0;
							break;
						case KEY_F1:
							chip8_reset();
							break;
						case KEY_HELP:
							EasyRequestArgs(win,&HelpReq,NULL,NULL);
							break;
						/* Left keys */
						case KEY_1:
							chip8_keys_left[C8_1]=0;
							break;
						case KEY_2:
							chip8_keys_left[C8_2]=0;
							break;
						case KEY_3:
							chip8_keys_left[C8_3]=0;
							break;
						case KEY_4:
							chip8_keys_left[C8_C]=0;
							break;
						case KEY_Q:
							chip8_keys_left[C8_4]=0;
							break;
						case KEY_W:
							chip8_keys_left[C8_5]=0;
							break;
						case KEY_E:
							chip8_keys_left[C8_6]=0;
							break;
						case KEY_R:
							chip8_keys_left[C8_D]=0;
							break;
						case KEY_A:
							chip8_keys_left[C8_7]=0;
							break;
						case KEY_S:
							chip8_keys_left[C8_8]=0;
							break;
						case KEY_D:
							chip8_keys_left[C8_9]=0;
							break;
						case KEY_F:
							chip8_keys_left[C8_E]=0;
							break;
						case KEY_Z:
							chip8_keys_left[C8_A]=0;
							break;
						case KEY_X:
							chip8_keys_left[C8_0]=0;
							break;
						case KEY_C:
							chip8_keys_left[C8_B]=0;
							break;
						case KEY_V:
							chip8_keys_left[C8_F]=0;
							break;

						/* Right keys */
						case KEY_9:
							chip8_keys_right[C8_1]=0;
							break;
						case KEY_0:
							chip8_keys_right[C8_2]=0;
							break;
						case KEY_MINUS:
							chip8_keys_right[C8_3]=0;
							break;
						case KEY_EQUAL:
							chip8_keys_right[C8_C]=0;
							break;
						case KEY_I:
							chip8_keys_right[C8_4]=0;
							break;
						case KEY_O:
							chip8_keys_right[C8_5]=0;
							break;
						case KEY_P:
							chip8_keys_right[C8_6]=0;
							break;
						case KEY_LBRACK:
							chip8_keys_right[C8_D]=0;
							break;
						case KEY_J:
							chip8_keys_right[C8_7]=0;
							break;
						case KEY_K:
							chip8_keys_right[C8_8]=0;
							break;
						case KEY_L:
							chip8_keys_right[C8_9]=0;
							break;
						case KEY_SEMICOL:
							chip8_keys_right[C8_E]=0;
							break;
						case KEY_N:
							chip8_keys_right[C8_A]=0;
							break;
						case KEY_M:
							chip8_keys_right[C8_0]=0;
							break;
						case KEY_COMMA:
							chip8_keys_right[C8_B]=0;
							break;
						case KEY_PERIOD:
							chip8_keys_right[C8_F]=0;
							break;
						default:
							break;
					}
				}
				else
				{
					switch(Code)
					{
						/* Left keys */
						case KEY_1:
							chip8_keys_left[C8_1]=1;
							break;
						case KEY_2:
							chip8_keys_left[C8_2]=1;
							break;
						case KEY_3:
							chip8_keys_left[C8_3]=1;
							break;
						case KEY_4:
							chip8_keys_left[C8_C]=1;
							break;
						case KEY_Q:
							chip8_keys_left[C8_4]=1;
							break;
						case KEY_W:
							chip8_keys_left[C8_5]=1;
							break;
						case KEY_E:
							chip8_keys_left[C8_6]=1;
							break;
						case KEY_R:
							chip8_keys_left[C8_D]=1;
							break;
						case KEY_A:
							chip8_keys_left[C8_7]=1;
							break;
						case KEY_S:
							chip8_keys_left[C8_8]=1;
							break;
						case KEY_D:
							chip8_keys_left[C8_9]=1;
							break;
						case KEY_F:
							chip8_keys_left[C8_E]=1;
							break;
						case KEY_Z:
							chip8_keys_left[C8_A]=1;
							break;
						case KEY_X:
							chip8_keys_left[C8_0]=1;
							break;
						case KEY_C:
							chip8_keys_left[C8_B]=1;
							break;
						case KEY_V:
							chip8_keys_left[C8_F]=1;
							break;

						/* Right keys */
						case KEY_9:
							chip8_keys_right[C8_1]=1;
							break;
						case KEY_0:
							chip8_keys_right[C8_2]=1;
							break;
						case KEY_MINUS:
							chip8_keys_right[C8_3]=1;
							break;
						case KEY_EQUAL:
							chip8_keys_right[C8_C]=1;
							break;
						case KEY_I:
							chip8_keys_right[C8_4]=1;
							break;
						case KEY_O:
							chip8_keys_right[C8_5]=1;
							break;
						case KEY_P:
							chip8_keys_right[C8_6]=1;
							break;
						case KEY_LBRACK:
							chip8_keys_right[C8_D]=1;
							break;
						case KEY_J:
							chip8_keys_right[C8_7]=1;
							break;
						case KEY_K:
							chip8_keys_right[C8_8]=1;
							break;
						case KEY_L:
							chip8_keys_right[C8_9]=1;
							break;
						case KEY_SEMICOL:
							chip8_keys_right[C8_E]=1;
							break;
						case KEY_N:
							chip8_keys_right[C8_A]=1;
							break;
						case KEY_M:
							chip8_keys_right[C8_0]=1;
							break;
						case KEY_COMMA:
							chip8_keys_right[C8_B]=1;
							break;
						case KEY_PERIOD:
							chip8_keys_right[C8_F]=1;
							break;
						default:
							break;
					}
				}
				break;
		}
	}

	/* Merge pressed keys */
	for(i=0;i<16;i++)
		chip8_keys[i]=chip8_keys_left[i]|chip8_keys_right[i];

	if(Sync)
	{
		WaitTOF();
	}

 if(!--UpdateCount)
 {
  UpdateCount=UpdatePeriod;
	UpdateDisplay();
 }
}

/****************************************************************************/
/** Deallocate all resources taken by InitMachine()                        **/
/****************************************************************************/
static void TrashMachine(void)
{
	if(Sound)
	{
		if(device==0)
			CloseDevice((struct IORequest *)&AudioIO);
		if(AudioMP)
			DeletePort(AudioMP);
	}

	if (temprp.BitMap)
		FreeBitMap(temprp.BitMap);

	if(Black!=-1)
	{
		ReleasePen(scr->ViewPort.ColorMap,Black);
	}

	if(White!=-1)
	{
		ReleasePen(scr->ViewPort.ColorMap,White);
	}

	if (win)
		CloseWindow(win);
}

/****************************************************************************/
/** Allocate resources needed by Amiga-dependent code                      **/
/****************************************************************************/
static int InitMachine(void)
{
	int scale;
	if (ScaleUp)
		scale=2;
	else
		scale=1;

	if (scr = LockPubScreen(PubScreen))
	{
		if(Borderless)
		{
			win = OpenWindowTags(NULL,
					WA_Left,(scr->Width - scale*(WIDTH))/2,
					WA_Top,(scr->Height - scale*HEIGHT)/2,
					WA_InnerWidth,scale*(WIDTH),
					WA_InnerHeight,scale*HEIGHT,
					WA_RMBTrap,TRUE,
					WA_Activate,TRUE,
					WA_IDCMP,IDCMP_RAWKEY|IDCMP_CLOSEWINDOW,
					WA_PubScreenName,PubScreen,
					WA_Flags,WFLG_BORDERLESS,
					TAG_END
			);
		}
		else
		{
			win = OpenWindowTags(NULL,
					WA_Left,(scr->Width - scale*(WIDTH))/2,
					WA_Top,(scr->Height - scale*HEIGHT)/2,
					WA_InnerWidth,scale*(WIDTH),
					WA_InnerHeight,scale*HEIGHT,
					WA_Title,Title,
					WA_CloseGadget,TRUE,
					WA_DepthGadget,TRUE,
					WA_DragBar,TRUE,
					WA_SizeGadget,FALSE,
					WA_RMBTrap,TRUE,
					WA_Activate,TRUE,
					WA_IDCMP,IDCMP_RAWKEY|IDCMP_CLOSEWINDOW,
					WA_PubScreenName,PubScreen,
					TAG_END
			);
		}

		UnlockPubScreen(PubScreen,scr);

		if(win)
		{
			rp = win->RPort;

			Black = ObtainBestPenA(scr->ViewPort.ColorMap,0x00000000,0x00000000,0x00000000,NULL);
			White = ObtainBestPenA(scr->ViewPort.ColorMap,0xff000000,0xff000000,0xff000000,NULL);

			CopyMem(rp,&temprp,sizeof(struct RastPort));
			temprp.Layer=NULL;
			if (!(temprp.BitMap=AllocBitMap(scale*(WIDTH),1,rp->BitMap->Depth,0,NULL)))
			{
				fprintf(stderr,"Error allocating temporary rastport.\n");
				return(0);
			}
		}
		else
		{
			fprintf(stderr,"Can't open window.\n");
			return(0);
		}
	}
	else
	{
		fprintf(stderr,"Screen not found.\n");
		return(0);
	}

	if(Sound)
	{
		if(!(AudioMP=CreatePort(0,0)))
		{
			fprintf(stderr,"Can't create message port for sound channel.\n");
			return(0);
		}
		AudioIO.ioa_Request.io_Message.mn_ReplyPort   = AudioMP;
		AudioIO.ioa_Request.io_Message.mn_Node.ln_Pri = 0;
		AudioIO.ioa_Request.io_Command                = ADCMD_ALLOCATE;
		AudioIO.ioa_AllocKey                          = 0;
		AudioIO.ioa_Data                              = whichannel;
		AudioIO.ioa_Length                            = sizeof(whichannel);

		if(device=OpenDevice("audio.device",0L,(struct IORequest *)&AudioIO,0L))
		{
			fprintf(stderr,"Can't allocate sound channel.\n");
			return(0);
		}
	}

	return(1);
}

#define OPT_PROGRAM ((STRPTR)(ArgPtrs[0]))
#define OPT_VERBOSE ((BOOL)(ArgPtrs[1]))
#define OPT_TRAP ((STRPTR)(ArgPtrs[2]))
#define OPT_UP ((ULONG*)(ArgPtrs[3]))
#define OPT_IP ((ULONG*)(ArgPtrs[4]))
#define OPT_NOSYNC ((BOOL)(ArgPtrs[5]))
#define OPT_SCALE ((BOOL)(ArgPtrs[6]))
#define OPT_BORDER ((BOOL)(ArgPtrs[7]))
#define OPT_PSCREEN ((STRPTR)(ArgPtrs[8]))
#define OPT_NOSOUND ((BOOL)(ArgPtrs[9]))
#define OPT_HELP ((BOOL)(ArgPtrs[10]))

/****************************************************************************/
/* Parse command line options the Amiga way and start emulation             */
/****************************************************************************/
int main(void)
{
	LONG ArgPtrs[11]={0};
	struct RDArgs *Args;
	STRPTR FileNameBuf=NULL;
	BPTR fh;

	chip8_iperiod=15;

  Verbose=FALSE;
  UpdatePeriod=1;
	Sync=TRUE;
	ScaleUp=FALSE;
	Borderless=FALSE;
	PubScreen=NULL;
	Sound=TRUE;

	if(Args = ReadArgs("Program,V=Verbose/S,T=Trap/K,UP=UpdatePeriod/K/N,IP=InstrPeriod/K/N,NS=NoSync/S,2=ScaleUp/S,B=Borderless/S,PS=PublicScreen/K,NB=NoBuzzer/S,Help/K/S",ArgPtrs,NULL))
	{
		if(OPT_PROGRAM)
		{
			if (FileNameBuf = AllocVec(strlen(OPT_PROGRAM)+1,MEMF_ANY|MEMF_CLEAR))
				strcpy(FileNameBuf,OPT_PROGRAM);
		}

		if(OPT_VERBOSE)
		{
			Verbose = TRUE;
		};

		if(OPT_TRAP)
		{
			sscanf(OPT_TRAP,"%hx",&chip8_trap);
		};

		if(OPT_UP)
		{
			UpdatePeriod = (int)(*OPT_UP);
		};

		if(OPT_IP)
		{
			chip8_iperiod = (int)(*OPT_IP);
		};

		if(OPT_NOSYNC)
		{
			Sync = FALSE;
		};

		if(OPT_SCALE)
		{
			ScaleUp = TRUE;
		};

		if(OPT_BORDER)
		{
			Borderless = TRUE;
		};

		if(OPT_PSCREEN)
		{
			if (PubScreen=AllocVec(strlen(OPT_PSCREEN)+1,MEMF_ANY|MEMF_CLEAR))
				strcpy(PubScreen,OPT_PSCREEN);
		}

		if(OPT_NOSOUND)
		{
			Sound = FALSE;
		};

		if(OPT_HELP)
		{
			puts(HelpText);
			FreeArgs(Args);
			if (FileNameBuf)
				FreeVec(FileNameBuf);
			if (PubScreen)
				FreeVec(PubScreen);
			return(0);
		};

		FreeArgs(Args);

		if (!FileNameBuf)
		{
			if((FileNameBuf = ReqFile()) == NULL) exit(0);
		}

		if(Verbose)
			printf("Trap %03x, UpdatePeriod %d, InstrPeriod %d, %s, %s size, %s, PublicScreen \"%s\", %s\n",
				chip8_trap,
				UpdatePeriod,
				chip8_iperiod,
				Sync?"Synchronized":"Freerun",
				ScaleUp?"Double":"Normal",
				Borderless?"Borderless":"Normal window",
				PubScreen?(char*)PubScreen:"<Default>",
				Sound?"Buzzer":"Quiet");

		if(UpdatePeriod<=0)
		{
			fprintf(stderr,"Invalid UpdatePeriod.\n");
			return(1);
		}

		if(chip8_iperiod<=0)
		{
			fprintf(stderr,"Invalid InstrPeriod.\n");
			return(1);
		}

		if((chip8_trap<0)||(chip8_trap>=0x1000))
		{
			fprintf(stderr,"Invalid Trap.\n");
			return(1);
		}

		if(Verbose&&(chip8_trap<0x200))
			fprintf(stderr,"Trap set outside user program.\n");

		if(Verbose)
			printf("Opening CHIP8 program \"%s\".\n",FileNameBuf);

		if(fh=Open(FileNameBuf,MODE_OLDFILE))
		{
			int len;
			len=Read(fh,chip8_mem+0x200,4096-0x200);
			if(Verbose)
				printf("%d bytes read.\n",len);
			Close(fh);
		}
		else
		{
			fprintf(stderr,"File \"%s\" not found.\n",FileNameBuf);
			if (FileNameBuf)
				FreeVec(FileNameBuf);
			if (PubScreen)
				FreeVec(PubScreen);
			return(1);
		}

		if (!InitMachine())
		{
			TrashMachine();
			if (FileNameBuf)
				FreeVec(FileNameBuf);
			if (PubScreen)
				FreeVec(PubScreen);
			return(1);
		}
		chip8();
		TrashMachine();
		if (FileNameBuf)
			FreeVec(FileNameBuf);
		if (PubScreen)
			FreeVec(PubScreen);
	  return(0);
	}
	else
	{
		fprintf(stderr,"Couldn't parse options. Uh, this is really serious!\n");
		return(1);
	}
}

STRPTR ReqFile(void)
{
	struct FileRequester *AslReq;
	STRPTR File=NULL;
	WORD namelen;

	if(AslReq = (struct FileRequester*)AllocAslRequestTags(ASL_FileRequest,
			ASLFR_PubScreenName,PubScreen,
			ASLFR_TitleText,"Select a program...",
			ASLFR_PositiveText,"Run",
			ASLFR_RejectIcons,TRUE,
			TAG_END))
	{
		if(AslRequest(AslReq,NULL))
		{
			namelen = strlen(AslReq->fr_File) + strlen(AslReq->fr_Drawer) + 2;
			if(File = AllocVec(namelen,MEMF_ANY | MEMF_CLEAR))
			{
				strcpy(File,AslReq->fr_Drawer);
				AddPart(File,AslReq->fr_File,namelen);
			}
		}
		FreeAslRequest((APTR)AslReq);
	}
	return File;
}
