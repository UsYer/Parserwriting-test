#ifndef KEYWORDEND_HPP_INCLUDED
#define KEYWORDEND_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
void End(ParserContext& Context)
{
    //End of parsing a scope, so pop the remaining operators on the Outqueue to not miss them
    while( !Context.OperatorStack().empty() )
    {
        Context.OutputQueue().push_back(boost::apply_visitor(Utilities::Get<boost::shared_ptr<IOperator>>(),Context.OperatorStack().top()));
        Context.OperatorStack().pop();
    }
    if( !Context.EndScope() )
        throw std::logic_error("Scope mismatch, end is superfluous.");
}
}//ns Keyword
}//ns Internal


#endif // KEYWORDEND_HPP_INCLUDED
