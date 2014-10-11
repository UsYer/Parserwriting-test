#ifndef KEYWORDNULL_HPP_INCLUDED
#define KEYWORDNULL_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
    void Null(ParserContext& PC)
    {
		PC.ThrowIfUnexpected(TokenTypeOld::KeywordWithValue, "Unexpected \"null\"");
		PC.UnexpectedToken() = TokenTypeOld::None;
        PC.OutputQueue().push_back(std::make_shared<ValueHolder>(NullReference()));
		PC.LastToken() = TokenTypeOld::KeywordWithValue;
    }
}//ns Keyword
}//ns Internal


#endif // KEYWORDNULL_HPP_INCLUDED
