//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "EmuThread.h"
#include "Main.h"
#include <stdio.h>
extern "C"
{
#include "CHIP8.h"
}
//---------------------------------------------------------------------------
// C wrapper functions
//---------------------------------------------------------------------------
extern "C" void chip8_sound_on(void)
{
 MainForm->EmuThread->SoundOn();
}
extern "C" void chip8_sound_off(void)
{
 MainForm->EmuThread->SoundOff();
}
extern "C" void chip8_interrupt(void)
{
 MainForm->EmuThread->Interrupt();
}
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TEmuThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall TEmuThread::TEmuThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
 FSound=new TSound();
 OnTerminate=TerminateHandler;
}
//---------------------------------------------------------------------------
__fastcall TEmuThread::~TEmuThread()
{
 delete FSound;
}
//---------------------------------------------------------------------------
void __fastcall TEmuThread::TerminateHandler(TObject *Sender)
{
 // MainForm->Close() can't be used because it calls the OnClose
 // event handler which causes this thread to be deleted before this
 // function has returned, so we use PostMessage() instead.
 PostMessage(MainForm->Handle,WM_CLOSE,0,0);
}
//---------------------------------------------------------------------------
void __fastcall TEmuThread::Execute(void)
{
 chip8();
}
//---------------------------------------------------------------------------
void TEmuThread::SoundOn(void)
{
 FSound->On();
}
//---------------------------------------------------------------------------
void TEmuThread::SoundOff(void)
{
 FSound->Off();
}
//---------------------------------------------------------------------------
void __fastcall TEmuThread::SynchronizedInterrupt(void)
{
 MainForm->Interrupt();
}
//---------------------------------------------------------------------------
void TEmuThread::Interrupt(void)
{
 if (Terminated)
  chip8_running=0;
 else
  Synchronize(SynchronizedInterrupt);
}
//---------------------------------------------------------------------------
