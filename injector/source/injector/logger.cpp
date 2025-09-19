#include "injector/logger.h"
#include <Windows.h>

void logger::info(const std::wstring& message)
{
    std::wcout << L"[INFO] " << message << std::endl;
}

void logger::error(const std::wstring& message, int error_code)
{
    std::wcerr << L"[ERROR] " << message;
    if (error_code != 0)
    {
        std::wcerr << L" (code: " << error_code << L")";
    }
    std::wcerr << std::endl;
}