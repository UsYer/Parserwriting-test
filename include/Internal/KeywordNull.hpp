#ifndef KEYWORDNULL_HPP_INCLUDED
#define KEYWORDNULL_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
    void Null(ParserContext& PC)
    {
        PC.ThrowIfUnexpected(TokenType::KeywordWithValue, "Unexpected \"null\"");
        PC.UnexpectedToken() = TokenType::None;
        PC.OutputQueue().push_back(std::make_shared<ValueHolder>(NullReference()));
        PC.LastToken() = TokenType::KeywordWithValue;
    }
}//ns Keyword
}//ns Internal


#endif // KEYWORDNULL_HPP_INCLUDED
