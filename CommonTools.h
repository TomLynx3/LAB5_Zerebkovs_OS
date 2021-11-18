#include <windows.h>
#include "TBuffer.h"

#pragma once
class CommonTools
{
public:
	static void InitializeListView(HWND hWnd);
	static bool BrowseFileName(HWND hWnd, char* FileName);
	static bool RunProcess(LPSTR command, HANDLE& handle, DWORD& PID);
	static DWORD FileTimeToMilliseconds(FILETIME ft);
};

