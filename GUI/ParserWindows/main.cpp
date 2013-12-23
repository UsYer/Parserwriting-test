#include <windows.h>
#include "nowide/args.hpp"
#include "nowide/fstream.hpp"
#include "nowide/iostream.hpp"
#include <io.h>
#include <fcntl.h>
#include "MariusParser.hpp"

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{

    int argc = 1;
    char** argv = nullptr;
    nowide::args a(argc,argv);
    std::string args;
    if( argc > 1 )
        args = argv[1];

    if( args.empty() )
    {
        Platform::OpenConsole();
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
            Platform::Out() << "\nEnter expression:" << std::endl;
            std::getline(Platform::In(), Input);
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

                Platform::Out() << "\ninput: " << Input << "\n";
                Platform::Out() << "Tokenizing:\t" << benchData.TicksTokenize << "\nParsing:\t" << benchData.TicksParse
                          << "\nEval:\t\t" << benchData.TicksEval << "\nAll:\t\t" << benchData.TicksSum << "\n";
            }
            catch(Exceptions::ParseError& e)
            {
                Platform::Out() << "\nerror: " << e.what() << std::endl;
                continue;
            }
            catch(Exceptions::RuntimeException& e)
            {
                Platform::Out() << "\n" << e.what() << ": " << e.Name() << std::endl;
                continue;
            }
            catch(std::logic_error& e)
            {
                Platform::Out() << "\nlogic error: " << e.what() << std::endl;
                continue;
            }
            catch(...)
            {
                Platform::Out() << "error: Unexpected internal error, quitting" << std::endl;
                exit = true;
            }
        }
        while( !exit );

        Platform::In().get();

    }
    else
    {
        #ifdef DEBUG
        Platform::OpenConsole();
        #endif // DEBUG
        nowide::ifstream file(args.c_str(),std::ios::in);
        if( !file )
        {
            Platform::Out() << "error opening file: " << args << "\n";
            Platform::In().get();

        }
        else
        {
            std::string input;
            file.seekg(0, std::ios::end);
            input.reserve(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&input[0],input.size());
            MariusParser MP;
            std::string temp;
            while(std::getline(file, temp))
            {
                input += temp;
                input += "\n";
            }
//            {
                try
                {
                    MP.Evaluate(input);
                }
                catch(Exceptions::ParseError& e)
                {
                    Platform::Out() << "\nerror: " << e.what() << std::endl;
                    Platform::In().get();

//                    break;
                }
                catch(Exceptions::RuntimeException& e)
                {
                    Platform::Out() << "\n" << e.what() << ": " << e.Name() << std::endl;
                    Platform::In().get();

//                    break;
                }
                catch(std::logic_error& e)
                {
                    Platform::Out() << "\nlogic error: " << e.what() << std::endl;
                    Platform::In().get();

//                    break;
                }
                catch(...)
                {
                    Platform::Out() << "error: Unexpected internal error, quitting" << std::endl;
                    Platform::In().get();

//                    break;
                }
//            }
            // Keeping the window open for debugging
            Platform::In().get();
        }
    }
    return 0;
}
