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
    public:
    std::deque<Token> m_FuncInstructions;
    TryBlock():
        IFunction("", "__TRY__", 1, 0)
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
        Types::Function CatchBlock = Utilities::GetWithResolve<Types::Function>(EC,Types::Object(m_LocalScope[0]),"Catchblock is not a function");
        if( CatchBlock->Representation() != "__CATCH__" )
            throw std::logic_error("No Catchblock");
        auto LocalScopeRef = EC.MC.Save(m_LocalScope);
        (*LocalScopeRef)["__PARENT__"] = EC.Scope(); //Save the actual parentscope before setting up the new scope because otherwise it would point to this scope: infinite recursion
        (*LocalScopeRef)["This"] = m_This;
        (*LocalScopeRef)["__CATCH__"] = CatchBlock;
//        (*EC.Scope())["__CATCH__"] = CatchBlock;
        EC.NewScope(LocalScopeRef,&m_FuncInstructions);
//        EC.NewScope(EC.Scope(),&m_FuncInstructions); //the try scope still operates on the previous scope
        EC.EvalScope();
    }
};
}//ns Detail
void Try(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"try\"");
    auto TryFunc = boost::make_shared<Detail::TryBlock>();
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    Context.OutputQueue().push_back(boost::make_shared<ValueHolder>(TryFunc, "__TRYHOLDER__"));
    Context.LastToken() = TokenType::KeywordWithValue;
    Context.SetUpNewScope(&TryFunc->m_FuncInstructions);
}
}//ns Keyword
}//ns Internal
#endif // KEYWORDTRY_HPP_INCLUDED
