//---------------------------------------------------------------------------
#ifndef SoundH
#define SoundH
//---------------------------------------------------------------------------
#define WIN95
#include <dsound.h>
//---------------------------------------------------------------------------
class TSound
{
 private:
 	LPDIRECTSOUND FDirectSound;
	LPDIRECTSOUNDBUFFER FDirectSoundBuffer;
 	bool FPlaying;
 public:
 	TSound();
    ~TSound();
	void On(void);
    void Off(void);
};
//---------------------------------------------------------------------------
#endif
