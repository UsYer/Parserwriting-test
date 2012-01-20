#include "../include/Internal/Types.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/Utilities.hpp"
#include "../include/Exceptions.hpp"
using namespace Internal;
namespace{
struct Evaluator : public boost::static_visitor<>
{
private:
    EvaluationContext& m_EC;
public:
    Evaluator(EvaluationContext& EC):
        m_EC(EC)
    {}
    void operator()(long long val)
    {
        m_EC.Stack.Push(val);
    }
    void operator()(double val)
    {
        m_EC.Stack.Push(val);
    }
    void operator()(const std::shared_ptr<IEvaluable>& Evaluable)
    {
        try
        { //Make sure native exceptions are properly transformed to internal exceptions
            Evaluable->Eval(m_EC);
        }
        catch( Exceptions::RuntimeException& e )
        {
            m_EC.Throw(e);
            m_EC.EndScope();
        }
    }
    void operator()(const std::string& s)
    {
        m_EC.Stack.Push(s);
    }
    void operator()(const Reference& Ref)
    {
        m_EC.Stack.Push(Ref);
    }
    void operator()(const NullReference& Ref)
    {
        m_EC.Stack.Push(Ref);
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
        if( !m_ScopeInstructions.back().second ) //check for null pointer in the instructions
            return;
        Evaluator Ev(*this);

        for(Token& T : *m_ScopeInstructions.back().second)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(Ev,T);
        }
    //}
}
Types::Function GetCatchBlock(EvaluationContext& EC)
{
    auto it = (*EC.Scope()).Find("__CATCH__");
    #ifdef DEBUG
    unsigned Depth = 0;
    #endif
    while( it == (*EC.Scope()).KeyEnd() )
    {
        if( !EC.EndScope() )
            throw std::runtime_error("No exception handler present"); // TODO (Marius#9#): Add translation from internal exception to native exceptions. Make corresponding native classes which will be thrown
        it = (*EC.Scope()).Find("__CATCH__");
        #ifdef DEBUG
        Depth++;
        #endif
    }
    #ifdef DEBUG
    if( Depth == 0 )
        std::cout << "\nFound exception handler in the same scope\n";
    else
        std::cout << "\nFound exception handler in " << Depth << " scope(s) above throw scope\n";
    #endif
    return boost::apply_visitor(Utilities::Get<Types::Function>(),it->second);
}
void EvaluationContext::Throw(const Exceptions::RuntimeException& Ex)
{
    long long ScopeDepth = m_ScopeInstructions.size() - 1; //Get the real ScopeDepth before it gets altered while searching for the ExHandler
    auto CatchBlock = GetCatchBlock(*this);
    if( CatchBlock->ArgCount() == 0 )
    {
        CatchBlock->SuppliedArguments(NullReference(),0);
    }
    else if( CatchBlock->ArgCount() == 1 )
    {
        Types::Table ExceptionTable;
        ExceptionTable["TypeId"] = Ex.TypeId();
        ExceptionTable["ScopeDepth"] = ScopeDepth; // 0 shall be the global scope
        CatchBlock->SuppliedArguments(MC.Save(ExceptionTable));
    }
    else
        throw std::logic_error("Catch handler expects too many arguments");
    CatchBlock->Eval(*this);
}
void EvaluationContext::Throw(const CountedReference& Ex)
{
    long long ScopeDepth = m_ScopeInstructions.size() - 1; //Get the real ScopeDepth before it gets altered while searching for the ExHandler
    auto CatchBlock = GetCatchBlock(*this);
    if( CatchBlock->ArgCount() == 0 )
    {
        CatchBlock->SuppliedArguments(Ex,0);
    }
    else if( CatchBlock->ArgCount() == 1 )
    {
        (*Ex)["ScopeDepth"] = ScopeDepth; // 0 shall be the global scope
        CatchBlock->SuppliedArguments(Ex);
    }
    else
        throw std::logic_error("Catch handler expects too many arguments");
    CatchBlock->Eval(*this);
}
struct ArgCounter : public boost::static_visitor<unsigned>
{
    ArgCounter()
    {}
    template <typename T>
    unsigned operator()(const T& ) const
    {
        return 1;
    }
    unsigned operator()(const CountedReference& Ref) const
    {
        auto it = (*Ref).Find("__ARGCOUNT__");
        if( it != (*Ref).KeyEnd() )
            return boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"),it->second);
        else
            return 1;
    }
    unsigned operator()(const std::shared_ptr<IFunction>& Func) const
    {
        if( *Func == "__ALSM__" )
            return 0;
        else
            return 1;
    }
};
void EvaluationContext::Call(const Types::Object& FuncObj, const ResolvedToken& Args)
{
    ResolvedToken FuncToken(Utilities::Resolve(*this,FuncObj));
    const std::shared_ptr<IFunction>& Func(boost::apply_visitor(Utilities::Get<Types::Function>(),FuncToken));
    Call(Func,Args,FuncObj.This());
}
void EvaluationContext::Call(const Types::Function& Func, const ResolvedToken& Args, const Types::Scope& ThisScope)
{
    Func->SuppliedArguments(Args,boost::apply_visitor(ArgCounter(),Args));
    Func->This(ThisScope);
    try
    { //Make sure native exceptions are properly transformed to internal exceptions
        Func->Eval(*this);
    }
    catch( Exceptions::RuntimeException& e )
    {
        Func->This(NullReference());
        Throw(e);
        EndScope();
    }
    Func->This(NullReference()); //Very Important! Reset the this-ref to prevent a crash on shutdown. The MC gets deleted before the functions
                                //who would still hold a ref to their this-scopes. When  funcs get deleted the and the ref does it's decref, the MC is already gone -> Crash!

}

void EvaluationContext::Return(const Types::Object& RetVal)
{
    (*Scope())["__RETURN__"] = Utilities::Resolve(*this,RetVal);
    EndScope();
}
