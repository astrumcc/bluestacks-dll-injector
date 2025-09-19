#include "injector/process_utils.h"
#include "injector/dll_injector.h"
#include "injector/logger.h"
#include <Windows.h>

int main()
{
    const std::wstring target_process = L"HD-Player.exe";
    DWORD process_id = process_utils::get_process_id_by_name(target_process);

    if (process_id == 0)
    {
        logger::error(L"Process not found");
        return 1;
    }

    std::wstring dll_path = L"you_cheat.dll";

    if (GetFileAttributesW(dll_path.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        logger::error(L"DLL not found: " + dll_path);
        return 1;
    }

    dll_injector injector;
    if (injector.inject(process_id, dll_path))
    {
        logger::info(L"DLL injected successfully");
    }
    else
    {
        logger::error(L"DLL injection failed");
        return 1;
    }

    return 0;
}