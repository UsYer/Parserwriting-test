#include <windows.h>
#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include "MariusParser.hpp"

//Source: http://dslweb.nwnexus.com/~ast/dload/guicon.htm
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole()
{
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
    std::ios_base::sync_with_stdio();

}

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    std::string args = lpszArgument;
    if( args.empty() )
    {
        RedirectIOToConsole();
        std::string Input;
        bool exit = false;

//        std::cout << "Version: " << AutoVersion::FULLVERSION_STRING;
//        #if (defined(_WIN64) || defined(WIN64) || defined(__WIN64) || defined(__WIN64__)) && defined(__MINGW64__) && (defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__))
//        std::cout << " 64bit\n";
//        #else
//        std::cout << " 32bit\n";
//        #endif
        MariusParser MP;
        do
        {
            std::cout << "\nEnter expression:" << std::endl;
            std::getline(std::cin, Input);
            if( Input.empty() )
            {
                continue;
            }
            if( Input == "q" )
            {
                exit = true;
                break;
            }
            try
            {
                BenchData benchData;
                MP.Evaluate(Input,&benchData);

                std::cout << "\ninput: " << Input << "\n";
                std::cout << "Tokenizing:\t" << benchData.TicksTokenize << "\nParsing:\t" << benchData.TicksParse
                          << "\nEval:\t\t" << benchData.TicksEval << "\nAll:\t\t" << benchData.TicksSum << "\n";
            }
            catch(Exceptions::ParseError& e)
            {
                std::cerr << "\nerror: " << e.what() << std::endl;
                continue;
            }
            catch(Exceptions::RuntimeException& e)
            {
                std::cerr << "\n" << e.what() << ": " << e.Name() << std::endl;
                continue;
            }
            catch(std::logic_error& e)
            {
                std::cerr << "\nlogic error: " << e.what() << std::endl;
                continue;
            }
            catch(...)
            {
                std::cerr << "error: Unexpected internal error, quitting" << std::endl;
                exit = true;
            }
        }
        while( !exit );
    }
    else
    {
        Sleep(5000);
        std::ifstream file(args,std::ios::in);
        if( !file )
        {
            RedirectIOToConsole();
            std::cerr << "error opening file: " << args << "\n";
        }
        std::string input;
        MariusParser MP;
        while(std::getline(file, input))
        {
            try
            {
                MP.Evaluate(input);
            }
            catch(Exceptions::ParseError& e)
            {
                std::cerr << "\nerror: " << e.what() << std::endl;
                break;
            }
            catch(Exceptions::RuntimeException& e)
            {
                std::cerr << "\n" << e.what() << ": " << e.Name() << std::endl;
                break;
            }
            catch(std::logic_error& e)
            {
                std::cerr << "\nlogic error: " << e.what() << std::endl;
                break;
            }
            catch(...)
            {
                std::cerr << "error: Unexpected internal error, quitting" << std::endl;
                break;
            }
        }
        std::cin.get();
    }
    return 0;
}
