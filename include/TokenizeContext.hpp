#ifndef TOKENIZECONTEXT_HPP_INCLUDED
#define TOKENIZECONTEXT_HPP_INCLUDED

enum struct LastCharType : unsigned
{
    UnaryPostfixOp = Internal::IOperator::ArityType::UnaryPostfix,
    UnaryPrefixOp = Internal::IOperator::ArityType::UnaryPrefix,
    BinaryOp = Internal::IOperator::ArityType::Binary,
    Start,
    Number,
    Dot,
    ArgSep,
    LikeClosingBracket,
    LikeOpeningBracket,
    Identifier,
    None,
    Unrecognized
};
namespace Internal
{
class LastCharProxy
{
    LastCharType m_LastChar[2];
public:
    LastCharProxy(LastCharType LastChar)
    {
        m_LastChar[1] = LastCharType::None;
        m_LastChar[0] = LastChar;
    }
    LastCharProxy& operator=(LastCharType LastChar)
    {
        m_LastChar[1] = m_LastChar[0];
        m_LastChar[0] = LastChar;
        return *this;
    }
    LastCharProxy(const LastCharProxy&) = delete;
    operator LastCharType() const
    {
        return m_LastChar[0];
    }
    LastCharType Before() const
    {
        return m_LastChar[1];
    }
};
}//ns Internal
struct TokenizeContext
{
    TokenizeContext(Internal::LastCharProxy& LastChar, std::deque<Internal::UnparsedToken>& OutputQueue):
        m_LastChar(LastChar),
        m_OutputQueue(OutputQueue)
    {}
    Internal::LastCharProxy& LastChar() const
    {
        return m_LastChar;
    }
    std::deque<Internal::UnparsedToken>& OutputQueue() const
    {
        return m_OutputQueue;
    }
    private:
    Internal::LastCharProxy& m_LastChar;
    std::deque<Internal::UnparsedToken>& m_OutputQueue;
};
#endif // TOKENIZECONTEXT_HPP_INCLUDED
