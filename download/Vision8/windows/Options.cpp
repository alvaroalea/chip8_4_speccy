//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "Options.h"
#include "Main.h"
#include "ProgramSettings.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TOptionsForm *OptionsForm;
//---------------------------------------------------------------------------
extern "C"
{
#include "CHIP8.h"
}
//---------------------------------------------------------------------------
__fastcall TOptionsForm::TOptionsForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TOptionsForm::FormCreate(TObject *Sender)
{
 ProgramSettings->Section=Name;
 Left=ProgramSettings->ReadInteger("Left",Left);
 Top=ProgramSettings->ReadInteger("Top",Top);
 IPeriodEdit->Text=chip8_iperiod;
 IFreqEdit->Text=MainForm->IFreq;
}
//---------------------------------------------------------------------------
void __fastcall TOptionsForm::FormClose(TObject *Sender, TCloseAction &Action)
{
 ProgramSettings->Section=Name;
 ProgramSettings->WriteInteger("Left",Left);
 ProgramSettings->WriteInteger("Top",Top);
 if (ModalResult!=mrOk)
  return;
 try
 {
  MainForm->IPeriod=IPeriodEdit->Text.ToInt();
 }
 catch (Exception &exception)
 {
 };
 try
 {
  MainForm->IFreq=IFreqEdit->Text.ToInt();
 }
 catch (Exception &exception)
 {
 };
}
//---------------------------------------------------------------------------