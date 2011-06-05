#ifndef IFUNCTION_HPP_INCLUDED
#define IFUNCTION_HPP_INCLUDED
#include "IEvaluable.hpp"
#include "Table.hpp"
namespace Internal
{
class EvaluationContext;
typedef std::function<void(EvaluationContext&)> CallbackFunction;
enum ArgCount : short
{
    Variable = -1
};
class IFunction : public IEvaluable
{
public:
    virtual ~IFunction() {}
    ///Number of arguments the function expects when called
    int ArgCount()const
    {
        return m_ArgCount;
    }
    void SuppliedArguments(const Member& Args, int ArgCount);
    void SuppliedArguments(const Member& Args);
    unsigned ReturnCount() const
    {
        return m_ReturnCount;
    }
    void This( const Types::Scope& ThisScope )
    {
        m_This = ThisScope;
    }

protected:
    IFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount):
        IEvaluable(Name,Representation),
        m_SuppliedArguments(0),
        m_This(NullReference()),
        m_ArgCount(ArgCount),
        m_ReturnCount(ReturnCount)
    {}
    unsigned m_SuppliedArguments;///< No. of supplied arguments when the function is called. Only necessary to check when m_ArgCount == Variable
    Types::Table m_LocalScope;
    Types::Scope m_This;

private:
    int m_ArgCount; //Don't change this to unsigned because the expected argcount may very well be -1 for a variable argcount
    unsigned m_ReturnCount; ///< No. of returned vars from a functioncall
};

class BindFunction : public IFunction
{
public:
    virtual ~BindFunction() {}
    BindFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount,const CallbackFunction& Func):
        IFunction(Name, Representation, ArgCount, ReturnCount),
        Function(Func)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        Function(EC);
    }
private:
    CallbackFunction Function;
};
}
#endif // IFUNCTION_HPP_INCLUDED
