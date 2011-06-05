#ifndef KEYWORDCATCH_HPP_INCLUDED
#define KEYWORDCATCH_HPP_INCLUDED
#include "ParseUtilities.hpp"
namespace Internal
{
namespace Keyword
{
namespace Detail
{
class CatchBlock: public IFunction
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
    const std::string m_Arg;
    public:
    std::deque<Token> m_FuncInstructions;
    Types::Scope m_CatchScope; //Uses different scope from localscope because we have to preserve the scope where the catch block was created. Will be set from TryCaller
    CatchBlock(const std::string& Arg):
        IFunction("", "__CATCH__", Arg.empty()? 0 : 1, 0),
        m_Arg(Arg),
        m_CatchScope(NullReference())
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
        if( !m_Arg.empty() )
            (*m_CatchScope)[m_Arg] = m_LocalScope[0];
        FuncEvaluator FE(EC.MC,m_CatchScope);
        foreach(Token& T,m_FuncInstructions)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(FE,T);
        }
    }
};
struct TryCaller : public IEvaluable
{
    boost::shared_ptr<Detail::CatchBlock> m_Catch;
    TryCaller(const std::string& Arg):
        IEvaluable("","__TRYCALLER__"),
        m_Catch(boost::make_shared<Detail::CatchBlock>(Arg))
    {
    }
    void Eval(EvaluationContext& EC)
    {
        Types::Function TryBlock = Utilities::GetWithResolve<Types::Function>(EC,EC.EvalStack.back(),"Tryblock is not a function");
        EC.EvalStack.pop_back();
        if( TryBlock->Representation() != "__TRY__" )
            throw std::logic_error("No Tryblock");
        Types::Table CatchScope;
        CatchScope["__PARENT__"] = EC.Scope;
        m_Catch->m_CatchScope = EC.MC.Save(CatchScope);
        //Push the Catchblock on top of the stack so the tryblock can find it there
        EC.EvalStack.push_back(Types::Object(m_Catch));
        TryBlock->Eval(EC);
    }
};
struct IsTry : public boost::static_visitor<bool>
{
    bool operator()( const boost::shared_ptr<IEvaluable>& Func ) const
    {
        return *Func == "__TRYHOLDER__";
    }
    template<typename T>
    bool operator()(const T&) const
    {
        return false;
    }
};
struct HasArg : public boost::static_visitor<std::string>
{
    HasArg(const ParserContext& PC):
        m_ParserContext(PC)
    {

    }
    template <typename T>
    std::string operator()(const T&) const
    {
        throw std::logic_error("Number literals are not allowed in the argument list");
    }
    std::string operator()(const boost::shared_ptr<IOperator>& Op) const
    {
        if( *m_ParserContext.ClosingBracket() == Op->Representation() )
        {
            m_ParserContext.InputQueue().pop_front();
            return "";
        }
        else
        {
            throw std::logic_error(std::string("Unexpected operator \"") + Op->Representation() + "\"");
        }
    }
    std::string operator()(const Parsable& P) const
    {
        if( *m_ParserContext.ClosingBracket() == P.Representation )
        {
            m_ParserContext.InputQueue().pop_front();
            return "";
        }
        else
        {
            throw std::logic_error(std::string("Unexpected token \"") + P.Representation + "\"");
        }
    }
    std::string operator()(const std::string& Identifier) const
    {
        m_ParserContext.InputQueue().pop_front();
        return Identifier;
    }
    private:
    const ParserContext& m_ParserContext;
};
}//ns Detail
// TODO (Marius#8#): Add catchargument
void Catch(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"catch\"");
    if( Context.LastToken() != TokenType::KeywordWithValue || !boost::apply_visitor(Detail::IsTry(), Context.OutputQueue().back()) )
        throw std::logic_error("Missing \"try\" block before \"catch\"");
    boost::apply_visitor(SwallowOperator(Context,Context.OpeningBracket(),"Expected openingbracket after \"catch\""),Context.InputQueue().front());
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected input after opening bracket");
    std::string Arg = boost::apply_visitor(Detail::HasArg(Context),Context.InputQueue().front());
    auto TryCallFunc = boost::make_shared<Detail::TryCaller>(Arg);
    if( !Arg.empty() )
        boost::apply_visitor(SwallowOperator(Context,Context.ClosingBracket()),Context.InputQueue().front());
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    Context.OutputQueue().push_back(TryCallFunc);
    Context.LastToken() = TokenType::KeywordWithValue;
    Context.SetUpNewScope(&TryCallFunc->m_Catch->m_FuncInstructions);
}
}//ns Keyword
}//ns Internal

#endif // KEYWORDCATCH_HPP_INCLUDED
