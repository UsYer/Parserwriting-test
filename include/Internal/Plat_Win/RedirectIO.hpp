#ifndef REDIRECTIO_HPP_INCLUDED
#define REDIRECTIO_HPP_INCLUDED
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "nowide/iostream.hpp"

namespace Internal
{
namespace Platform
{
//Source: http://dslweb.nwnexus.com/~ast/dload/guicon.htm
static const WORD MAX_CONSOLE_LINES = 500;

static bool RedirectIOToConsole()
{
    //only do something, if there's no console present
//    if( GetStdHandle(STD_OUTPUT_HANDLE) != nullptr )
//        return false;
     CONSOLE_SCREEN_BUFFER_INFO coninfo;

    // allocate a console for this app
        AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
        coninfo.dwSize.Y = MAX_CONSOLE_LINES;
        SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

        int hConHandle;
        HANDLE StdHandle;

    // redirect unbuffered STDOUT to the console
        StdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(StdHandle), _O_TEXT);

        FILE *fp = _fdopen( hConHandle, "w" );
        *stdout = *fp;
        setvbuf( stdout, NULL, _IONBF, 0 );

    // redirect unbuffered STDIN to the console
        StdHandle = GetStdHandle(STD_INPUT_HANDLE);
        hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(StdHandle), _O_TEXT);
        fp = _fdopen( hConHandle, "r" );
        *stdin = *fp;
        setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console
        StdHandle = GetStdHandle(STD_ERROR_HANDLE);
        hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(StdHandle), _O_TEXT);
        fp = _fdopen( hConHandle, "w" );
        *stderr = *fp;
        setvbuf( stderr, NULL, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
    // point to console as well
        std::cout.flush();
        std::ios_base::sync_with_stdio();


        nowide::cin.reset();
        nowide::cout.reset(1);
        nowide::cerr.reset(2);
        nowide::clog.reset(3);

        nowide::cin.tie(&nowide::cout);
        nowide::cerr.tie(&nowide::cout);
        nowide::clog.tie(&nowide::cout);

        nowide::cout << "RedirectIOToConsole called\n";
    return true;
}
}//ns Platform
}//ns Internal
#endif // REDIRECTIO_HPP_INCLUDED
