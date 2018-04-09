//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("Main.cpp", MainForm);
USERES("Vision8.res");
USEUNIT("CHIP8.c");
USEUNIT("C8Debug.c");
USEUNIT("EmuThread.cpp");
USEUNIT("Sound.cpp");
USEFORM("Options.cpp", OptionsForm);
USEFORM("About.cpp", AboutBox);
USEUNIT("ProgramSettings.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "Vision-8";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
