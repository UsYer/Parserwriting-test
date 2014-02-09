#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED
#include <stdexcept>
#include <boost/lexical_cast.hpp>
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
// TODO (Marius#8#): Make RuntimeException useful. e.g.: Stacktrace and further info
class RuntimeException : public std::runtime_error
{
    const std::string m_Name;
    const long long m_TypeId;
public:
    RuntimeException(const std::string& Message, const std::string& Name = "RuntimeException", long long TypeId = 1):
        std::runtime_error(Message),
        m_Name(Name),
        m_TypeId(TypeId)
    {}
    virtual ~RuntimeException() throw()
    {}
    const std::string& Name() const
    {
        return m_Name;
    }
    long long TypeId() const
    {
        return m_TypeId;
    }
};

///Can be used when a (native) function expects a certain type as an argument but the wrong type is supplied
class TypeException : public RuntimeException
{
    public:
    TypeException(const std::string& Message):
        RuntimeException(Message, "TypeException", 2)
    {}
};

class NameException : public RuntimeException
{
    public:
    NameException(const std::string& Message):
        RuntimeException(Message, "NameException", 3)
    {}
};
class NullReferenceException : public RuntimeException
{
public:
    explicit NullReferenceException(const std::string& What):
        RuntimeException(What, "NullReferenceException", 4)
    {
    }
};
class SyntaxException : public RuntimeException
{
    public:
		SyntaxException(const std::string& Message) :
        RuntimeException(Message, "SyntaxEception", 5)
    {}
};

}//ns Exceptions
/*
namespace Marshal
{
template<>
struct Type<Exceptions::RuntimeException>
{
    typedef CountedReference ToInternal;
};
template<>
struct Value<Exceptions::RuntimeException>
{
    static Exceptions::RuntimeException ConvertOut(const CountedReference& ExceptionRef, Internal::EvaluationContext& EC)
    {
        return Exceptions::RuntimeException("Dummymessage until internal strings are supported",EC.Scope(),(*ExceptionRef)["TypeId"]);
    }
};
}//ns Marshal
*/
#endif // EXCEPTIONS_HPP_INCLUDED
