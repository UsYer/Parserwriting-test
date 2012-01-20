#ifndef MARIUSPARSER_H
#define MARIUSPARSER_H
#define BOOST_VARIANT_LIMIT_TYPES 8
#include "Object.hpp"
#include "Function.hpp"
#include "Internal/BindFunc.hpp"
#include "Exceptions.hpp"

struct BenchData
{
    BenchData(unsigned long long TT = 0, unsigned long long TP = 0, unsigned long long TE = 0, unsigned long long TS = 0, unsigned long long Freq = 0):
        TicksTokenize(TT),
        TicksParse(TP),
        TicksEval(TE),
        TicksSum(TS),
        Frequency(Freq)
    {
    }
    unsigned long long TicksTokenize, TicksParse, TicksEval, TicksSum, Frequency;
};

namespace Internal{ namespace Types
{
        class CallbackFunction;
}
}
class IToken;
class MariusParser
{
public:
    /** Default constructor */
    MariusParser();
    MariusParser(const MariusParser&) = delete;
    MariusParser& operator=(MariusParser) = delete;
    /** Default destructor */
    ~MariusParser();
    /** \brief Registers a token via the IToken interface
     *
     * Ownership of the pointer will be taken, so you don't have to delete it yourself
     *
     * \param Tok Operator to register
     * \return void
     *
     */
    void RegisterToken(IToken* Tok);
    void RegisterFunction(const Internal::Types::Function& p);
    void RegisterFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount,const Internal::CallbackFunction& Func);
    template<typename T>
    void RegisterFunction(const std::string& Name, const std::string& Representation, T Func)
    {
        RegisterFunction(Internal::BindFunc(Name,Representation,Func));
    }
    Types::Object Evaluate(const std::string& Input, BenchData* BD = nullptr);

    ::Types::Function GetFunction(const std::string& Identifier) const;
    ::Types::Object GlobalScope();
protected:
private:
    struct Impl;
    Impl* m_Impl;
};

#endif // MARIUSPARSER_H
