#include "injector/dll_injector.h"
#include "injector/logger.h"

bool dll_injector::inject(DWORD process_id, const std::wstring& dll_path)
{
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (!process)
    {
        logger::error(L"Failed to open process", GetLastError());
        return false;
    }

    SIZE_T alloc_size = (dll_path.size() + 1) * sizeof(wchar_t);
    LPVOID remote_memory = VirtualAllocEx(process, nullptr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_memory)
    {
        logger::error(L"Failed to allocate memory in target process", GetLastError());
        CloseHandle(process);
        return false;
    }

    if (!WriteProcessMemory(process, remote_memory, dll_path.c_str(), alloc_size, nullptr))
    {
        logger::error(L"Failed to write DLL path into target process memory", GetLastError());
        VirtualFreeEx(process, remote_memory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    LPVOID load_library = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!load_library)
    {
        logger::error(L"Failed to resolve LoadLibraryW", GetLastError());
        VirtualFreeEx(process, remote_memory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    HANDLE thread = CreateRemoteThread(process, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(load_library),
        remote_memory, 0, nullptr);

    if (!thread)
    {
        logger::error(L"Failed to create remote thread", GetLastError());
        VirtualFreeEx(process, remote_memory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    WaitForSingleObject(thread, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeThread(thread, &exit_code);
    logger::info(L"Remote thread finished with code " + std::to_wstring(exit_code));

    CloseHandle(thread);
    VirtualFreeEx(process, remote_memory, 0, MEM_RELEASE);
    CloseHandle(process);

    return true;
}