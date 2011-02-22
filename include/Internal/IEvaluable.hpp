#ifndef IEVALUABLE_H
#define IEVALUABLE_H
//#include <stack>
#include <string>
#include "Tokens.hpp"
namespace Internal
{
class MemoryController;
class Reference;
class EvaluationContext;
class IEvaluable
{
public:
    /** Default constructor */
    IEvaluable(const std::string& Name, const std::string& Representation):
        m_Name(Name), m_Representation(Representation)
    {}
    /** Default destructor */
    virtual ~IEvaluable() {}

    std::string Name() const
    {
        return m_Name;
    }
    std::string Representation() const
    {
        return m_Representation;
    }
    virtual void Eval(EvaluationContext&) = 0;

    bool operator == (const std::string& s) const
    {
        return m_Representation == s;
    }
    bool operator != (const std::string& s) const
    {
        return m_Representation != s;
    }
    friend bool operator== (const std::string& lhs, const IEvaluable& rhs);
    friend bool operator== (const char* lhs, const IEvaluable& rhs);
    friend bool operator!= (const char* lhs, const IEvaluable& rhs);
protected:
    IEvaluable()
    {}
    std::string m_Name;
    std::string m_Representation;
private:
};
//inline is used here, because otherwise we would get some linker errors
inline bool operator== (const std::string& lhs, const IEvaluable& rhs)
{
    return lhs == rhs.m_Representation;
}
inline bool operator== (const char* lhs, const IEvaluable& rhs)
{
    return lhs == rhs.m_Representation;
}
inline bool operator!= (const char* lhs, const IEvaluable& rhs)
{
    return !(lhs == rhs);
}
}
#endif // IEVALUABLE_H
