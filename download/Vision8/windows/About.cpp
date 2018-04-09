//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "About.h"
#include "ProgramSettings.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TAboutBox *AboutBox;
//--------------------------------------------------------------------- 
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TAboutBox::FormCreate(TObject *Sender)
{
 ProgramSettings->Section=Name;
 Left=ProgramSettings->ReadInteger("Left",Left);
 Top=ProgramSettings->ReadInteger("Top",Top);
}
//---------------------------------------------------------------------------
void __fastcall TAboutBox::FormClose(TObject *Sender, TCloseAction &Action)
{
 ProgramSettings->Section=Name;
 ProgramSettings->WriteInteger("Left",Left);
 ProgramSettings->WriteInteger("Top",Top);
}
//---------------------------------------------------------------------------
