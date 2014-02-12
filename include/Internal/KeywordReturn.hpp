#ifndef KEYWORDRETURN_HPP_INCLUDED
#define KEYWORDRETURN_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
namespace Detail
{
class Returner: public IOperator
{
    bool m_HasArg;
    public:
    Returner(bool HasArg):
        IOperator("", "return", -20, ArityType::UnaryPrefix, AssociativityType::None),
        m_HasArg(HasArg)
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
		if (m_HasArg)
			EC.Return(EC.Stack.Pop());
		else
			EC.Return(Types::Object(NullReference{}));
			//EC.EndScope();
    }
};
}//ns Detail
void Return(ParserContext& Context)
{
    if( Context.State() != ParserState::FuncDef )
        throw std::logic_error("\"return\" not allowed outside functions");
    if( Context.InputQueue().empty() )
        Context.Parse(std::make_shared<Detail::Returner>(false));
    else
        Context.Parse(std::make_shared<Detail::Returner>(true));
    Context.LastToken() = TokenType::OpUnaryPrefix; //this is more correct than KeywordWithValue and it avoids that IsLastTokenValue() in the Parser returns true
}
}//ns Keyword
}//ns Internal


#endif // KEYWORDRETURN_HPP_INCLUDED
