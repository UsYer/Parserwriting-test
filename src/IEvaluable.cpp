/*
#include <boost/shared_ptr.hpp>
#include "../MemoryManagment/include/Exceptions.hpp"
#include "../include/Internal/IEvaluable.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/Table.hpp"
#include "../include/Internal/Object.hpp"
using namespace Internal;

struct GetNumberTokenVisitor : public boost::static_visitor< NumberToken >
{
    GetNumberTokenVisitor(EvaluationContext& EC):
        m_EC(EC)
    {}
    NumberToken operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(*this,Val);
        }
        return val;
    }
    NumberToken operator()(double val)const
    {
        return val;
    }
    NumberToken operator()(const boost::shared_ptr<IEvaluable>& op)const
    {
        throw std::invalid_argument("Can't take functions as operands");
    }
    NumberToken operator()(const std::string& s)const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope;
        else
        {
            Scope = m_EC.This;
            m_EC.This = NullReference();
        }
        auto it = (*Scope).Find(s);
        if( it == (*Scope).KeyEnd() )
            throw std::logic_error("Identifier " + s + " unknown");

        return boost::apply_visitor(*this,it->second);
    }
    NumberToken operator()(const Reference& ref)const
    {
        if( ref.IsNull() )
                throw std::logic_error("Dereferencing a nullreference");
        else
            throw std::logic_error("Table is not convertible to a number");
    }
    NumberToken operator()(const NullReference&)const
    {
        throw NullReferenceException("Dereferencing a null reference");
    }
    EvaluationContext& m_EC;
};

NumberToken IEvaluable::GetNumberToken( EvaluationContext& EC, Types::Object& Val ) const
{
    EC.This = Val.This();
    return Val.Visit(GetNumberTokenVisitor(EC));
}
*/
