#include "../include/Internal/Types.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/Utilities.hpp"
//#include "../include/Internal/EvaluationContext.hpp"
using namespace Internal;
namespace{
struct Evaluator : public boost::static_visitor<>
{
private:
    EvaluationContext::StackWrapper& m_Stack;
    EvaluationContext& m_EC;
public:
    Evaluator(EvaluationContext::StackWrapper& Stack,EvaluationContext& EC):
        m_Stack(Stack),
        m_EC(EC)
    {}
    void operator()(long long val)
    {
        m_Stack.Push(val);
    }
    void operator()(double val)
    {
        m_Stack.Push(val);
    }
    void operator()(const boost::shared_ptr<IEvaluable>& Evaluable)
    {
        Evaluable->Eval(m_EC);
    }
    void operator()(const std::string& s)
    {
        m_Stack.Push(s);
    }
    void operator()(const Reference& Ref)
    {
        m_Stack.Push(Ref);
    }
    void operator()(const NullReference& Ref)
    {
        m_Stack.Push(Ref);
    }
};
}//ns
void EvaluationContext::EvalScope()
{
    //We don't need another Evaluator for each new scope, because the new instructions will be pushed on top of the scopeinstructions stack and will therefore
    //automatically used by the current Evaluator
    /*if( !m_Evaluating )
    {
        //Exceptionsave reset auf m_Evaluating. Who knows what's gonna throw out of the instructions
        struct RAIISetter
        {
            bool& m_b;
            RAIISetter(bool& b): m_b(b)
            {
                m_b = true;
            }
            ~RAIISetter()
            {
                m_b = false;
            }
        };
        RAIISetter ScopedBool(m_Evaluating);*/
        Evaluator Ev(Stack,*this);

        foreach(Token& T,*m_ScopeInstructions.top().second)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(Ev,T);
        }
    //}
}
