#undef UNICODE
#include <windows.h>
#include "resource.h"
#include "TBuffer.h"
#include <CommCtrl.h>
#include "CommonTools.h"
#include <string>



using namespace std;

#pragma warning (disable:4996)

#define WM_UPDATELIST WM_USER


BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);



TBuffer* Buffer;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, (DLGPROC)MainWndProc);

	return 0;
}




void HandleBrowse(HWND hWnd) {
	char filename[MAX_PATH] = "";
	if (CommonTools::BrowseFileName(hWnd, filename)) {
		SetDlgItemText(hWnd, IDC_COMMANDLINE, filename);
	}

}


DWORD WINAPI ProcessThread(void* APos)
{
	int pos = (int)APos;

	TProcessInfo procInfo;

	Buffer->Get(pos,procInfo);

	HANDLE handle;
	DWORD PID;

	LPSTR command = const_cast<char*>(procInfo.Name.c_str());

	if (CommonTools::RunProcess(command, handle, PID)) {
		procInfo.Handle = handle;
		procInfo.State = TProcessState::psRunning;
		procInfo.PID = PID;

		Buffer->Set(pos, procInfo);

		FILETIME creationTime, exitTime, kerneltime, usertime;

		while (WaitForSingleObject(procInfo.Handle, 0) == WAIT_TIMEOUT) {
			
			GetProcessTimes(procInfo.Handle, &creationTime, &exitTime, &kerneltime, &usertime);

			procInfo.UserTime = CommonTools::FileTimeToMilliseconds(usertime);
			procInfo.KernelTime = CommonTools::FileTimeToMilliseconds(kerneltime);

			Buffer->Set(pos, procInfo);

			Sleep(100);
 		}

		CloseHandle(procInfo.Handle);
			procInfo.State = TProcessState::psTerminated;
			procInfo.Handle = NULL;
		
	}
	else {
		procInfo.State = TProcessState::psError;

	}

	Buffer->Set(pos, procInfo);

	return 0;
}




void  UpdateList(HWND hWnd, int pos) {

	int row = -1;
	LVFINDINFO fitem = { 0 };
	fitem.flags = LVFI_PARAM;
	fitem.lParam = pos;
	row =SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_FINDITEM, -1, (LPARAM)&fitem);

	if (row >= 0) {
		TProcessInfo procInfo;

		Buffer->Get(pos, procInfo);

		char buf[100];

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;
		item.iItem = row;
		
		item.iSubItem = 1;
		sprintf(buf, "%d", procInfo.PID);
		item.pszText =buf;
		SendDlgItemMessage(hWnd, IDC_LIST, LVM_SETITEM, 0, (LPARAM)&item);

		item.iSubItem = 2;
		item.pszText = (LPSTR) Buffer->ProcStateEnumToString(procInfo.State);
		SendDlgItemMessage(hWnd, IDC_LIST, LVM_SETITEM, 0, (LPARAM)&item);

		item.iSubItem = 3;
		sprintf(buf, "%d", procInfo.UserTime);
		item.pszText = buf;
		SendDlgItemMessage(hWnd, IDC_LIST, LVM_SETITEM, 0, (LPARAM)&item);

		item.iSubItem = 4;
		sprintf(buf, "%d", procInfo.KernelTime);
		item.pszText = buf;
		SendDlgItemMessage(hWnd, IDC_LIST, LVM_SETITEM, 0, (LPARAM)&item);

	}
	
}


void HandleStart(HWND hWnd) {


	char filename[MAX_PATH] = "";
	GetDlgItemText(hWnd, IDC_COMMANDLINE, filename, MAX_PATH);
		

	int pos =Buffer->AddProcess(filename);

	if (pos == BUF_SIZE-1) {
		EnableWindow(GetDlgItem(hWnd, IDC_START), FALSE);
	}

	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.lParam = pos;
	int count = SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETITEMCOUNT, 0, 0);
	item.iItem = count;
	item.pszText = (LPSTR)filename;


	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_INSERTITEM, 0, (LPARAM)&item);

	item.mask = LVIF_TEXT;
	item.iSubItem = 0;
	item.pszText = (LPSTR)filename;

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_SETITEM, 0, (LPARAM)&item);

	item.iSubItem = 2;
	item.pszText = (LPSTR)"new";

	SendDlgItemMessage(hWnd, IDC_LIST,
		LVM_SETITEM, 0, (LPARAM)&item);

	CloseHandle(CreateThread(NULL, 0, ProcessThread, (LPVOID)pos, 0, NULL));
}


void HandleDelete(HWND hWnd) {
	int selectedRow = SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETSELECTIONMARK, 0, 0);
	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.iItem = selectedRow;
	SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETITEM, 0, (LPARAM)&item);
	int pos = item.lParam;
	if (Buffer->DeleteBufItem(pos)) {

		if (Buffer->Count() < 100) {
			EnableWindow(GetDlgItem(hWnd, IDC_START), TRUE);
		}
		SendDlgItemMessage(hWnd, IDC_LIST,
			LVM_DELETEITEM, selectedRow, 0);
	}
	
}


void HandleKill(HWND hWnd) {
	int selectedRow = SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETSELECTIONMARK, 0, 0);
	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.iItem = selectedRow;
	SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETITEM, 0, (LPARAM)&item);
	int pos = item.lParam;

	Buffer->TerminateProc(pos);
}


void HandleBtns(HWND hWnd) {
	int selectedRow = SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETSELECTIONMARK, 0, 0);

	if (selectedRow == -1) {

		EnableWindow(GetDlgItem(hWnd, IDC_KILLBTN), FALSE);

		EnableWindow(GetDlgItem(hWnd, IDC_DELETE), FALSE);

		return;
	}

	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.iItem = selectedRow;
	SendDlgItemMessage(hWnd, IDC_LIST, LVM_GETITEM, 0, (LPARAM)&item);
	int pos = item.lParam;
	TProcessInfo procInfo;
	Buffer->Get(pos, procInfo);

	if (procInfo.State == TProcessState::psRunning) {
		EnableWindow(GetDlgItem(hWnd, IDC_KILLBTN), TRUE);

		EnableWindow(GetDlgItem(hWnd, IDC_DELETE), FALSE);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_KILLBTN), FALSE);

		EnableWindow(GetDlgItem(hWnd, IDC_DELETE), TRUE);
	}
}


BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	switch (Msg) {
	case WM_INITDIALOG:
		Buffer = new TBuffer(hWnd);
		
		CommonTools::InitializeListView(hWnd);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXIT:
			DestroyWindow(hWnd);
			return TRUE;
		case IDC_BROWSE:
			HandleBrowse(hWnd);
			return TRUE;

		case IDC_START:
			HandleStart(hWnd);
			return TRUE;
		case IDC_DELETE:
			HandleDelete(hWnd);
			return TRUE;
		case IDC_KILLBTN:
			HandleKill(hWnd);
			return TRUE;
		}
		return FALSE;
	case WM_DESTROY:
		delete Buffer;
		PostQuitMessage(0);
		return TRUE;
	case WM_UPDATELIST:
		UpdateList(hWnd, wParam);

		return TRUE;
	case WM_NOTIFY:
		HandleBtns(hWnd);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;
	}
	return FALSE;
}