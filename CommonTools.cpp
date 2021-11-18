#include "CommonTools.h"
#include <windows.h>
#include <CommCtrl.h>
#include "resource.h"

#pragma warning (disable:4996)

void CommonTools::InitializeListView(HWND hWnd)
{
	
	LVCOLUMN col = { 0 };
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	col.pszText = (LPSTR)"Name";

	col.cx = 100;
	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTCOLUMN, 0, (LPARAM)&col);

	col.iOrder = { 1 };
	col.pszText = (LPSTR)"PID";
	col.cx = 50;

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTCOLUMN, 1, (LPARAM)&col);


	col.iOrder = { 2 };
	col.pszText = (LPSTR)"State";
	col.cx = 70;

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTCOLUMN, 2, (LPARAM)&col);


	col.iOrder = { 3 };
	col.pszText = (LPSTR)"User Time";
	col.cx = 80;

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTCOLUMN, 3, (LPARAM)&col);

	col.iOrder = { 4 };
	col.pszText = (LPSTR)"Kernel Time";
	col.cx = 95;

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTCOLUMN, 4, (LPARAM)&col);

	SendDlgItemMessage(
		hWnd, IDC_LIST,
		LVM_SETEXTENDEDLISTVIEWSTYLE,
		0,
		LVS_EX_FULLROWSELECT
	);
}


bool CommonTools::BrowseFileName(HWND hWnd, char* FileName)
{
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0"
		"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe";
	return GetOpenFileName(&ofn);
}


bool CommonTools::RunProcess(LPSTR command, HANDLE& handle, DWORD& PID)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	if (CreateProcess(
		NULL,
		command,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)
		)
	{
		handle = pi.hProcess;
		PID = pi.dwProcessId;

		CloseHandle(pi.hThread);


		return true;
	}
	return false;

}

DWORD CommonTools::FileTimeToMilliseconds(FILETIME ft)
{
	ULARGE_INTEGER i;
	i.LowPart = ft.dwLowDateTime;
	i.HighPart = ft.dwHighDateTime;
	return i.QuadPart / 10000;
}
