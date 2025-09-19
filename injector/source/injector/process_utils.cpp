#include "injector/process_utils.h"
#include <TlHelp32.h>

DWORD process_utils::get_process_id_by_name(const std::wstring& process_name)
{
    DWORD process_id = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &entry))
        {
            do
            {
                if (process_name == entry.szExeFile)
                {
                    process_id = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return process_id;
}