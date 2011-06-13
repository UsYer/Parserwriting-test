#ifndef KEYWORDEND_HPP_INCLUDED
#define KEYWORDEND_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
namespace Detail
{
struct ScopeEnder : public IFunction
{
    ScopeEnder():
        IFunction("","__SCOPEENDER__",0,0)
    {}
    virtual void Eval(EvaluationContext& EC)
    {
        EC.EndScope();
    }
};
}//ns Detail
void End(ParserContext& Context)
{
    if( Context.LastToken() == TokenType::OpUnaryPrefix || Context.LastToken() == TokenType::OpBinary )
        throw std::logic_error("missing argument after operator");
    //End of parsing a scope, so pop the remaining operators on the Outqueue to not miss them
    while( !Context.OperatorStack().empty() )
    {
        Context.OutputQueue().push_back(Context.OperatorStack().top());
        Context.OperatorStack().pop();
    }
//    Context.OutputQueue().push_back(boost::make_shared<FuncHolder>(boost::make_shared<Detail::ScopeEnder>()));
    Context.OutputQueue().push_back(boost::make_shared<Detail::ScopeEnder>());
    if( !Context.EndScope() )
        throw std::logic_error("Scope mismatch, end is superfluous.");
}
}//ns Keyword
}//ns Internal


#endif // KEYWORDEND_HPP_INCLUDED
