#ifndef EVALUATIONCONTEXT_HPP_INCLUDED
#define EVALUATIONCONTEXT_HPP_INCLUDED
#include <stack>
#include "Object.hpp"
namespace Exceptions
{
    class RuntimeException;
}
namespace Internal
{
enum struct SignalType: unsigned
{
    None = 0,
    FuncCall = 1
};
struct EvaluationContext
{
    struct StackWrapper
    {
        friend struct EvaluationContext;
        void Push(const Types::Object& Obj) const;
        void Push(const ValueToken& Obj) const;
        Types::Object Pop() const;
        ///Gets a reference to an item on the stack indexed by their position. Enumerated from top (0) to bottom
        Types::Object& Get(unsigned Pos) const;
        Types::Object& Top() const;
        void Clear();
        bool Empty() const;
        size_t Size() const;
    private:
        Types::Stack& m_EvalStack;
		StackWrapper(Types::Stack& EvalStack);
    };

    EvaluationContext(Types::Stack& Stack, Types::Scope& Global, MemoryController& _MC):
        EvalStack(Stack),
        Stack(EvalStack),
        GlobalScope(Global),
        MC(_MC),
        This(NullReference()),
        m_Evaluating(false)
    {
		m_ScopeInstructions.push_back(std::pair<Types::Scope, std::deque<Token>*>(Global, 0));
    }
    Types::Stack& EvalStack;
    StackWrapper Stack;
//    Types::Scope& Scope;
    Types::Scope& GlobalScope;
    MemoryController& MC;
    CountedReference This;
    
	//--Scope specific:
	CountedReference NewScope(const Types::Table& LocalScope, std::deque<Token>* Instructions);
	CountedReference NewScope(const Types::Scope& LocalScope, std::deque<Token>* Instructions);
	void SetGlobalScopeInstructions(std::deque<Token>* Instructions);
	const CountedReference& Scope() const;
	CountedReference& Scope();
    void EvalScope(); //implemented in cpp file because of use of Utilities
	bool EndScope();

    //function specific:
    void Call(const Types::Object& Callable, const ResolvedToken& Args, long long NumOfArgs);
    void Call(const Types::Object& Callable, const ResolvedToken& Args);
    void Call(const Types::Function& Callable, const ResolvedToken& Args, const Types::Scope& ThisScope = NullReference());
	void Call(const Types::Function& Callable, const ResolvedToken& Args, long long NumOfArgs, const Types::Scope& ThisScope = NullReference());
    void Return(const Types::Object& RetVal);
    //--Exception specific:
    void Throw(const Exceptions::RuntimeException& Ex);
    void Throw(const CountedReference& Ex);
    //--Signals:
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
		std::deque<std::pair<CountedReference, std::deque<Token>*>> m_ScopeInstructions;
    std::stack<std::string> m_CustomSignals;
    std::stack<SignalType> m_Signals;
    bool m_Evaluating;
};
} //ns Internal
#endif // EVALUATIONCONTEXT_HPP_INCLUDED
