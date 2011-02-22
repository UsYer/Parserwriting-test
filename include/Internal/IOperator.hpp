#ifndef OPAERATOR_H
#define OPAERATOR_H
#include "Types.hpp"
#include "IEvaluable.hpp"
namespace Internal
{
class IOperator : public IEvaluable
{
public:
    enum ArityType
    {
        UnaryPostfix = 0, //Takes one operand
        UnaryPrefix = 1,
        Binary = 2 //Takes two operands
    };
    enum AssociativityType
    {
        Right,
        Left,
        None
    };
    IOperator(const std::string& Name, const std::string& Representation, int Precedence, ArityType AT, AssociativityType AssocType):
        IEvaluable(Name,Representation),
        m_Arity(AT),
        m_Associativity(AssocType),
        m_Precedence(Precedence)
    {}
    virtual ~IOperator() {}
    int Precedence() const
    {
        return m_Precedence;
    }
    ArityType Arity() const
    {
        return m_Arity;
    }
    AssociativityType Associativity() const
    {
        return m_Associativity;
    }
    bool operator == (const IOperator& other) const
    {
        return m_Arity == other.m_Arity && m_Associativity == other.m_Associativity && m_Precedence == other.m_Precedence && m_Name == other.m_Name && m_Representation == other.m_Representation;
    }
    bool operator != (const IOperator& other) const
    {
        return !(*this == other);
    }
    using IEvaluable::operator==;
    using IEvaluable::operator!=;
protected:
private:
    ArityType m_Arity;
    AssociativityType m_Associativity;
    int m_Precedence;
};
}
#endif // OPAERATOR_H
