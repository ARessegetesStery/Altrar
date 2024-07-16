#include "atrpch.h"

#include "ATROSSpec.h"

namespace ATR
{

#ifdef _WIN32
#include <windows.h>

	void OS_ChangeConsoleColor(unsigned int color)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

#endif

}