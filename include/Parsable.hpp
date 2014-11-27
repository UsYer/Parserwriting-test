#ifndef PARSABLE_HPP_INCLUDED
#define PARSABLE_HPP_INCLUDED
#include <functional>
namespace Internal
{
    struct ParserContext;
}
class Parsable
{
public:
    Parsable(const std::string& Rep, const std::function<void(Internal::ParserContext&)>& ParseFunc):
        Representation(Rep),
        Parse(ParseFunc)
    {}
    ~Parsable(){}
    std::string Representation;
    std::function<void(Internal::ParserContext&)> Parse;
};

#endif // PARSABLE_HPP_INCLUDED
