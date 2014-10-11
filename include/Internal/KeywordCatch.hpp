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
    const std::string m_Arg;
    public:
    std::deque<ParsedToken> m_FuncInstructions;
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
        EC.NewScope(m_CatchScope,&m_FuncInstructions);
        EC.EvalScope();
    }
};
struct TryCaller : public IEvaluable
{
    std::shared_ptr<Detail::CatchBlock> m_Catch;
    TryCaller(const std::string& Arg):
        IEvaluable("","__TRYCALLER__"),
        m_Catch(std::make_shared<Detail::CatchBlock>(Arg))
    {
    }
    void Eval(EvaluationContext& EC)
    {// TODO (Marius#9#): Somehow the scope has to stay above the try-catch block before assignments and so forth shall be done as if there is try-catch-block
        Types::Function TryBlock = Utilities::GetWithResolve<Types::Function>(EC,EC.Stack.Pop(),"Tryblock is not a function");
        if( TryBlock->Representation() != "__TRY__" )
            throw std::logic_error("No Tryblock");
        Types::Table CatchScope;
        CatchScope["__PARENT__"] = EC.Scope();
        m_Catch->m_CatchScope = EC.MC.Save(CatchScope);
//        m_Catch->m_CatchScope = EC.Scope();
        TryBlock->SuppliedArguments(m_Catch);
        TryBlock->Eval(EC);
    }
};
struct IsTry : public boost::static_visitor<bool>
{
    bool operator()( const std::shared_ptr<IEvaluable>& Func ) const
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
    std::string operator()(const std::shared_ptr<IOperator>& Op) const
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
		std::string copy_identifier = Identifier;
        m_ParserContext.InputQueue().pop_front();
		return copy_identifier;
    }
    private:
    const ParserContext& m_ParserContext;
};
}//ns Detail
void Catch(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"catch\"");
	if (Context.LastToken() != TokenTypeOld::KeywordWithValue || !boost::apply_visitor(Detail::IsTry(), Context.OutputQueue().back()))
        throw std::logic_error("Missing \"try\" block before \"catch\"");
    boost::apply_visitor(SwallowOperator(Context,Context.OpeningBracket(),"Expected openingbracket after \"catch\""),Context.InputQueue().front());
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected input after opening bracket");
    std::string Arg = boost::apply_visitor(Detail::HasArg(Context),Context.InputQueue().front());
    auto TryCallFunc = std::make_shared<Detail::TryCaller>(Arg);
    if( !Arg.empty() )
        boost::apply_visitor(SwallowOperator(Context,Context.ClosingBracket()),Context.InputQueue().front());
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    Context.OutputQueue().push_back(TryCallFunc);
	Context.LastToken() = TokenTypeOld::KeywordWithValue;
    Context.SetUpNewScope(&TryCallFunc->m_Catch->m_FuncInstructions);
}
}//ns Keyword
}//ns Internal

#endif // KEYWORDCATCH_HPP_INCLUDED
