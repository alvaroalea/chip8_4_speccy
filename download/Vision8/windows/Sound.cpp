//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "Main.h"
#include "Sound.h"
//---------------------------------------------------------------------------
static short Sample[]=
{
#include "sample.h"
};
//---------------------------------------------------------------------------
TSound::TSound()
{
 FDirectSound=NULL;
 FDirectSoundBuffer=NULL;
 if (DirectSoundCreate(NULL,&FDirectSound,NULL)!=DS_OK)
 {
  FDirectSound=NULL;
  return;
 }
 if (FDirectSound->SetCooperativeLevel(MainForm->Handle,DSSCL_NORMAL)!=DS_OK)
 {
  FDirectSound=NULL;
  return;
 }
 PCMWAVEFORMAT WaveFormat;
 DSBUFFERDESC BufferDesc;
 memset(&WaveFormat,0,sizeof(WaveFormat));
 WaveFormat.wf.wFormatTag=WAVE_FORMAT_PCM;
 WaveFormat.wf.nChannels=1;
 WaveFormat.wf.nSamplesPerSec=44100;
 WaveFormat.wf.nBlockAlign=2;
 WaveFormat.wf.nAvgBytesPerSec=88200;
 WaveFormat.wBitsPerSample=16;
 memset(&BufferDesc,0,sizeof(BufferDesc));
 BufferDesc.dwSize=sizeof(BufferDesc);
 BufferDesc.dwFlags=DSBCAPS_CTRLDEFAULT;
 BufferDesc.dwBufferBytes=sizeof(Sample);
 BufferDesc.lpwfxFormat=(LPWAVEFORMATEX)&WaveFormat;
 if (FDirectSound->CreateSoundBuffer(&BufferDesc,&FDirectSoundBuffer,NULL)!=DS_OK)
 {
  FDirectSound=NULL;
  FDirectSoundBuffer=NULL;
  return;
 }
 void *DataPointer;
 int DataSize;
 if (FDirectSoundBuffer->Lock
 		(0,0,&DataPointer,(LPDWORD)&DataSize,NULL,NULL,DSBLOCK_ENTIREBUFFER)
 	 !=DS_OK)
 {
  FDirectSound=NULL;
  FDirectSoundBuffer=NULL;
  return;
 }
 memcpy(DataPointer,Sample,DataSize);
 if (FDirectSoundBuffer->Unlock(DataPointer,DataSize,NULL,0)!=DS_OK)
 {
  FDirectSound=NULL;
  FDirectSoundBuffer=NULL;
  return;
 }
}
//---------------------------------------------------------------------------
TSound::~TSound()
{
}
//---------------------------------------------------------------------------
void TSound::On(void)
{
 if (FDirectSoundBuffer)
  FDirectSoundBuffer->Play(0,0,DSBPLAY_LOOPING);
}
//---------------------------------------------------------------------------
void TSound::Off(void)
{
 if (FDirectSoundBuffer)
  FDirectSoundBuffer->Stop();
}
//---------------------------------------------------------------------------