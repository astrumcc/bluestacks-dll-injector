#pragma once
#include <string>
#include <iostream>

class logger
{
public:
    static void info(const std::wstring& message);
    static void error(const std::wstring& message, int error_code = 0);
};
