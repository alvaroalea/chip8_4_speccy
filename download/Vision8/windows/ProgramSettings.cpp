//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "ProgramSettings.h"
#include <dir.h>
//---------------------------------------------------------------------------
TProgramSettings *ProgramSettings=NULL;
//---------------------------------------------------------------------------
TProgramSettings::TProgramSettings()
{
 char ModuleFileName[MAXPATH];
 char IniFileName[MAXPATH];
 char Drive[MAXDRIVE];
 char Dir[MAXDIR];
 char File[MAXFILE];
 GetModuleFileName (HInstance,ModuleFileName,sizeof(ModuleFileName));
 fnsplit(ModuleFileName,Drive,Dir,File,NULL);
 fnmerge(IniFileName,Drive,Dir,File,".ini");
 FIniFile=new TIniFile(IniFileName);
 FSection=File;
}
//---------------------------------------------------------------------------
TProgramSettings::~TProgramSettings()
{
 delete FIniFile;
}
//---------------------------------------------------------------------------
void TProgramSettings::SetSection(AnsiString Section)
{
 FSection=Section;
}
//---------------------------------------------------------------------------
int TProgramSettings::ReadInteger(AnsiString Key,int Default)
{
 return FIniFile->ReadInteger(FSection,Key,Default);
}
//---------------------------------------------------------------------------
bool TProgramSettings::ReadBool(AnsiString Key,bool Default)
{
 return FIniFile->ReadBool(FSection,Key,Default);
}
//---------------------------------------------------------------------------
AnsiString TProgramSettings::ReadString(AnsiString Key,AnsiString Default)
{
 return FIniFile->ReadString(FSection,Key,Default);
}
//---------------------------------------------------------------------------
void TProgramSettings::WriteInteger(AnsiString Key,int Value)
{
 FIniFile->WriteInteger(FSection,Key,Value);
}
//---------------------------------------------------------------------------
void TProgramSettings::WriteBool(AnsiString Key,bool Value)
{
 FIniFile->WriteBool(FSection,Key,Value);
}
//---------------------------------------------------------------------------
void TProgramSettings::WriteString(AnsiString Key,AnsiString Value)
{
 FIniFile->WriteString(FSection,Key,Value);
}
//---------------------------------------------------------------------------
void TProgramSettings::ReadSection(TStrings *Items)
{
 FIniFile->ReadSection(FSection,Items);
}
//---------------------------------------------------------------------------
void TProgramSettings::ReadSectionValues(TStrings *Items)
{
 FIniFile->ReadSectionValues(FSection,Items);
}
//---------------------------------------------------------------------------
