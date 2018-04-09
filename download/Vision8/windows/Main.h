//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <ddraw.h>
//---------------------------------------------------------------------------
#include "EmuThread.h"
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu;
	TOpenDialog *OpenDialog;
	TMenuItem *FileMenu;
	TMenuItem *OpenMenu;
	TMenuItem *ExitMenu;
	TMenuItem *HelpMenu;
	TMenuItem *AboutMenu;
	TPanel *ButtonBarPanel;
	TBevel *Bevel1;
	TStatusBar *StatusBar1;
	TSpeedButton *OpenButton;
	TMenuItem *ResetMenu;
	TSpeedButton *ResetButton;
	TSpeedButton *PauseButton;
	TSpeedButton *ResumeButton;
	TMenuItem *EmulationMenu;
	TMenuItem *PauseMenu;
	TMenuItem *ResumeMenu;
	TMenuItem *OptionsMenu;
	TMenuItem *Size1;
	TMenuItem *Menu256x128;
	TMenuItem *Menu384x196;
	TMenuItem *Menu512x256;
	TMenuItem *Menu768x384;
	TMenuItem *Menu640x320;
	TMenuItem *Menu128x64;
	TPaintBox *EmuPaintBox;
	void __fastcall ExitMenuClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall EmuPaintBoxPaint(TObject *Sender);
	void __fastcall OpenMenuClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ResetMenuClick(TObject *Sender);
	void __fastcall PauseMenuClick(TObject *Sender);
	void __fastcall ResumeMenuClick(TObject *Sender);
	void __fastcall OptionsMenuClick(TObject *Sender);
	void __fastcall AboutMenuClick(TObject *Sender);
	void __fastcall SizeMenuClick(TObject *Sender);
	
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
private:	// User declarations
	TEmuThread *FEmuThread;
    bool FNotYetShown;
    int FOldClock;
    int FNewClock;
    int FFramesSkipped;
    bool CheckScreenRefresh(void);
    bool FOpenClicked;
    bool FResetClicked;
    bool FEmuPaintBoxPainted;
    void CheckKeyboard(void);
    int FVirtualKeyCodes[256];
    int FIFreq;
    bool FWantToClose;
    void SetIFreq(int IFreq);
    void Pause(void);
    void Resume(void);
    void LoadProgram(AnsiString FileName);
    int GetIPeriod(void);
    void SetIPeriod(int IPeriod);
    bool CheckCommandLine(void);
    AnsiString FProgramToLoad;
    bool PaintImage(void);
	LPDIRECTDRAW FDirectDraw;
	LPDIRECTDRAWSURFACE	FDrawSurfacePrimary;
	LPDIRECTDRAWSURFACE	FDrawSurfaceSecondary;
	LPDIRECTDRAWCLIPPER FClipper;
public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
    __property TEmuThread *EmuThread = { read=FEmuThread };
    __property int IFreq = { read=FIFreq, write=SetIFreq };
    __property int IPeriod = { read=GetIPeriod, write=SetIPeriod };
    void Interrupt(void);
};
//---------------------------------------------------------------------------
extern TMainForm *MainForm;
//---------------------------------------------------------------------------
#define START_EMU_THREAD_MSG	(WM_USER+0x1000)
//---------------------------------------------------------------------------
#endif
