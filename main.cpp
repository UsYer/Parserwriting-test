#include <iostream>
#include "version.h"
#include "include/MariusParser.hpp"

/* FIXME (Marius#7#): Function calls are still very buggy. Related: AnonFuncDefInFuncCall fails because of this */
/* TODO (Marius#9#): Implement marshaling of userside types to internal types.
Like extracting the actual value of a userside Object.
When marshaling values in make sure to dereference pointers in order to copy the argument.
 */
// TODO (Marius#3#): Implement function overloading
/* TODO (Marius#4#): Replace std-exceptions with own Exceptions that clearly represent the kind of error that happened.
E.g.: ParsingException : std::exception; EvaluationException: std::exception;
UserExeption : std::exception; the last one covers all error concerning embedding: marshaling ... */
/* TODO (Marius#4#): Decide wether exceptions or result-returnvalue should be used inside MariusParser and how the user should be informed about errors (expetions or returnvalues?) */
/* NOTE (Marius#3#): Ideas for Scope:
All variables are going into a local scope
Globals: Scope.Global["var"] or Scope.Global.var
Maybe: Accesing parent scopes Scope.Parent["var"] or Scope.Parent.var
*/

using namespace std;
void Hallo(long long e,double z)
{
    std::cout << "\nUserFunc: Hallo " << e << " und " << z << "\n";
}
void lll(long long First,long long Second,long long Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
void lld(long long First,long long Second,double Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
void ldd(long long First,double Second,double Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
void ddd(double First,double Second,double Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
void ddl(double First,double Second,long long Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
void dll(double First,long long Second,long long Third)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << std::endl;
}
Internal::CountedReference NotBuiltinReturnType(double, double,double)
{
    static Internal::MemoryController MC;
    Internal::Types::Table Tab;
    Tab[0] = 1ll;
    return MC.Save(Tab);
}
void dddl(double First,double Second,double Third,long long Fourth)
{
    std::cout << "\n0: " << First << " 1: " << Second << " 2: " << Third << " 3: " << Fourth << std::endl;
}
long long ManyArgs(double,double,double,double,double,double,double,double)
{
    return 8ll;
}
Types::Object TesterReturnMarshal(const Types::Object& Val)
{
    return Val;
}
int main()
{
    std::string Input;
    bool exit = false;
    MariusParser MP;
    MP.RegisterFunction("UserDefTestFunc","dddl",&dddl);
    MP.RegisterFunction("UserDefTestFunc","lll",&lll);
    MP.RegisterFunction("UserDefTestFunc","lld",&lld);
    MP.RegisterFunction("UserDefTestFunc","ldd",&ldd);
    MP.RegisterFunction("UserDefTestFunc","ddd",&ddd);
    MP.RegisterFunction("UserDefTestFunc","ddl",&ddl);
    MP.RegisterFunction("UserDefTestFunc","dll",&dll);
    MP.RegisterFunction("UserDefTestFunc","ManyArgs",&ManyArgs);
    MP.RegisterFunction("bla","NBRT",&NotBuiltinReturnType);
    MP.RegisterFunction("","TRM",TesterReturnMarshal);

    cout << "Version: " << AutoVersion::FULLVERSION_STRING;
    #if (defined(_WIN64) || defined(WIN64) || defined(__WIN64) || defined(__WIN64__)) && defined(__MINGW64__) && (defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__))
    cout << " 64bit";
    #else
    cout << " 32bit";
    #endif
    cout << std::endl;
    cout << "\nlong long size:\t" << sizeof(long long) << " bytes; max:\t" << std::numeric_limits<long long>::max();
    cout << "\nlong size:\t" << sizeof(long) << " bytes; max:\t" << std::numeric_limits<long>::max();
    cout << "\nint size:\t" << sizeof(int) << " bytes; max:\t" << std::numeric_limits<int>::max() << std::endl;
    do
    {
        cout << "\nEnter mathematical expression:" << endl;
        std::getline(cin, Input);
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
            MP.Evaluate(Input);
        }
        catch(Exceptions::ParseError& e)
        {
            std::cerr << std::string("\nerror: ") + e.what() << std::endl;
            continue;
        }
        catch(std::logic_error& e)
        {
            std::cerr << std::string("\nruntime error: ") + e.what() << std::endl;
            continue;
        }
        catch(...)
        {
            std::cerr << "error: Unexpected internal error, quitting" << std::endl;
            exit = true;
        }
    }
    while( !exit );
    return 0;
}
