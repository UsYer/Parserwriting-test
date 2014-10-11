#ifndef PARSER_H
#define PARSER_H
#include <stack>
#include <queue>
#include <string>
#include <memory>
#include "Tokens.hpp"
#include "IOperator.hpp"
#include "IFunction.hpp"
#include "StateSaver.hpp"
namespace Internal
{
enum struct ParserState
{
    FunctionCall,
    Assignment,
    BracketPairWithinFunctionCall,
    FuncDef,
    TryBlock,
    CatchBlock,
    None = ~(FunctionCall | BracketPairWithinFunctionCall)
};

enum struct TokenTypeOld : unsigned
{
    Long            = (1u << 0),
    Double          = (1u << 1),
    OpUnaryPrefix   = (1u << 2),
    OpUnaryPostfix  = (1u << 3),
    OpBinary        = (1u << 4),
    OpeningBracket  = (1u << 5),
    ClosingBracket  = (1u << 6),
    Identifier      = (1u << 7),
    Assignment      = (1u << 8),
    ArgSeperator    = (1u << 9),
    KeywordWithValue = (1u << 10), //Keyword that produces a new value, like function, null ...
    Keyword         = (1u << 11),
    None            = (1u << 12),
    EOL             = (1u << 13),
    String          = (1u << 14),
    //Composed Tokentypes for convenience:
    Value = Long | Double | KeywordWithValue | Identifier | String
};

inline TokenTypeOld operator|(TokenTypeOld Lhs, TokenTypeOld Rhs)
{
    return static_cast<TokenTypeOld>(static_cast<unsigned>(Lhs) | static_cast<unsigned>(Rhs));
}
inline TokenTypeOld operator&(TokenTypeOld Lhs, TokenTypeOld Rhs)
{
	return static_cast<TokenTypeOld>(static_cast<unsigned>(Lhs)& static_cast<unsigned>(Rhs));
}
class Parser;
struct ParserContext
{
    inline ParserContext(Parser& P, std::deque<UnparsedToken>* InputQueue = 0);
    ///Provide a pointer to a new Outputqueue if you want to redirect the parsed tokens to somewhere else as the standard outputqueue
	inline void SetUpNewScope(std::deque<ParsedToken>* NewOutputQueue = 0);
    inline bool EndScope();
    void Parse(const UnparsedToken&) const; //defined in the cpp-file
    inline void ThrowIfUnexpected(TokenTypeOld TType, const std::string& ErrMessage = "Unexpected Token") const;
    inline const Types::Operator& OpeningBracket() const;
    inline const Types::Operator& ClosingBracket() const;
    inline std::stack<Types::Operator>& ExpectedBracket() const;
    inline const Types::Operator& ArgumentSeperator() const;
    inline const std::vector<std::pair<Types::Operator,Types::Operator>>& BracketOperators() const;
    inline std::stack<Types::Operator>& OperatorStack() const;
    inline std::deque<UnparsedToken>& InputQueue() const;
    inline void InputQueue(std::deque<UnparsedToken>* NewInputQueue);
	inline std::deque<ParsedToken>& OutputQueue() const;
    inline StateSaver<ParserState>& State() const;
    inline TokenTypeOld& LastToken() const;
    inline TokenTypeOld& UnexpectedToken() const;
	inline bool IsLastTokenValue() const;
    private:
    Parser& m_Parser;
    std::deque<UnparsedToken>* m_InputQueue;
	std::stack<std::deque<ParsedToken>*> m_OutputQueues;
    std::stack<bool> m_PopOutputQueue;
};
/**
@brief Parses a given String to a postfix represantation(Reversed Polish Notation)
*/
class Parser
{
public:
    friend struct ParserContext;
    Parser();
    /** Default destructor */
    ~Parser() = default;

    void Parse( std::deque<UnparsedToken> TokExpr );

	std::deque<ParsedToken> GetOutput() const
    {
        return m_OutputQueue;
    }
    void Clear()
    {
        while( m_OperatorStack.size() > 1 )
            m_OperatorStack.pop();
        while( !m_OperatorStack.top().empty() )
            m_OperatorStack.top().pop();
        m_OutputQueue.clear();
        ResetStates();
    }
    void RegisterBracketOperator(const std::shared_ptr<IOperator>& OpeningBracket, const std::shared_ptr<IOperator>& ClosingBracket)
    {
        m_BracketOperators.push_back(std::make_pair(OpeningBracket, ClosingBracket));
    }

protected:
private:
    //Fields for parsed Tokens:
    std::stack<std::stack<Types::Operator>> m_OperatorStack;
	std::deque<ParsedToken> m_OutputQueue;
    Types::Operator m_OpeningBracket;
    Types::Operator m_ClosingBracket;
    Types::Operator m_ArgumentSeperator;
    std::vector<std::pair<Types::Operator/*Openingbracket*/,Types::Operator/*Closingbracket*/>> m_BracketOperators;

