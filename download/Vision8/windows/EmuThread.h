//---------------------------------------------------------------------------
#ifndef EmuThreadH
#define EmuThreadH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
//---------------------------------------------------------------------------
#include "Sound.h"
//---------------------------------------------------------------------------
class TEmuThread : public TThread
{
private:
	bool FReset;
	void __fastcall SynchronizedInterrupt(void);
    TSound *FSound;
protected:
	void __fastcall Execute(void);
public:
	__fastcall TEmuThread(bool CreateSuspended);
    __fastcall ~TEmuThread();
    void __fastcall TerminateHandler(TObject *Sender);
    void SoundOn(void);
    void SoundOff(void);
    void Interrupt(void);
    __property bool Reset = { read=FReset, write=FReset };
};
//---------------------------------------------------------------------------
#endif
