#ifndef KEYWORDFUNCTION_HPP_INCLUDED
#define KEYWORDFUNCTION_HPP_INCLUDED
#include "RuntimeFunc.hpp"
#include "ParseUtilities.hpp"
namespace Internal
{
namespace Keyword
{
namespace Detail
{

struct ParseFuncBeginning : public boost::static_visitor<std::string>
{
    const ParserContext& m_Context;
    ParseFuncBeginning(const ParserContext& Context):
        m_Context(Context)
    {

    }
    std::string operator()(long long) const
    {
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(double) const
    {
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(const std::string& Identifier ) const
    {
        m_Context.InputQueue().pop_front();
        return Identifier;
    }
    std::string operator()(const boost::shared_ptr<IOperator>& Op ) const
    {
        if( *m_Context.OpeningBracket() == *Op )
        {
            return "";
        }
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(const Parsable& P) const
    {
        if( P.Representation == *m_Context.OpeningBracket() )
        {
            return "";
        }
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
};

class FuncRegistrar : public IEvaluable
{
    boost::shared_ptr<IFunction> m_Func;
    public:
    FuncRegistrar(const boost::shared_ptr<IFunction>& Func):
        IEvaluable("","FuncRegistrar"),
        m_Func(Func)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( EC.Signal(SignalType::FuncCall) )
        {
            EC.DropSignal();
            throw std::logic_error("Named function definition is not allowed in a function call");
        }
        (*EC.Scope())[m_Func->Representation()] = m_Func;
    }
};

} //namespace Detail

/* TODO (Marius#9#): Implement returning values from functions
example: function test([args...]) return [...] end
1. This needs a reworked Evaluation mechanism. At least a way to end the current Evaluationscope is mandatory to support returns
*/
///Parses a function defined at runtime
void Function(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"func\"");
    //Check the identifier of the funtion:
    std::string Identifier = boost::apply_visitor(Detail::ParseFuncBeginning(Context),Context.InputQueue().front());
    if( Context.State() == ParserState::FunctionCall && !Identifier.empty() )
        throw std::logic_error("Named function definition is not allowed in a function call");
    #ifdef DEBUG
    std::cout << "FuncName: " << Identifier << "\n";
    #endif
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected opening bracket");
    //Parse the argumentlist of the function
    //But first swallow the opening bracket
    boost::apply_visitor(SwallowOperator(Context,Context.OpeningBracket()),Context.InputQueue().front());
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected input after opening bracket");
    //Now we start parsing the arguments
    ParseArgumentList ArgListParser(Context);
    while( !Context.InputQueue().empty() && !ArgListParser.m_Finished)
    {
        boost::apply_visitor(ArgListParser,Context.InputQueue().front());
        Context.InputQueue().pop_front();
    }
    if( ArgListParser.m_Expected != TokenType::None )
        throw std::logic_error("Missung input in argument list; expected closing bracket\n");

    #ifdef DEBUG
        std::cout << "No. of args: " << ArgListParser.m_Args.size() << "\n";
        std::cout << "Args: ";
        for( size_t i = 0; i < ArgListParser.m_Args.size(); i++ )
        {
            std::cout << ArgListParser.m_Args[i] + ", ";
        }
    #endif
    auto RTFunc = boost::make_shared<RuntimeFunc>("",Identifier,ArgListParser.m_Args,0);
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    if( Identifier.empty() )
        Context.OutputQueue().push_back(boost::make_shared<ValueHolder>(RTFunc));
    else
        Context.OutputQueue().push_back(boost::make_shared<Detail::FuncRegistrar>(RTFunc));
    Context.LastToken() = TokenType::KeywordWithValue;
    Context.SetUpNewScope(&RTFunc->m_FuncInstructions);
    Context.State() = ParserState::FuncDef;
}
}//namespace Keyword
}//namespace Internal


#endif // KEYWORDFUNCTION_HPP_INCLUDED
