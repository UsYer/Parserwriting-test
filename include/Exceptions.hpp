#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED
#include <exception>
namespace Exceptions
{
class ParseError : public std::exception
{
public:
    struct Location
    {
        Location(const std::string& F, unsigned L):
            File(F),
            Line(L)
        {}
        std::string File;
        unsigned Line;
    };
    ParseError(const std::string& Message, const Location& Loc):
        std::exception(),
        m_Message(Loc.File + "[" + boost::lexical_cast<std::string>(Loc.Line) + "]: " + Message),
        m_Location(Loc)
    {

    }
    virtual ~ParseError() throw()
    {}
    const char* what() const throw()
    {
        return m_Message.c_str();
    }
    unsigned Line() const
    {
        return m_Location.Line;
    }
    const std::string& File() const
    {
        return m_Location.File;
    }
private:
    std::string m_Message;
    Location m_Location;
};
}
#endif // EXCEPTIONS_HPP_INCLUDED
