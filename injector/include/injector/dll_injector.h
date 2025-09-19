#pragma once
#include <Windows.h>
#include <string>

class dll_injector
{
public:
    bool inject(DWORD process_id, const std::wstring& dll_path);
};