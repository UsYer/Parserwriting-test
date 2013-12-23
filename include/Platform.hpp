#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED
#include <iosfwd>

namespace Platform
{

    void OpenConsole();
    std::ostream& Out();
    std::istream& In();
}

#endif // PLATFORM_HPP_INCLUDED
