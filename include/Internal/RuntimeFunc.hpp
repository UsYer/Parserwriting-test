#ifndef RUNTIMEFUNC_HPP_INCLUDED
#define RUNTIMEFUNC_HPP_INCLUDED
#include "EvaluationContext.hpp"
namespace Internal
{
class RuntimeFunc: public IFunction
{
    public:
    std::deque<ParsedToken> m_FuncInstructions;
    RuntimeFunc(const std::string& Name, const std::string& Representation, const std::vector<std::string>& Args, unsigned ReturnCount):
        IFunction(Name, Representation, Args.size(), ReturnCount)
    {
        for( const std::string& Arg: Args )
        {
            m_LocalScope[Arg];
        }
    }
    virtual void Eval(EvaluationContext& EC) override
    {
        auto LocalScopeRef = EC.MC.Save(m_LocalScope);
        (*LocalScopeRef)["__PARENT__"] = EC.Scope(); //Save the actual parentscope before setting up the new scope because otherwise it would point to this scope: infinite recursion
        (*LocalScopeRef)["This"] = m_This;
        EC.NewScope(LocalScopeRef,&m_FuncInstructions);
        EC.EvalScope();
        EC.Stack.Push((*LocalScopeRef)["__RETURN__"]);
    }
};
}//ns Internal


#endif // RUNTIMEFUNC_HPP_INCLUDED
