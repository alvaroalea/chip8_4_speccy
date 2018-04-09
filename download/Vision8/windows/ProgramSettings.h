//---------------------------------------------------------------------------
#ifndef ProgramSettingsH
#define ProgramSettingsH
//---------------------------------------------------------------------------
#include <vcl/inifiles.hpp>
//---------------------------------------------------------------------------
class TProgramSettings
{
 private:
  TIniFile *FIniFile;
  AnsiString FSection;
  void SetSection(AnsiString Section);
 public:
  TProgramSettings();
  ~TProgramSettings();
  __property AnsiString Section = { read=FSection, write=SetSection };
  int ReadInteger(AnsiString Key,int Default);
  bool ReadBool(AnsiString Key,bool Default);
  void ReadSection(TStrings *Items);
  void ReadSectionValues(TStrings *Items);
  AnsiString ReadString(AnsiString Key,AnsiString Default);
  void WriteInteger(AnsiString Key,int Value);
  void WriteBool(AnsiString Key,bool Value);
  void WriteString(AnsiString Key,AnsiString Value);
};
//---------------------------------------------------------------------------
extern TProgramSettings *ProgramSettings;
//---------------------------------------------------------------------------
#endif
