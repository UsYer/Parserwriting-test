#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <boost/ptr_container/ptr_set.hpp>
#include "../IToken.hpp"

namespace Internal
{
/*struct SetOrdering
{
    bool operator()(const boost::shared_ptr<IOperator>& lhs, const boost::shared_ptr<IOperator>& rhs)
    {
        return lhs->Representation() < rhs->Representation();
    }
};*/
struct SetOrdering : std::binary_function<IToken,IToken,bool>
{
    bool operator()(const IToken& lhs, const IToken& rhs) const;
};

/**Performs the lexical analysis of an expression

*/
class Tokenizer
{
public:
    friend struct TokenizeContext;
    /** Default constructor */
    Tokenizer():
        LastChar(LastCharType::Start),
        m_Context(LastChar,OutputQueue)
    {}
    /** \brief Tokenizes the given expression and returns the result
     *
     * \param expression The expression to be parsed
     * \return std::deque<UnparsedToken>
     *
     */
    std::deque<UnparsedToken> Tokenize(const std::string& expression);
    /**
     * \brief Registers the provided operator within the Tokenizer so it will be recognized in a string while tokenizing
     * \param p Operator to register
     */
    void RegisterToken(IToken* Token); //not const because ptr_set requires lvalues

    /** \brief Clears any previous states and buffers
     *
     * \return void
     *
     */
    void Clear()
    {
        LastChar = LastCharType::Start;
        m_NumberBuffer.clear();
        OperatorBuffer.clear();
        OutputQueue.clear();
    }


protected:
private:
    /**Parses an operator from a given string

    @return true if a operator was recognized, false otherwise
    */
    bool ParseOperator();
    void FlushNumBuffer(bool DecimalNumber);
    bool ParseOperator(std::string& Expr);
    std::deque<UnparsedToken> TokenizeGreedy(const std::string& expression);

    LastCharProxy LastChar;
    std::string m_NumberBuffer;
    std::string OperatorBuffer;
    boost::ptr_set< IToken,SetOrdering > Operators;
    std::deque<UnparsedToken> OutputQueue;
    TokenizeContext m_Context;
};

}//ns Internal
#endif // TOKENIZER_H
