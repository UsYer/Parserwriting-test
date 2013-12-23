#include "../../include/Platform.hpp"
#include <mutex>
#include <iostream>
#ifdef PLAT_WIN
#include "../include/Internal/Plat_Win/RedirectIO.hpp"
#endif

namespace Platform
{
static std::once_flag flag;

void OpenConsole()
{
    std::call_once(flag, Internal::Platform::RedirectIOToConsole);
}
std::ostream& Out()
{
    std::call_once(flag, Internal::Platform::RedirectIOToConsole);
    return nowide::cout;
}
std::istream& In()
{
    std::call_once(flag, Internal::Platform::RedirectIOToConsole);
    return nowide::cin;
}
}
