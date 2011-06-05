#ifndef KEYWORDTRY_HPP_INCLUDED
#define KEYWORDTRY_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
namespace Detail
{
class TryBlock: public IFunction
{
    class FuncEvaluator : public boost::static_visitor<>
    {
        Types::Stack m_Stack;
        EvaluationContext m_EC;

        public:
        FuncEvaluator(MemoryController& MC, Types::Scope& LocalScope):
            m_EC(m_Stack, LocalScope, MC)
        {}
        void operator()(long long Val)
        {
            m_Stack.push_back(Types::Object(Val));
        }
        void operator()(double val)
        {
            m_Stack.push_back(Types::Object(val));
        }
        void operator()(const boost::shared_ptr<IEvaluable>& Evaluable)
        {
            Evaluable->Eval(m_EC);
        }
        void operator()(const std::string& s)
        {
            m_Stack.push_back(Types::Object(s));
        }
    };
    public:
    std::deque<Token> m_FuncInstructions;
    TryBlock():
        IFunction("", "__TRY__", 1, 0)
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
        CountedReference LocalScopeRef(EC.MC.Save(m_LocalScope));
        (*LocalScopeRef)["__PARENT__"] = EC.Scope;
        (*LocalScopeRef)["This"] = m_This;
        Types::Function CatchBlock = Utilities::GetWithResolve<Types::Function>(EC,EC.EvalStack.back(),"Catchblock is not a function");
        EC.EvalStack.pop_back();
        if( CatchBlock->Representation() != "__CATCH__" )
            throw std::logic_error("No Catchblock");
        (*LocalScopeRef)["__CATCH__"] = CatchBlock;
        FuncEvaluator FE(EC.MC,LocalScopeRef);
        foreach(Token& T,m_FuncInstructions)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(FE,T);
        }
    }
};
class TryHolder : public IEvaluable
{
    boost::shared_ptr<IFunction> m_Func;
    public:
    TryHolder(const boost::shared_ptr<IFunction>& Func):
        IEvaluable("","__TRYHOLDER__"),
        m_Func(Func)
    {}
    void Eval(EvaluationContext& EC)
    {
        EC.EvalStack.push_back(Types::Object(m_Func));
    }
};
}//ns Detail
void Try(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"try\"");
    auto TryFunc = boost::make_shared<Detail::TryBlock>();
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    Context.OutputQueue().push_back(boost::make_shared<Detail::TryHolder>(TryFunc));
    Context.LastToken() = TokenType::KeywordWithValue;
    Context.SetUpNewScope(&TryFunc->m_FuncInstructions);
}
}//ns Keyword
}//ns Internal
#endif // KEYWORDTRY_HPP_INCLUDED