    friend struct Visitor;
    struct Visitor;

    std::stack<TokenTypeOld> m_LastToken;
    std::stack<TokenTypeOld> m_UnexpectedToken;
    std::stack<StateSaver<ParserState>> m_State;
    //Keeps track of the expected brackets. If there are still brackets after completely parsing the input at least one closing bracket is missing
    std::stack<Types::Operator/*Closingbracket*/> m_ExpectedBracket;
    ParserContext m_Context;
    void ResetStates()
    {
        while( m_Context.EndScope() );
            //No-Op
        while( !m_ExpectedBracket.empty() )
            m_ExpectedBracket.pop();
    }
};
inline ParserContext::ParserContext(Parser& P, std::deque<UnparsedToken>* InputQueue):
    m_Parser(P),
    m_InputQueue(InputQueue)
{
    m_OutputQueues.push(&m_Parser.m_OutputQueue);
    m_PopOutputQueue.push(false);
}
inline void ParserContext::SetUpNewScope(std::deque<ParsedToken>* NewOutputQueue)
{
    if( NewOutputQueue )
    {
        m_OutputQueues.push(NewOutputQueue);
        m_PopOutputQueue.push(true);
    }
    else
        m_PopOutputQueue.push(false);
    m_Parser.m_OperatorStack.push(std::stack<Types::Operator>());
    m_Parser.m_LastToken.push(TokenTypeOld::None);
    m_Parser.m_UnexpectedToken.push(TokenTypeOld::None);
    m_Parser.m_State.push(ParserState::None);
}
inline bool ParserContext::EndScope()
{
    //Before doing anything, we first check that we aren't in the global scope, which means, that there hasn't been set up a new scope so far
    if( m_OutputQueues.size() <= 1 || m_Parser.m_OperatorStack.size() <= 1 || m_Parser.m_LastToken.size() <= 1 ||
        m_Parser.m_UnexpectedToken.size() <= 1 || m_Parser.m_State.size() <= 1 )
        return false;
    if( m_PopOutputQueue.top() )
    {
        m_OutputQueues.pop();
    }
    m_PopOutputQueue.pop();
    m_Parser.m_OperatorStack.pop();
    m_Parser.m_LastToken.pop();
    m_Parser.m_UnexpectedToken.pop();
    m_Parser.m_State.pop();
    return true;
}
void ParserContext::ThrowIfUnexpected(TokenTypeOld TType, const std::string& ErrMessage) const
{
    if( (m_Parser.m_UnexpectedToken.top() & TType) == TType )
    {
        m_Parser.m_UnexpectedToken.top() = TokenTypeOld::None;
        throw std::logic_error(ErrMessage);
    }
}
inline const std::shared_ptr<IOperator>& ParserContext::OpeningBracket() const
{
    return m_Parser.m_OpeningBracket;
}
inline const std::shared_ptr<IOperator>& ParserContext::ClosingBracket() const
{
    return m_Parser.m_ClosingBracket;
}
inline std::stack<std::shared_ptr<IOperator>>& ParserContext::ExpectedBracket() const
{
    return m_Parser.m_ExpectedBracket;
}
inline const std::shared_ptr<IOperator>& ParserContext::ArgumentSeperator() const
{
    return m_Parser.m_ArgumentSeperator;
}
inline const std::vector<std::pair<std::shared_ptr<IOperator>,std::shared_ptr<IOperator>>>& ParserContext::BracketOperators() const
{
    return m_Parser.m_BracketOperators;
}
inline std::stack<Types::Operator>& ParserContext::OperatorStack() const
{
    return m_Parser.m_OperatorStack.top();
}
inline std::deque<UnparsedToken>& ParserContext::InputQueue() const
{
    assert(m_InputQueue);
    return *m_InputQueue;
}
inline void ParserContext::InputQueue(std::deque<UnparsedToken>* NewInputQueue)
{
    assert(NewInputQueue);
    m_InputQueue = NewInputQueue;
}
inline std::deque<ParsedToken>& ParserContext::OutputQueue() const
{
    assert(!m_OutputQueues.empty());
    return *m_OutputQueues.top();
}
inline StateSaver<ParserState>& ParserContext::State() const
{
    return m_Parser.m_State.top();
}
inline TokenTypeOld& ParserContext::LastToken() const
{
    return m_Parser.m_LastToken.top();
}
inline TokenTypeOld& ParserContext::UnexpectedToken() const
{
    return m_Parser.m_UnexpectedToken.top();
}
inline bool ParserContext::IsLastTokenValue() const
{
	return (TokenTypeOld::Value & LastToken()) == LastToken();
}

}//ns Internal
#endif // PARSER_H
