// dll_injector.cpp: 定義主控台應用程式的進入點。
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include <Windows.h>
#include <iostream>

using namespace std;

TCHAR dllName[] = _T("dllhook_messagebox.dll");
int namelen = _tcslen(dllName) + 1;
int GetPidByWindow() {
	char winname[260];
	cin.getline(winname, 260);
	HWND myWindow = FindWindowA(NULL, winname);
	cout << hex << myWindow << endl;
	DWORD PID;
	GetWindowThreadProcessId(myWindow, &PID);
	if (PID == 0) {
		cout << "NotFound";
		return 0;
	}
	else {
		return PID;
	}

}

void debugPrivilege() {
	HANDLE hToken;
	bool bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (bRet) {
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
	}
}
int main()
{
	debugPrivilege();
	DWORD dwPid = GetPidByWindow();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPid);
	if (hProcess == NULL)cout << "[!] Fail to openProcess" << endl;
	auto remoteString = VirtualAllocEx(hProcess, NULL, namelen * 2, MEM_COMMIT, PAGE_EXECUTE);
	WriteProcessMemory(hProcess, remoteString, dllName, namelen * 2, NULL);
	cout << hex << remoteString << endl;
	HMODULE k32 = GetModuleHandleA("kernel32.dll");
	if (k32 == NULL)cout << "[!] Fail to laod kernel32" << endl;
	auto funcAdr = GetProcAddress(k32, "LoadLibraryW");

	HANDLE thread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)funcAdr, remoteString, NULL, NULL);
	WaitForSingleObject(thread,INFINITE);
	CloseHandle(thread);
	system("pause");
    return 0;
}

