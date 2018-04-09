//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include <time.h>
#include "Main.h"
#include "About.h"
#include "Options.h"
#include "ProgramSettings.h"
#include <dos.h>
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
extern "C"
{
#include "CHIP8.h"
}
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
bool TMainForm::CheckScreenRefresh(void)
{
 FNewClock=clock();
 FOldClock+=CLK_TCK/FIFreq;
 if ((FOldClock-FNewClock)>0)
 {
  do
   FNewClock=clock();
  while ((FNewClock-FOldClock)<0);
  FFramesSkipped=0;
  return true;
 }
 else
  if (++FFramesSkipped>=5)
  {
   FOldClock=clock();
   FFramesSkipped=0;
   return true;
  }
  else
   return false;
}
//---------------------------------------------------------------------------
void TMainForm::LoadProgram(AnsiString FileName)
{
 try
 {
  TFileStream *FileStream=new TFileStream(FileName,fmOpenRead);
  FileStream->Read(chip8_mem+0x200,sizeof(chip8_mem)-0x200);
  delete FileStream;
 }
 catch (Exception &exception)
 {
  // The default exception handler not only shows the exception that
  // occured, but also quits the message handler
  Application->ShowException(&exception);
 }
 chip8_reset();
}
//---------------------------------------------------------------------------
bool TMainForm::CheckCommandLine(void)
{
 char *options[] = { "iperiod","ifreq",NULL };
 bool Error=false;
 AnsiString Usage="Usage: vision8 [options] [filename]\n"
                  "Available options are:\n"
                  " -iperiod <value> - Select number of opcodes per interrupt [15]\n"
                  " -ifreq <value> - Select interrupt frequency in hertz [50]\n";
 for (int i=1,k=0;i<_argc && !Error;++i)
 {
  if (*_argv[i]!='-')
   switch (k++)
   {
    case 0:  FProgramToLoad=_argv[i];
             break;
    default: Error=true;
             break;;
   }
  else
  {
   int j;
   for (j=0;options[j];j++)
    if (!strcmp(_argv[i]+1,options[j])) break;
   switch (j)
   {
    case 0:  i++;
             if (i<_argc) IPeriod=atoi(_argv[i]);
             else Error=true;
             break;
    case 1:  i++;
             if (i<_argc) IFreq=atoi(_argv[i]);
             else Error=true;
             break;
    default: Error=true;
             break;
   }
  }
 }
 if (Error)
 {
  Application->MessageBox(Usage.c_str(),Application->Title.c_str(),MB_OK);
  return false;
 }
 return true;
}
//---------------------------------------------------------------------------
void TMainForm::Interrupt(void)
{
 if (FResetClicked)
 {
  FResetClicked=false;
  chip8_reset();
 }
 if (FProgramToLoad!="")
 {
  LoadProgram(FProgramToLoad);
  FProgramToLoad="";
 }
 if (FOpenClicked)
 {
  FOpenClicked=false;
  if (OpenDialog->Execute())
   LoadProgram(OpenDialog->FileName);
  FOldClock=clock();
 }
 CheckKeyboard();
 if (CheckScreenRefresh())
 {
  if (PaintImage())
   EmuPaintBoxPaint(this);
 }
}
//---------------------------------------------------------------------------
bool TMainForm::PaintImage(void)
{
 DDSURFACEDESC SurfaceDesc;
 memset(&SurfaceDesc,0,sizeof(SurfaceDesc));
 SurfaceDesc.dwSize=sizeof(SurfaceDesc);
 if (FDrawSurfaceSecondary->Lock
 		(NULL,&SurfaceDesc,DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL)
 	 !=DD_OK)
  return false;
 switch (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount)
 {
  case 32:
  {
   BYTE *p=chip8_display;
   __int32 *q=(__int32*)SurfaceDesc.lpSurface;
   for (int y=0;y<32;++y)
   {
    for (int x=0;x<64;x++)
    {
     q[x*2]=q[x*2+1]=q[SurfaceDesc.lPitch/4+x*2]=q[SurfaceDesc.lPitch/4+x*2+1]=
     	(*p)? 0x00ffffff : 0;
     p++;
    }
    q+=2*SurfaceDesc.lPitch/4;
   }
   break;
  }
  case 16:
  {
   BYTE *p=chip8_display;
   __int16 *q=(__int16*)SurfaceDesc.lpSurface;
   for (int y=0;y<32;++y)
   {
    for (int x=0;x<64;x++)
    {
     q[x*2]=q[x*2+1]=q[SurfaceDesc.lPitch/2+x*2]=q[SurfaceDesc.lPitch/2+x*2+1]=
     	(__int16)((*p)? 0xffff : 0);
     p++;
    }
    q+=SurfaceDesc.lPitch/2;
   }
   break;
  }
  case 8:
  {
   BYTE *p=chip8_display;
   __int8 *q=(__int8*)SurfaceDesc.lpSurface;
   for (int y=0;y<32;++y)
   {
    for (int x=0;x<64;x++)
    {
     q[x*2]=q[x*2+1]=q[SurfaceDesc.lPitch+x*2]=q[SurfaceDesc.lPitch+x*2+1]=
     	(__int8)((*p)? 0xff : 0);
     p++;
    }
    q+=SurfaceDesc.lPitch;
   }
   break;
  }
  default:
   OutputDebugString("Unknown pixel format\n");
 }
 FDrawSurfaceSecondary->Unlock(SurfaceDesc.lpSurface);
 return true;
}
//---------------------------------------------------------------------------
void TMainForm::CheckKeyboard(void)
{
 BYTE KeyState[256];
 if (!GetKeyboardState(KeyState))
  return;
 memset (chip8_keys,0,sizeof(chip8_keys));
 struct KeyInfo_struct
 {
  int code,key;
 };
 static struct KeyInfo_struct KeyInfo[]=
 {
  { 0x02,0x01 }, { 0x03,0x02 }, { 0x04,0x03 }, { 0x05,0x0c },
  { 0x10,0x04 }, { 0x11,0x05 }, { 0x12,0x06 }, { 0x13,0x0d },
  { 0x1e,0x07 }, { 0x1f,0x08 }, { 0x20,0x09 }, { 0x21,0x0e },
  { 0x2c,0x0a }, { 0x2d,0x00 }, { 0x2e,0x0b }, { 0x2f,0x0f },

  { 0x0a,0x01 }, { 0x0b,0x02 }, { 0x0c,0x03 }, { 0x0d,0x0c },
  { 0x17,0x04 }, { 0x18,0x05 }, { 0x19,0x06 }, { 0x1a,0x0d },
  { 0x24,0x07 }, { 0x25,0x08 }, { 0x26,0x09 }, { 0x27,0x0e },
  { 0x31,0x0a }, { 0x32,0x00 }, { 0x33,0x0b }, { 0x34,0x0f },

  { 0x00,0x00 }
 };
 for (int i=0;KeyInfo[i].code;++i)
  if (KeyState[FVirtualKeyCodes[KeyInfo[i].code]]&0x80)
   chip8_keys[KeyInfo[i].key]=1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ExitMenuClick(TObject *Sender)
{
 Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
 FNotYetShown=true;
 FEmuThread=NULL;
 FDirectDraw=NULL;
 FDrawSurfacePrimary=NULL;
 FDrawSurfaceSecondary=NULL;
 FWantToClose=false;
}
//---------------------------------------------------------------------------
void TMainForm::SetIFreq(int IFreq)
{
 if (IFreq<2) IFreq=2;
 if (IFreq>100) IFreq=100;
 FIFreq=IFreq;
}
//---------------------------------------------------------------------------
void TMainForm::SetIPeriod(int IPeriod)
{
 if (IPeriod<1) IPeriod=1;
 if (IPeriod>100) IPeriod=100;
 chip8_iperiod=(byte)IPeriod;
}
//---------------------------------------------------------------------------
int TMainForm::GetIPeriod(void)
{
 return chip8_iperiod;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
 if (!FNotYetShown) return;
 FProgramToLoad="";
 ProgramSettings=new TProgramSettings();
 ProgramSettings->Section=Name;
 Left=ProgramSettings->ReadInteger("Left",Left);
 Top=ProgramSettings->ReadInteger("Top",Top);
 Height=ProgramSettings->ReadInteger("Height",Height);
 Width=ProgramSettings->ReadInteger("Width",Width);
 IFreq=ProgramSettings->ReadInteger("IFreq",50);
 IPeriod=ProgramSettings->ReadInteger("IPeriod",15);
 FOpenClicked=false;
 FResetClicked=false;
 memset (chip8_mem,0,sizeof(chip8_mem));
 for (int i=0;i<256;++i)
  FVirtualKeyCodes[i]=MapVirtualKey(i,1);
 if (!CheckCommandLine())
 {
  Close();
  return;
 }
 DDSURFACEDESC SurfaceDesc;
 if (DirectDrawCreate(NULL,&FDirectDraw,NULL)!=DD_OK)
 {
  Application->MessageBox("Cannot initialise DirectDraw","DirectX error",MB_OK);
  Close();
  return;
 }
 if (FDirectDraw->SetCooperativeLevel(Handle,DDSCL_NORMAL)!=DD_OK)
 {
  Application->MessageBox("Cannot set cooperative level","DirectX error",MB_OK);
  Close();
  return;
 }
 SurfaceDesc.dwSize=sizeof(SurfaceDesc);
 SurfaceDesc.dwFlags=DDSD_CAPS;
 SurfaceDesc.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
 if (FDirectDraw->CreateSurface(&SurfaceDesc,&FDrawSurfacePrimary,NULL)!=DD_OK)
 {
  Application->MessageBox("Cannot create primary buffer","DirectX error",MB_OK);
  Close();
  return;
 }
 if (FDirectDraw->CreateClipper(0,&FClipper,NULL)!=DD_OK)
 {
  Application->MessageBox("Cannot create a clipper","DirectX error",MB_OK);
  Close();
  return;
 }
 if (FClipper->SetHWnd(0,Handle)!=DD_OK)
 {
  Application->MessageBox("Cannot set clipper window handle","DirectX error",MB_OK);
  Close();
  return;
 }
 if (FDrawSurfacePrimary->SetClipper(FClipper)!=DD_OK)
 {
  Application->MessageBox("Cannot set clipper","DirectX error",MB_OK);
  Close();
  return;
 }
 memset(&SurfaceDesc,0,sizeof(SurfaceDesc));
 SurfaceDesc.dwSize=sizeof(SurfaceDesc);
 SurfaceDesc.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
 SurfaceDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
 SurfaceDesc.dwWidth=128;
 SurfaceDesc.dwHeight=64;
 if (FDirectDraw->CreateSurface(&SurfaceDesc,&FDrawSurfaceSecondary,NULL) != DD_OK)
 {
  Application->MessageBox("Cannot create secondary buffer","DirectX error",MB_OK);
  Close();
  return;
 }
 if (FDrawSurfaceSecondary->GetSurfaceDesc(&SurfaceDesc) != DD_OK)
 {
  Application->MessageBox("Cannot get surface description","DirectX error",MB_OK);
  Close();
  return;
 }
 switch (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount)
 {
  case 32: case 16: case 8:
   break;
  default:
   Application->MessageBox
   	("This program requires an 8, 16 or 32 bits per pixel display mode","Error",MB_OK);
   Close();
   return;
 }
 if (FProgramToLoad=="")
  OpenMenuClick(this);
 FOldClock=clock();
 FFramesSkipped=0;
 FEmuThread=new TEmuThread(true);
 FEmuThread->Resume();
 FNotYetShown=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EmuPaintBoxPaint(TObject *Sender)
{
 RECT SourceRect;
 SourceRect.left=SourceRect.top=0;
 SourceRect.right=128;
 SourceRect.bottom=64;
 POINT Point;
 Point.x=Point.y=0;
 Point=EmuPaintBox->ClientToScreen(Point);
 RECT DestRect;
 DestRect.left=Point.x;
 DestRect.top=Point.y;
 DestRect.right=DestRect.left+EmuPaintBox->Width;
 DestRect.bottom=DestRect.top+EmuPaintBox->Height;
 while(true)
 {
  HRESULT hr=FDrawSurfacePrimary->Blt(&DestRect,FDrawSurfaceSecondary,&SourceRect,0,NULL);
  if (hr==DD_OK)
   break;
  if (hr==DDERR_SURFACELOST)
  {
   if (FDrawSurfacePrimary->Restore()!=DD_OK)
    break;
   if (FDrawSurfaceSecondary->Restore()!=DD_OK)
    break;
   if (!PaintImage())
    break;
  }
  if (hr!=DDERR_WASSTILLDRAWING)
   break;
 }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OpenMenuClick(TObject *Sender)
{
 FOpenClicked=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
 if (FEmuThread)
 {
  FEmuThread->WaitFor();
  delete FEmuThread;
 }
 if (FDirectDraw)
 {
  if (FDrawSurfacePrimary)
   FDrawSurfacePrimary->Release();
  if (FDrawSurfaceSecondary)
   FDrawSurfaceSecondary->Release();
  FDirectDraw->Release();
 }
 if (!FNotYetShown)
 {
  ProgramSettings->Section=Name;
  ProgramSettings->WriteInteger("Left",Left);
  ProgramSettings->WriteInteger("Top",Top);
  ProgramSettings->WriteInteger("Height",Height);
  ProgramSettings->WriteInteger("Width",Width);
  ProgramSettings->WriteInteger("IFreq",IFreq);
  ProgramSettings->WriteInteger("IPeriod",IPeriod);
 }
 if (ProgramSettings)
  delete ProgramSettings;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ResetMenuClick(TObject *Sender)
{
 FResetClicked=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::PauseMenuClick(TObject *Sender)
{
 if (!FEmuThread->Suspended)
  FEmuThread->Suspend();
 PauseMenu->Checked=true;
 PauseButton->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ResumeMenuClick(TObject *Sender)
{
 if (FEmuThread->Suspended)
 {
  FOldClock=clock();
  FEmuThread->Resume();
 }
 PauseMenu->Checked=false;
 PauseButton->Down=false;
}
//---------------------------------------------------------------------------
void TMainForm::Pause(void)
{
 FEmuThread->Suspend();
}
//---------------------------------------------------------------------------
void TMainForm::Resume(void)
{
 FOldClock=clock();
 FEmuThread->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OptionsMenuClick(TObject *Sender)
{
 Pause();
 TOptionsForm *OptionsForm=new TOptionsForm(this);
 OptionsForm->ShowModal();
 delete OptionsForm;
 Resume();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::AboutMenuClick(TObject *Sender)
{
 Pause();
 TAboutBox *AboutBox=new TAboutBox(this);
 AboutBox->ShowModal();
 delete AboutBox;
 Resume();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SizeMenuClick(TObject *Sender)
{
 int n;
 if (Sender==Menu128x64)
  n=1;
 else if (Sender==Menu256x128)
  n=2;
 else if (Sender==Menu384x196)
  n=3;
 else if (Sender==Menu512x256)
  n=4;
 else if (Sender==Menu640x320)
  n=5;
 else if (Sender==Menu768x384)
  n=6;
 else return;
 // If the window is minimised or maximised, restore it's original
 // position and size first.
 if (IsIconic(Handle) || IsZoomed(Handle))
  ShowWindow(Handle,SW_RESTORE);
 // Set width first, because the height of the menu bar might change
 Width=Width-EmuPaintBox->Width+n*128;
 Height=Height-EmuPaintBox->Height+n*64;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
 // If the emulation thread is still running, set it's terminated
 // property. After the thread has terminated, the OnCloseQuery event
 // will be triggered again in the thread's OnTerminate event, and then
 // the form will be allowed to close.
 // If the emulation thread is terminated in the OnClose event, there's
 // a chance that it is waiting for a Synchronize, causing a deadlock.
 if (!FWantToClose)
 {
  if (FEmuThread->Suspended)
   FEmuThread->Resume();
  FEmuThread->Terminate();
  FWantToClose=true;
  CanClose=false;
 }
}
//---------------------------------------------------------------------------
