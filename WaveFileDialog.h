#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
static HWND Hwnd;

int32_t WaveOpenFileDialog(int8_t* Path, const int8_t* FileExtensions)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Hwnd;
	ofn.lpstrFile = Path;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 1000;
	ofn.lpstrFilter = "All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	
 //   int8_t CurrentPath[4096];
//    GetCurrentDirectory(4096, CurrentPath);
	int32_t Ret = GetOpenFileName(&ofn);
//    SetCurrentDirectory(CurrentPath);

	return Ret;
}

int32_t WaveSaveFileDialog(int8_t* Path, const int8_t* FileExtensions)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Hwnd;
	ofn.lpstrFile = Path;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 1000;
	ofn.lpstrFilter = "All Files\0*.*\0";
	ofn.nFilterIndex = 1;

//	int8_t CurrentPath[4096];
//	GetCurrentDirectory(4096, CurrentPath);
	int32_t Ret = GetSaveFileName(&ofn);
//	SetCurrentDirectory(CurrentPath);

	return Ret;
}

#elif __linux__

int32_t WaveOpenFileDialog(int8_t* Path, const int8_t* FileExtensions)
{
	const int8_t ZenityPath[] = "/usr/bin/zenity";
  
	sprintf(Call, "%s  --file-selection --modal --title=\"%s\" ", ZenityPath, "Open");

	FILE *File = popen(Call, "r");
	if (!File)
		return 0;

	fgets(Path, 4096, File); 

	pclose(File);

	return 1;
}

int32_t WaveSaveFileDialog(int8_t* Path, const int8_t* FileExtensions)
{

}

#endif

