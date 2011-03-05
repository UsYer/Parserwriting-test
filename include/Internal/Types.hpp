#ifndef TOKEN_H
#define TOKEN_H
#include <deque>
#include <stack>
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"

namespace Internal
{
class IOperator;
namespace Types
{
    typedef CountedReference Scope;
    class Object;
    typedef std::deque<Object> Stack;
    typedef boost::shared_ptr<IOperator> Operator;
}
enum struct SignalType: unsigned
{
    None = 0,
    FuncCall = 1
};
struct EvaluationContext
{
    EvaluationContext(Types::Stack& Stack, Types::Scope& Table, MemoryController& _MC):
        EvalStack(Stack),
        Scope(Table),
        MC(_MC),
        This(NullReference())
    {}
    Types::Stack& EvalStack;
    Types::Scope& Scope;
    MemoryController& MC;
    CountedReference This;
    void SetSignal(SignalType Sig)
    {
        m_Signals.push(Sig);
    }
    bool Signal(SignalType Sig) const
    {
        return !m_Signals.empty() && m_Signals.top() == Sig;
    }
    SignalType GetSignal() const
    {
        return !m_Signals.empty() ? m_Signals.top() : SignalType::None;
    }
    void DropSignal()
    {
        if( !m_Signals.empty() )
            m_Signals.pop();
    }
    void SetCustomSignal(const std::string& Signal)
    {
        m_CustomSignals.push(Signal);
    }
    bool CustomSignal(const std::string& Signal) const
    {
        return !m_CustomSignals.empty() && m_CustomSignals.top() == Signal;
    }
    std::string GetCustomSignal() const
    {
        return !m_CustomSignals.empty() ? m_CustomSignals.top() : "";
    }
    void DropCustomSignal()
    {
        if( !m_CustomSignals.empty() )
            m_CustomSignals.pop();
    }
    private:
    std::stack<std::string> m_CustomSignals;
    std::stack<SignalType> m_Signals;
};
}
/*
namespace Types
{

}*/

#endif // TOKEN_H
