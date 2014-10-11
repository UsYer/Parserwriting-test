#ifndef PARSEEOL_HPP_INCLUDED
#define PARSEEOL_HPP_INCLUDED
namespace Internal
{
inline void ParseEOL(ParserContext& PC)
{
	PC.ThrowIfUnexpected(TokenTypeOld::EOL);
    //if the last operator is a prefix or a binary signal an error, because it always needs more input to evaluate
	if (PC.LastToken() == TokenTypeOld::OpUnaryPrefix || PC.LastToken() == TokenTypeOld::OpBinary)
        throw std::logic_error("Missing input after operator");
    //Check if there are closing brackets missing
    if( !PC.ExpectedBracket().empty() )
        throw std::logic_error("Expected '" + PC.ExpectedBracket().top()->Representation() + "' before end of input");
    //No more tokens to be read; pop all operators left from the OpStack to the OutputQueue
    while( !PC.OperatorStack().empty() )
    {
        PC.OutputQueue().push_back(PC.OperatorStack().top());
        PC.OperatorStack().pop();
    }
	PC.LastToken() = TokenTypeOld::EOL;
}
class EOLToken : public IToken
{
public:
    EOLToken():
        IToken("\n")
    {

    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(Token{ TokenType::EOL, Parsable{ "\n", &ParseEOL } });
        return LastCharType::EOL;
    }
};
}
#endif // PARSEEOL_HPP_INCLUDED
