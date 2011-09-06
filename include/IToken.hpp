#ifndef ITOKEN_HPP_INCLUDED
#define ITOKEN_HPP_INCLUDED
#include <boost/make_shared.hpp>
#include "TokenizeContext.hpp"
class IToken
{
    public:
    IToken(const std::string& Rep):
        m_Rep(Rep)
    {    }
    virtual ~IToken()
    {    }
    std::string Representation() const
    {
        return m_Rep;
    }
    virtual LastCharType Tokenize(TokenizeContext&) const = 0;

    bool operator == (const std::string& s) const
    {
        return m_Rep == s;
    }
    bool operator != (const std::string& s) const
    {
        return m_Rep != s;
    }
    friend bool operator== (const std::string& lhs, const IToken& rhs);
    friend bool operator== (const char* lhs, const IToken& rhs);
    friend bool operator!= (const char* lhs, const IToken& rhs);
    protected:
    std::string m_Rep;
};
//inline is used here, because otherwise we would get some linker errors
inline bool operator== (const std::string& lhs, const IToken& rhs)
{
    return lhs == rhs.m_Rep;
}
inline bool operator== (const char* lhs, const IToken& rhs)
{
    return lhs == rhs.m_Rep;
}
inline bool operator!= (const char* lhs, const IToken& rhs)
{
    return !(lhs == rhs);
}
///Used when a usual operator should be registered, which doesn't need any special parsing
template <typename T>
class OperatorToken : public IToken
{
    public:
    OperatorToken():
        IToken(T().Representation()),// TODO (Marius#5#): Two instantiations? that's bad
        m_Operator(new T)
    {}
    OperatorToken(const std::shared_ptr<Internal::IOperator>& Op):
        IToken(Op->Representation()),
        m_Operator(Op)
    {
    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(m_Operator);
        switch( m_Operator->Arity() )// TODO (Marius#5#): Ugly, IOperator::Arity() should somehow be assignable to TokenizeContext::LastChar()
        {
        case Internal::IOperator::ArityType::UnaryPostfix:
            return LastCharType::UnaryPostfixOp;

        case Internal::IOperator::ArityType::UnaryPrefix:
            return LastCharType::UnaryPrefixOp;

        default: //Internal::IOperator::ArityType::Binary:
            return LastCharType::BinaryOp;
        }
    }
    std::shared_ptr<Internal::IOperator> GetOp() const
    {
        return m_Operator;
    }
    private:
    std::shared_ptr<Internal::IOperator> m_Operator;
};

///Used when an usual operator should be registered, which needs special parsing
class ParsedOperatorToken : public IToken
{
    public:
    template <typename T, typename U>
    ParsedOperatorToken(const T& Op, const U& ParseFunc):
        IToken(Op.Representation()),
        m_Arity(Op.Arity()),
        m_ParseFunc(Op.Representation(), ParseFunc)
    {
    }
    template <typename U>
    ParsedOperatorToken(const std::shared_ptr<Internal::IOperator>& Op, const U& ParseFunc):
        IToken(Op->Representation()),
        m_Arity(Op->Arity()),
        m_ParseFunc(Op->Representation(), ParseFunc)
    {
    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(m_ParseFunc);
        switch( m_Arity )// TODO (Marius#5#): Ugly, IOperator::Arity() should somehow be assignable to TokenizeContext::LastChar()
        {
        case Internal::IOperator::ArityType::UnaryPostfix:
            return LastCharType::UnaryPostfixOp;

        case Internal::IOperator::ArityType::UnaryPrefix:
            return LastCharType::UnaryPrefixOp;

        default: //Internal::IOperator::ArityType::Binary:
            return LastCharType::BinaryOp;
        }
    }
    private:
    Internal::IOperator::ArityType m_Arity;
    Parsable m_ParseFunc;
};

template <typename T>
class GenericOpeningBracketToken : public OperatorToken<T>
{
    public:
    GenericOpeningBracketToken():
        OperatorToken<T>(std::make_shared<T>())
    {}
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(OperatorToken<T>::GetOp());
        return LastCharType::LikeOpeningBracket;
    }
};
template <typename T>
class GenericClosingBracketToken : public OperatorToken<T>
{
    public:
    GenericClosingBracketToken():
        OperatorToken<T>(std::make_shared<T>())
    {}
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(OperatorToken<T>::GetOp());
        return LastCharType::LikeClosingBracket;
    }
};

class KeywordToken : public IToken
{
    Parsable m_Keyword;
public:
    template<typename T>
    KeywordToken(const std::string& Rep, const T& Keyword):
        IToken(Rep),
        m_Keyword(Rep, Keyword)
    {
    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(m_Keyword);
        return LastCharType::Identifier;// TODO (Marius#7#): Add Keyword to LastCharType
    }
};
#endif // ITOKEN_HPP_INCLUDED
