
#include <windows.h>
#include <string>
#pragma once


using namespace std;

#define BUF_SIZE 100
enum TProcessState { psEmpty, psNew, psRunning, psTerminated, psError};

struct TProcessInfo {
	TProcessState State;
	string Name;
	HANDLE Handle;
	int PID;
	int UserTime, KernelTime;
};

class TBuffer
{
private:
	HWND hWnd;
	CRITICAL_SECTION cs;
	TProcessInfo Buf[BUF_SIZE];

public:
	int Count();
	int AddProcess(string AName);
	void Get(int Id, TProcessInfo& Pi); 
	void Set(int Id, const TProcessInfo Pi);
	bool DeleteBufItem(int Id);
	void TerminateProc(int Id);
	const char* ProcStateEnumToString(TProcessState state);
	TBuffer(HWND AhWnd);
	~TBuffer();

};

