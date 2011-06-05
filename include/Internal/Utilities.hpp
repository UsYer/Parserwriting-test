#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED
#include <boost/lexical_cast.hpp>
#include "../../MemoryManagment/include/Exceptions.hpp"
#include "Table.hpp"
#include "Object.hpp"
#include "Typenames.hpp"
#include "Is.hpp"
namespace Internal
{
namespace Utilities
{
struct PrintTypeVisitor : public boost::static_visitor<std::string>
{
    PrintTypeVisitor(EvaluationContext& EC):m_EC(EC)
    {}
    std::string operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(*this,Val);
        }
        return "long";
    }
    std::string operator()(double val)const
    {
        return "double";
    }
    std::string operator()(const boost::shared_ptr<IOperator>& op)const
    {
        return "operator";
    }
    std::string operator()(const boost::shared_ptr<IFunction>& op)const
    {
        return "function";
    }
    std::string operator()(const std::string& s)const
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
    std::string operator()(const Reference& ref)const
    {
        return "Reference -> table";
    }
    std::string operator()(const NullReference&)const
    {
        return "Nullreference";
    }
    EvaluationContext& m_EC;
};
inline std::string PrintType(EvaluationContext& EC, Types::Object& Val )
{
    EC.This = Val.This();
    return Val.Visit(PrintTypeVisitor(EC));
}

struct PrintValueVisitor : public boost::static_visitor<std::string>
{
    PrintValueVisitor(EvaluationContext& EC):m_EC(EC)
    {}
    std::string operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(*this,Val);
        }
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(double val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(const boost::shared_ptr<IEvaluable>& op)const
    {
        return op->Representation();
    }
    std::string operator()(const std::string& s)const
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
    std::string operator()(const Reference& ref)const
    {
        if( ref.IsNull() )
            throw NullReferenceException("Dereferencing a nullreference");
        else
            return "Reference -> table";
    }
    std::string operator()(const NullReference&)const
    {
        throw NullReferenceException("Dereferencing a null reference");
    }
    EvaluationContext& m_EC;
};

inline std::string PrintValue(EvaluationContext& EC, const Types::Object& Val )
{
    EC.This = Val.This();
    return Val.Visit(PrintValueVisitor(EC));
}
///Used in Parser
struct PrintValueNoResolve : public boost::static_visitor<std::string>
{
    std::string operator()(long long val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(double val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(const Parsable& P) const
    {
        return P.Representation;
    }
    std::string operator()(const boost::shared_ptr<IEvaluable>& op)const
    {
        return op->Representation();
    }
    std::string operator()(const std::string& s)const
    {
        return s;
    }
    std::string operator()(const Reference& ref)const
    {
        if( ref.IsNull() )
            return "Nullreference";

        return "Reference";
    }
    std::string operator()(const NullReference&)const
    {
        throw NullReferenceException("Dereferencing a null reference");
    }
};

struct GetNumberTokenVisitor : public boost::static_visitor< NumberToken >
{
    GetNumberTokenVisitor(EvaluationContext& EC):m_EC(EC)
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
    NumberToken operator()(const boost::shared_ptr<IOperator>& op)const
    {
        throw std::logic_error("Can't take operators as operands");
    }
    NumberToken operator()(const boost::shared_ptr<IFunction>& op)const
    {
        throw std::logic_error("Can't take functions as operands");
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
    NumberToken operator()(const Reference& R)const
    {
        if( R.IsNull() )
            throw NullReferenceException("Dereferencing a nullreference");
        else
            throw std::logic_error("Table is not convertible to a number");
    }
    NumberToken operator()(const NullReference&)const
    {
        throw NullReferenceException("Dereferencing a null reference");
    }
    EvaluationContext& m_EC;
};

inline NumberToken GetNumberToken( EvaluationContext& EC, Types::Object& Val )
{
    EC.This = Val.This();
    return Val.Visit(GetNumberTokenVisitor(EC));
}

struct GetNumberTokenNoResolveVisitor : public boost::static_visitor< NumberToken >
{
    GetNumberTokenNoResolveVisitor()
    {}
    NumberToken operator()(long long val)const
    {
        return val;
    }
    NumberToken operator()(double val)const
    {
        return val;
    }
    NumberToken operator()(const Reference& R)const
    {
        if( R.IsNull() )
            throw NullReferenceException("Dereferencing a nullreference");
        else
            throw std::logic_error("Table is not convertible to a number");
    }
    NumberToken operator()(const NullReference&)const
    {
        throw NullReferenceException("Dereferencing a null reference");
    }
    template<typename U>
    NumberToken operator()(U) const
    {
        throw std::logic_error(std::string("Expected number; Is ") + Type<U>::Name());
    }
};
inline NumberToken GetNumberToken(const ResolvedToken& Val )
{
    return boost::apply_visitor(GetNumberTokenNoResolveVisitor(),Val);
}
template<typename T>
struct Get : public boost::static_visitor<T>
{
    std::string m_ErrorMessage;
    Get():
        m_ErrorMessage("")
    {}
    Get(const std::string& ErrorMessage):
            m_ErrorMessage(ErrorMessage)
    {}
    template<typename U>
    T operator()(U) const
    {
        if( m_ErrorMessage.empty() )
            throw std::logic_error(std::string("Expected ") + Type<T>::Name() + "; Is " + Type<U>::Name());
        else
            throw std::logic_error(m_ErrorMessage);
    }
    T operator()(T Type) const
    {
        return Type;
    }
};
struct ResolveVisitor : public boost::static_visitor<ResolvedToken>
{
    EvaluationContext& m_EC;
    ResolveVisitor(EvaluationContext& EC):
        m_EC(EC)
    {
    }
    ResolvedToken operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(*this,Val);
        }
        return val;
    }
    ResolvedToken operator()(const std::string& Identifer) const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope;
        else
        {
            Scope = m_EC.This;
            m_EC.This = NullReference();
        }
        auto it = (*Scope).Find(Identifer);
        /*
        if( it == (*Scope).KeyEnd() )
            throw std::logic_error("Identifier " + Identifer + " unknown");
        */
        while( it == (*Scope).KeyEnd() )
        {
            auto Parent = (*Scope).Find("__PARENT__");
            if( Parent == (*Scope).KeyEnd() ) //We reached global scope
                throw std::logic_error("Identifier " + Identifer + " unknown");
            Scope = boost::apply_visitor(Get<Types::Scope>(),Parent->second);
            it = (*Scope).Find(Identifer);
        }
        return it->second;
    }
    template<typename T>
    ResolvedToken operator()(T Type) const
    {
        return Type;
    }
};
///Resolves the Object to an actual value.
//inline ResolvedToken Resolve( EvaluationContext& EC, Types::Object& Val )
//{
//    EC.This = Val.This();
//    return Val.Visit(ResolveVisitor(EC));
//}
inline ResolvedToken Resolve( EvaluationContext& EC, const Types::Object& Val )
{
    EC.This = Val.This();
    return Val.Visit(ResolveVisitor(EC));
}
template<typename T>
inline bool ResolvesTo( EvaluationContext& EC, const Types::Object& Val )
{
    return boost::apply_visitor(Is<T>(),Resolve(EC,Val));
}
template<typename T>
static T GetWithResolve( EvaluationContext& EC, const Types::Object& Val, const std::string& ErrorMessage = "" )
{
    ResolvedToken Token(Utilities::Resolve(EC,Val));
    return boost::apply_visitor(Get<T>(ErrorMessage),Token);
}
}//namespace Utilities
}//namespace Internal
#endif // UTILITIES_HPP_INCLUDED
