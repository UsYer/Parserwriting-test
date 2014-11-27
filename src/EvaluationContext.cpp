#include "../include/Internal/EvaluationContext.hpp"
#include "../include/Internal/Object.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/Utilities.hpp"
#include "../include/Exceptions.hpp"
using namespace Internal;
namespace{
struct Evaluator : public boost::static_visitor<>
{
private:
    EvaluationContext& m_EC;

//    struct LiteralVisitor : public boost::static_visitor<>
//    {
//        EvaluationContext& m_EC;
//        LiteralVisitor(EvaluationContext& EC):m_EC(EC)
//        {}
//        void operator()(long long val)
//        {
//            m_EC.Stack.Push(val);
//        }
//        void operator()(double val)
//        {
//            m_EC.Stack.Push(val);
//        }
//        void operator()(utf8::ustring val)const
//        {
//            m_EC.Stack.Push(val);
//        }
//    };

public:
    Evaluator(EvaluationContext& EC):
        m_EC(EC)
    {}
//    void operator()(const LiteralToken& lt)
//    {
//        LiteralVisitor lv(m_EC);
//        boost::apply_visitor(lv,lt);
//    }
	void operator()(long long val) const
    {
        m_EC.Stack.Push(val);
    }
    void operator()(double val) const
    {
        m_EC.Stack.Push(val);
    }
    void operator()(utf8::ustring val)const
    {
        m_EC.Stack.Push(val);
    }
	void operator()(const std::shared_ptr<IEvaluable>& Evaluable) const
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
	void operator()(const std::string& s) const
    {
        m_EC.Stack.Push(s);
    }
	void operator()(const Reference& Ref) const
    {
        m_EC.Stack.Push(Ref);
    }
	void operator()(const NullReference& Ref) const
    {
        m_EC.Stack.Push(Ref);
    }
};
}//ns

void EvaluationContext::StackWrapper::Push(const Types::Object& Obj) const
{
	m_EvalStack.push_back(Obj);
}
void EvaluationContext::StackWrapper::Push(const ValueToken& Obj) const
{
	m_EvalStack.push_back(Types::Object(Obj));
}
Types::Object EvaluationContext::StackWrapper::Pop() const
{
	auto Obj = m_EvalStack.back();
	m_EvalStack.pop_back();
	return Obj;
}
///Gets a reference to an item on the stack indexed by their position. Enumerated from top (0) to bottom
Types::Object& EvaluationContext::StackWrapper::Get(unsigned Pos) const
{
	return m_EvalStack.at(m_EvalStack.size() - 1 - Pos);
}
Types::Object& EvaluationContext::StackWrapper::Top() const
{
	return m_EvalStack.back();
}
void EvaluationContext::StackWrapper::Clear()
{
	m_EvalStack.clear();
}
bool EvaluationContext::StackWrapper::Empty() const
{
	return m_EvalStack.empty();
}
size_t EvaluationContext::StackWrapper::Size() const
{
	return m_EvalStack.size();
}
EvaluationContext::StackWrapper::StackWrapper(Types::Stack& EvalStack) :
m_EvalStack(EvalStack)
{}

CountedReference EvaluationContext::NewScope(const Types::Table& LocalScope, std::deque<Token>* Instructions)
{
	m_ScopeInstructions.push_back(std::make_pair(MC.Save(LocalScope), Instructions));
	return m_ScopeInstructions.back().first;
}

CountedReference EvaluationContext::NewScope(const Types::Scope& LocalScope, std::deque<Token>* Instructions)
{
	m_ScopeInstructions.push_back(std::make_pair(LocalScope, Instructions));
	return m_ScopeInstructions.back().first;
}

void EvaluationContext::SetGlobalScopeInstructions(std::deque<Token>* Instructions)
{
	m_ScopeInstructions[0].second = Instructions;
}

const CountedReference& EvaluationContext::Scope() const
{
	return m_ScopeInstructions.back().first;
}

CountedReference& EvaluationContext::Scope()
{
	return m_ScopeInstructions.back().first;
}

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

		for (Token& T : *m_ScopeInstructions.back().second)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(Ev,T);
        }
    //}
}

bool EvaluationContext::EndScope()
{
	if (m_ScopeInstructions.size() <= 1) //preserve global scope
		return false;
	m_ScopeInstructions.pop_back();
	return true;
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
        ExceptionTable["Message"] = Ex.what();
		ExceptionTable["Name"] = Ex.Name();
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
//struct ArgCounter : public boost::static_visitor<unsigned>
//{
//    ArgCounter()
//    {}
//    template <typename T>
//    unsigned operator()(const T& ) const
//    {
//        return 1;
//    }
//    unsigned operator()(const CountedReference& Ref) const
//    {
//        auto it = (*Ref).Find("__ARGCOUNT__");
//        if( it != (*Ref).KeyEnd() )
//            return boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"),it->second);
//        else
//            return 1;
//    }
//    unsigned operator()(const std::shared_ptr<IFunction>& Func) const
//    {
//        if( *Func == "__ALSM__" )
//            return 0;
//        else
//            return 1;
//    }
//};
void EvaluationContext::Call(const Internal::Types::Object& Callable, const ResolvedToken& Args)
{
	ResolvedToken FuncToken = Utilities::Resolve(*this, Callable);
    const Types::Function& Func = boost::apply_visitor(Utilities::GetFunc(*this),FuncToken);
	Call(Func, Args, Callable.This());
}
void EvaluationContext::Call(const Internal::Types::Object& Callable, const ResolvedToken& Args, long long NumOfArgs)
{
	ResolvedToken FuncToken = Utilities::Resolve(*this, Callable);
	const std::shared_ptr<IFunction>& Func = boost::apply_visitor(Utilities::GetFunc(*this), FuncToken);
	Call(Func, Args, NumOfArgs, Callable.This());
}
void EvaluationContext::Call(const Types::Function& Func, const ResolvedToken& Args, const Types::Scope& ThisScope)
{
	Call(Func, Args, boost::apply_visitor(Utilities::ArgCounter(*this), Args), ThisScope);
}
void EvaluationContext::Call(const Types::Function& Func, const ResolvedToken& Args, long long NumOfArgs, const Types::Scope& ThisScope)
{
    Func->SuppliedArguments(Args,NumOfArgs);
    Func->This(ThisScope);
    try
    { //Make sure native exceptions are properly transformed to internal exceptions
        Func->Eval(*this);
        if( Func->ReturnCount() == 0 )
        {
            EvalStack.push_back(ValueToken(NullReference())); // A function always returns something, which is null when not returning anything by definition
        }
    }
    catch( Exceptions::RuntimeException& e )
    {
        Func->This(NullReference());
        Throw(e);
        EndScope();
    }
    Func->This(NullReference()); //Very Important! Reset the this-ref to prevent a crash on shutdown. The MC gets deleted before the functions
                                //who would still hold a ref to their this-scopes. When funcs get deleted the and the ref does its decref, the MC is already gone -> Crash!

}

void EvaluationContext::Return(const Types::Object& RetVal)
{
    (*Scope())["__RETURN__"] = Utilities::Resolve(*this,RetVal);
    EndScope();
}
