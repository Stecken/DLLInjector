#include <Windows.h> //DWORD
#include <iostream>
#include <string>
#include <psapi.h> //EnumProcessModules
#include <VersionHelpers.h>
#include <atlstr.h> // CString
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/*
    Base of code -> CASUALGAMER
    Modify by Stecken, 2020.
*/

#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)
#define _CRT_SECURE_NO_WARNINGS
using namespace std;

bool ThisDirExists(LPCSTR PATH) {
    FILE* fp;
    errno_t err;
    err = fopen_s(&fp, PATH, "r");
    if (err == 0) {
        printf("\nDLL encontrada\n");
        fclose(fp);
        return true;
    }
    else {
        printf("Diretorio Inexistente.\nPrograma Finalizado");
        return false;
    }
}

BOOL InjectDLL(DWORD ProcessID, LPCSTR DLL_PATH){
    TCHAR szProcessName[MAX_PATH] = TEXT("<desconhecido>");
    LPVOID LoadLibAddy, RemoteString;
    if (!ThisDirExists(DLL_PATH)) {
        return false;
    }
    if (!ProcessID)
        return false;

    HANDLE Proc = OpenProcess(CREATE_THREAD_ACCESS, FALSE, ProcessID);

    if (!Proc){
        cout << "OpenProcess() failed: " << GetLastError() << endl;
        return false;
    }
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(Proc, &hMod, sizeof(hMod), &cbNeeded)){
        GetModuleBaseName(Proc, hMod, szProcessName,
            sizeof(szProcessName) / sizeof(TCHAR));
    }
    LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_PATH) + 1, MEM_COMMIT, PAGE_READWRITE);
    _tprintf(TEXT("\nProcesso a ser injetado:\n%s  (PID: %u)\n"), szProcessName, ProcessID);
    BOOL write = WriteProcessMemory(Proc, RemoteString, (LPVOID)DLL_PATH, strlen(DLL_PATH) + 1, NULL);
    
    HANDLE THread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, NULL, NULL);

    CloseHandle(Proc);
    cout << "DLL Injetada!" << endl;
    return true;
}

//BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    DWORD dwThreadId, dwProcessId;
    HINSTANCE hInstance;
    char String[255];
    if (!hWnd)
        return TRUE;        // Not a window
    if (!::IsWindowVisible(hWnd))
        return TRUE;        // Not visible
    if (!SendMessage(hWnd, WM_GETTEXT, sizeof(String), (LPARAM)String))
        return TRUE;        // No window title
    hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
    dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
    std::cout << "PID: " << dwProcessId << '\t' << String << '\t' << std::endl;
    return TRUE;
}

int main(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    system("title DLLInjectorStecken");
    system("color 0a");
    if (IsWindowsXPOrGreater()) {
        string PathString = "";
        DWORD PID;
        SetConsoleTextAttribute(hConsole, 10);
        cout << "*****************" << endl;
        cout << "*    Stecken    *" << endl;
        cout << "*****************\n" << endl;
        cout << "Possiveis processos:\n\n" << endl;
        EnumWindows(EnumWindowsProc, NULL);
        SetConsoleTextAttribute(hConsole, 11);
        cout << "\nInsira o ID(PID) do processo:" << endl;
        SetConsoleTextAttribute(hConsole, 15);
        cin >> PID;
        SetConsoleTextAttribute(hConsole, 11);
        cout << "\nInsira o diretório da DLL a ser injetada: " << endl;
        SetConsoleTextAttribute(hConsole, 15);
        cin >> PathString;
        SetConsoleTextAttribute(hConsole, 10);
        LPCSTR PathFinal;
        PathFinal = PathString.c_str();
        InjectDLL(PID, PathFinal);
    }
    else {
        cout << "Method not supported by OS. Terminating" << endl;
        return 0;
    }
    system("pause");
    return 0;
}