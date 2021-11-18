#include "TBuffer.h"
#define WM_UPDATELIST WM_USER

TBuffer::TBuffer(HWND AhWnd)
{
	Buf[BUF_SIZE];
	hWnd = AhWnd;
	InitializeCriticalSection(&cs);
}


int TBuffer::Count()
{
	int count= 0;
	EnterCriticalSection(&cs);

	for (int i = 0; i < BUF_SIZE; i++) {
		if (Buf[i].State != TProcessState::psEmpty) {
			count++;
		}
	}

	LeaveCriticalSection(&cs);
	return  count;
}

int TBuffer::AddProcess(string AName)
{

	EnterCriticalSection(&cs);

	int index = 0;
	for ( index = 0; index < BUF_SIZE-1; index++) {
		if (Buf[index].State == TProcessState::psEmpty)break;
	}


	TProcessInfo newProcess = { TProcessState::psNew,AName};

	Buf[index] = newProcess;
	
	LeaveCriticalSection(&cs);
	return index;
}

void TBuffer::Get(int Id, TProcessInfo& Pi)
{
	EnterCriticalSection(&cs);

	Pi = Buf[Id];
	
	LeaveCriticalSection(&cs);
}

void TBuffer::Set(int Id, const TProcessInfo Pi)
{
	EnterCriticalSection(&cs);
	Buf[Id] = Pi;
	PostMessage(hWnd, WM_UPDATELIST, Id, 0);
	LeaveCriticalSection(&cs);
}

bool TBuffer::DeleteBufItem(int Id)
{
	bool result = true;
	EnterCriticalSection(&cs);
	if (Buf[Id].State != TProcessState::psRunning) {

		Buf[Id].State = TProcessState::psEmpty;
		Buf[Id].Handle = NULL;
		Buf[Id].PID = NULL;
		Buf[Id].Name = "";
		Buf[Id].KernelTime = NULL;
		Buf[Id].UserTime= NULL;

	}
	else {
		result = false;
	}
	LeaveCriticalSection(&cs);
	return result;
}

void TBuffer::TerminateProc(int Id)
{

	EnterCriticalSection(&cs);
	if (Buf[Id].State == TProcessState::psRunning) {
		TerminateProcess(Buf[Id].Handle, 0);
	}
	LeaveCriticalSection(&cs);
	
}

const char* TBuffer::ProcStateEnumToString(TProcessState state)
{
		switch (state) {
		case TProcessState::psNew:
			return "new";
		case TProcessState::psEmpty:
			return "Empty";
		case TProcessState::psError:
			return "Error";
		case TProcessState::psRunning:
			return "Running";
		case TProcessState::psTerminated:
			return "Terminated";
		} 
}



TBuffer::~TBuffer()
{
	DeleteCriticalSection(&cs);
}
