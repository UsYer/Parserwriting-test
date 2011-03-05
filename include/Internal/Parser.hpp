#ifndef PARSER_H
#define PARSER_H
#include <stack>
#include <queue>
#include <string>
#include <boost/shared_ptr.hpp>
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
    None = ~(FunctionCall | BracketPairWithinFunctionCall)
};

enum struct TokenType
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
    //Composed Tokentypes for convenience:
    Value = Long | Double | KeywordWithValue | Identifier
};
class Parser;
struct ParserContext
{
    inline ParserContext(Parser& P, std::deque<UnparsedToken>* InputQueue = 0);
    ///Provide a pointer to a new Outputqueue if you want to redirect the parsed tokens to somewhere else as the standard outputqueue
    inline void SetUpNewScope(std::deque<Token>* NewOutputQueue = 0);
    inline bool EndScope();
    void Parse(const UnparsedToken&) const; //defined in the cpp-file
//    inline const std::map<std::string,std::function<bool(ParserContext&)>>& Keywords() const;
    inline const boost::shared_ptr<IOperator>& OpeningBracket() const;
    inline const boost::shared_ptr<IOperator>& ClosingBracket() const;
    inline std::stack<boost::shared_ptr<IOperator>>& ExpectedBracket() const;
    inline const boost::shared_ptr<IOperator>& ArgumentSeperator() const;
    inline const std::vector<std::pair<boost::shared_ptr<IOperator>,boost::shared_ptr<IOperator>>>& BracketOperators() const;
    inline std::stack<OperatorOrFuncCaller>& OperatorStack() const;
    inline std::deque<UnparsedToken>& InputQueue() const;
    inline void InputQueue(std::deque<UnparsedToken>* NewInputQueue);
    inline std::deque<Token>& OutputQueue() const;
    inline StateSaver<ParserState>& State() const;
    inline TokenType& LastToken() const;
    inline TokenType& UnexpectedToken() const;
    inline std::stack<bool>& ArgExistsStack() const;
    inline std::stack<unsigned>& ArgCounterStack() const;
    private:
    Parser& m_Parser;
    std::deque<UnparsedToken>* m_InputQueue;
    std::stack<std::deque<Token>*> m_OutputQueues;
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

    std::queue<Token> GetOutput() const
    {
        return std::queue<Token>(m_OutputQueue);
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
    void RegisterBracketOperator(const boost::shared_ptr<IOperator>& OpeningBracket, const boost::shared_ptr<IOperator>& ClosingBracket)
    {
        m_BracketOperators.push_back(std::make_pair(OpeningBracket, ClosingBracket));
    }

protected:
private:
    //Fields for parsed Tokens:
    std::stack<std::stack<OperatorOrFuncCaller>> m_OperatorStack;
    std::deque<Token> m_OutputQueue;
//    std::map<std::string,std::function<bool(ParserContext&)>> m_Keywords;
    boost::shared_ptr<IOperator> m_OpeningBracket;
    boost::shared_ptr<IOperator> m_ClosingBracket;
    boost::shared_ptr<IOperator> m_ArgumentSeperator;
    std::vector<std::pair<boost::shared_ptr<IOperator>/*Openingbracket*/,boost::shared_ptr<IOperator/*Closingbracket*/>>> m_BracketOperators;

    friend struct Visitor;
    struct Visitor;

    std::stack<TokenType> m_LastToken;
    std::stack<TokenType> m_UnexpectedToken;
    std::stack<StateSaver<ParserState>> m_State;
    std::stack<std::stack<bool>> m_ArgExistsStack;
    std::stack<std::stack<unsigned>> m_ArgCounterStack;
    //Keeps track of the expected brackets. If there are still brackets after completly parsing the input at least one closing bracket is missing
    std::stack<boost::shared_ptr<IOperator/*Closingbracket*/>> m_ExpectedBracket;
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
inline void ParserContext::SetUpNewScope(std::deque<Token>* NewOutputQueue)
{
    if( NewOutputQueue )
    {
        m_OutputQueues.push(NewOutputQueue);
        m_PopOutputQueue.push(true);
    }
    else
        m_PopOutputQueue.push(false);
    m_Parser.m_OperatorStack.push(std::stack<OperatorOrFuncCaller>());
    m_Parser.m_LastToken.push(TokenType::None);
    m_Parser.m_UnexpectedToken.push(TokenType::None);
    m_Parser.m_State.push(ParserState::None);

    std::stack<unsigned> InitialArgCounter;
    InitialArgCounter.push(0);
    m_Parser.m_ArgCounterStack.push(InitialArgCounter);

    std::stack<bool> InitialArgExist;
    InitialArgExist.push(false);
    m_Parser.m_ArgExistsStack.push(InitialArgExist);
}
inline bool ParserContext::EndScope()
{
    //Before doing anything, we first check that we aren't in the global scope, which means, that there hasn't been set up a new scope so far
    if( m_OutputQueues.size() <= 1 || m_Parser.m_OperatorStack.size() <= 1 || m_Parser.m_LastToken.size() <= 1 ||
        m_Parser.m_UnexpectedToken.size() <= 1 || m_Parser.m_State.size() <= 1 || m_Parser.m_ArgCounterStack.size() <= 1 ||
        m_Parser.m_ArgExistsStack.size() <= 1 )
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
    m_Parser.m_ArgCounterStack.pop();
    m_Parser.m_ArgExistsStack.pop();
    return true;
}
//inline const std::map<std::string,std::function<bool(ParserContext&)>>& ParserContext::Keywords() const
//{
//    return m_Parser.m_Keywords;
//}
inline const boost::shared_ptr<IOperator>& ParserContext::OpeningBracket() const
{
    return m_Parser.m_OpeningBracket;
}
inline const boost::shared_ptr<IOperator>& ParserContext::ClosingBracket() const
{
    return m_Parser.m_ClosingBracket;
}
inline std::stack<boost::shared_ptr<IOperator>>& ParserContext::ExpectedBracket() const
{
    return m_Parser.m_ExpectedBracket;
}
inline const boost::shared_ptr<IOperator>& ParserContext::ArgumentSeperator() const
{
    return m_Parser.m_ArgumentSeperator;
}
inline const std::vector<std::pair<boost::shared_ptr<IOperator>,boost::shared_ptr<IOperator>>>& ParserContext::BracketOperators() const
{
    return m_Parser.m_BracketOperators;
}
inline std::stack<OperatorOrFuncCaller>& ParserContext::OperatorStack() const
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
inline std::deque<Token>& ParserContext::OutputQueue() const
{
    assert(!m_OutputQueues.empty());
    return *m_OutputQueues.top();
}
inline StateSaver<ParserState>& ParserContext::State() const
{
    return m_Parser.m_State.top();
}
inline TokenType& ParserContext::LastToken() const
{
    return m_Parser.m_LastToken.top();
}
inline TokenType& ParserContext::UnexpectedToken() const
{
    return m_Parser.m_UnexpectedToken.top();
}
inline std::stack<bool>& ParserContext::ArgExistsStack() const
{
    return m_Parser.m_ArgExistsStack.top();
}
inline std::stack<unsigned>& ParserContext::ArgCounterStack() const
{
    return m_Parser.m_ArgCounterStack.top();
}
}//ns Internal
#endif // PARSER_H
