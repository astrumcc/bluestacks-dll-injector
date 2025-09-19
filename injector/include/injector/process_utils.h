#pragma once
#include <Windows.h>
#include <string>

class process_utils
{
public:
    static DWORD get_process_id_by_name(const std::wstring& process_name);
};