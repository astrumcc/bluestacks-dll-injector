#include <iostream>
#include <Windows.h>
#include <TlHelp32.h> 
#include <string>


DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &processEntry)) {
            do {
                if (wcscmp(processName, processEntry.szExeFile) == 0) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnapshot, &processEntry));
        }

        CloseHandle(hSnapshot);
    }

    return processId;
}


bool InjectDLL(DWORD processId, const wchar_t* dllName) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    if (hProcess == nullptr) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return false;
    }

   
    LPVOID dllNameAddress = VirtualAllocEx(hProcess, nullptr, wcslen(dllName) * 2 + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (dllNameAddress == nullptr) {
        std::cerr << "Failed to allocate memory in process. Error code: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }


    if (!WriteProcessMemory(hProcess, dllNameAddress, dllName, wcslen(dllName) * 2 + 2, nullptr)) {
        std::cerr << "Failed to write DLL name to process memory. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, dllNameAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    
    LPVOID loadLibraryAddress = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");

    if (loadLibraryAddress == nullptr) {
        std::cerr << "Failed to get LoadLibraryW address. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, dllNameAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 4. ������� ����� � ������� ��������, ������� �������� LoadLibraryW
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, dllNameAddress, 0, nullptr);

    if (hThread == nullptr) {
        std::cerr << "Failed to create remote thread. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, dllNameAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    
    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);

   
    VirtualFreeEx(hProcess, dllNameAddress, 0, MEM_RELEASE);

    CloseHandle(hProcess);

    return true;
}

int main() {
   
    const wchar_t* targetProcessName = L"HD-Player.exe"; //это процесс эмулятора, куда инжектится dll файл
    DWORD processId = GetProcessIdByName(targetProcessName);

    if (processId == 0) {
        std::cerr << "Process Not Found!" << std::endl;
        return 1;
    }

    // 2. �������� ��� DLL (��� �������� ������� ����)
    const wchar_t* dllName = L"tools\\dllname.dll"; //создаем папку на рабочем столе, после чего мы создаем папку tools и кидаем туда файл .dll
    //также кидаем лоадер в папку!
    // 
    
    wchar_t currentDirectory[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDirectory);

    std::wstring fullDllPath = currentDirectory;
    fullDllPath += L"\\";
    fullDllPath += dllName;
  
    if (GetFileAttributesW(fullDllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "DLL not found: " << fullDllPath.c_str() << std::endl;
        return 1;
    }

   
    if (InjectDLL(processId, fullDllPath.c_str())) {
        std::cout << "DLL injected successfully!" << std::endl;
    }
    else {
        std::cerr << "DLL injection failed." << std::endl;
        return 1;
    }

    return 0;
}