#ifndef PARSEUTILITIES_HPP_INCLUDED
#define PARSEUTILITIES_HPP_INCLUDED
namespace Internal
{
template <typename T>
struct Swallow : boost::static_visitor<bool>
{
    std::deque<UnparsedToken>& m_Input;
    Swallow(std::deque<UnparsedToken>& Input):
        m_Input(Input)
    {

    }
    bool operator()(const T&) const
    {
        m_Input.pop_front();
        return true; //Swallowed a token
    }
    template <typename U>
    bool operator()(const U&) const
    {
        return false; //Did not swallow a token
    }
};
struct SwallowOperator : public boost::static_visitor<>
{
    ParserContext& m_PC;
    const std::string m_ErrorMessage;
    const Types::Operator& m_OpToSwallow;
    SwallowOperator(ParserContext& PC, const Types::Operator& OpToSwallow):
        m_PC(PC),
        m_ErrorMessage("Expected operator \"" + OpToSwallow->Representation() + "\""),
        m_OpToSwallow(OpToSwallow)
    {
    }
    SwallowOperator(ParserContext& PC, const Types::Operator& OpToSwallow, const std::string& ErrorMessage):
        m_PC(PC),
        m_ErrorMessage(ErrorMessage),
        m_OpToSwallow(OpToSwallow)
    {
    }
    void operator()(const std::shared_ptr<IOperator>& Op) const
    {
        if( *m_OpToSwallow == *Op )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error(m_ErrorMessage);
    }
    void operator()(const Parsable& P) const
    {
        if( *m_OpToSwallow == P.Representation )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error(m_ErrorMessage);
    }
    template <typename T>
    void operator()(const T&) const
    {
        throw std::logic_error(m_ErrorMessage);
    }
};
/*
struct SwallowOpeningBracket : public boost::static_visitor<>
{
    ParserContext& m_PC;
    const std::string m_ErrorMessage;
    SwallowOpeningBracket(ParserContext& PC, const std::string& ErrorMessage = "Expected opening bracket"):
        m_PC(PC),
        m_ErrorMessage(ErrorMessage)
    {
    }
    void operator()(const std::shared_ptr<IOperator>& Op) const
    {
        if( *m_PC.OpeningBracket() == *Op )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error(m_ErrorMessage);
    }
    void operator()(const Parsable& P) const
    {
        if( *m_PC.OpeningBracket() == P.Representation )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error(m_ErrorMessage);
    }
    template <typename T>
    void operator()(const T&) const
    {
        throw std::logic_error(m_ErrorMessage);
    }
};*/
///See enum struct TokenType in Parser.hpp
const char* Expected(TokenType Type)
{
    switch(Type)
    {
        case TokenType::OpUnaryPrefix:
        case TokenType::OpUnaryPostfix:
        case TokenType::OpBinary:
            return "operator";
        break;
        case TokenType::Identifier:
            return "identifier";
        break;
        case TokenType::ArgSeperator:
            return "argument seperator";
        break;
        case TokenType::None:
            return "none";
        break;
        default:
            return "unknown";
        break;
    }
}
struct ParseArgumentList : public boost::static_visitor<void>
{
    ParseArgumentList(const ParserContext& ParserContext):
        m_Expected(TokenType::None),
        m_Finished(false),
        m_ParserContext(ParserContext)
    {

    }
    template <typename T>
    void operator()(T)
    {
        throw std::logic_error("Number literals are not allowed in the argument list");
    }
    void operator()(const std::shared_ptr<IOperator>& Op)
    {
        CheckForOperator(Op->Representation());
    }
    void operator()(const Parsable& P)
    {
        CheckForOperator(P.Representation);
    }
    void operator()(const std::string& Identifier)
    {
        if( m_Expected != TokenType::Identifier && m_Expected != TokenType::None ) //TokenType::None means we just started so we can't throw then
                throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + " before Identifier \"" + Identifier + "\"");
        //check for duplicate argumentnames
        auto it = std::find(m_Args.begin(),m_Args.end(),Identifier);
        if( it != m_Args.end() )
            throw std::logic_error("Duplicate argument \"" + Identifier + "\" in argument list");

        m_Args.push_back(Identifier);
        m_Expected = TokenType::ArgSeperator;
    }

    TokenType m_Expected;
    std::vector<std::string> m_Args;
    bool m_Finished;
    private:
    const ParserContext& m_ParserContext;
    void CheckForOperator(const std::string& Rep)
    {
        if( *m_ParserContext.ClosingBracket() == Rep )
        {
            if( m_Expected == TokenType::Identifier)
                throw std::logic_error(std::string("Expected Identifier; got closing bracket"));
            //the arg list is finished with the occurences of a closing bracket
            m_Expected = TokenType::None;
            m_Finished = true;
            return;
        }
        else if( *m_ParserContext.ArgumentSeperator() == Rep )
        {
            if( m_Expected != TokenType::ArgSeperator )
                throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + "; got argument seperator");

            //Argsep is expected so we swallow it and expect a new identifier
            m_Expected = TokenType::Identifier;
        }
        else
        {
            throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + "; got " + Rep);
        }
    }
};
/**
Holds a value which should not be directly evaluated.

Necessary, because the function would otherwise be evaluated directly when encoutered.
Instead, the ValueHolder gets evaluated which then pushes the real func on the stack.
Now something like this is possible: f = func() ... end
*/
class ValueHolder : public IEvaluable
{
    ValueToken m_Value;
    public:
    template< typename T>
    ValueHolder(const T& Value, const std::string& FuncHolderName = "__VALUEHOLDER__"):
        IEvaluable("", FuncHolderName),
        m_Value(Value)
    {}
    void Eval(EvaluationContext& EC)
    {
        EC.Stack.Push(m_Value);
    }
};
}//ns Internal
#endif // PARSEUTILITIES_HPP_INCLUDED
