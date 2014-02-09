#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED
#include <boost/lexical_cast.hpp>
#include "../Exceptions.hpp"
#include "Table.hpp"
#include "Object.hpp"
#include "Is.hpp"
#include "Get.hpp"
#include "EvaluationContext.hpp"
#include "IOperator.hpp"
#include "IFunction.hpp"
namespace Internal
{
namespace Utilities
{
struct GetFunc: public boost::static_visitor<std::shared_ptr<IFunction>>
{
    EvaluationContext& m_EC;
    GetFunc(EvaluationContext& EC):
        m_EC(EC)
    {

    }
    std::shared_ptr<IFunction> operator()(long long ) const
    {
        throw Exceptions::TypeException("Expected function; Is long");
    }
    std::shared_ptr<IFunction> operator()(double )const
    {
        throw Exceptions::TypeException("Expected function; Is double");
    }
    std::shared_ptr<IFunction> operator()(const std::shared_ptr<IOperator>& op)const
    {
        throw Exceptions::TypeException("Expected function; Is operator " + op->Representation());
    }
    std::shared_ptr<IFunction> operator()(const utf8::ustring& str)const
    {
        throw Exceptions::TypeException("Expected function; Is string");
    }
    std::shared_ptr<IFunction> operator()(const Reference& R)const
    {
        if( R.IsNull() )
            throw Exceptions::NullReferenceException("Calling a nullreference");
        else
        {    //check whether the table has an @-specialmethod which makes it callable
            if( (*R).Contains("@call") )
                return boost::apply_visitor(*this, (*R)["@call"]); //recursivly check whether @call is a function
            throw Exceptions::TypeException("Expected function; Is table");
        }
    }
    std::shared_ptr<IFunction> operator()(NullReference)const
    {
        throw Exceptions::NullReferenceException("Calling a nullreference");
    }
    std::shared_ptr<IFunction> operator()(const std::shared_ptr<IFunction>& op)const
    {
        return op;
    }
};
struct ArgCounter : public boost::static_visitor<long long>
{
    EvaluationContext& m_EC;
    ArgCounter(EvaluationContext& EC):
        m_EC(EC)
    {}
    template <typename T>
	long long operator()(const T&) const
    {
        return 1;
    }
	long long operator()(const CountedReference& Ref) const
    {
        auto it = (*Ref).Find("__ARGCOUNT__");
        if( it != (*Ref).KeyEnd() )
            return boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"),it->second);
        else
            return 1;
    }
	long long operator()(const std::shared_ptr<IFunction>& Func) const
    {
        if( *Func == "__ALSM__" )
            return 0;
        else
            return 1;
    }
};
struct PrintTypeVisitor : public boost::static_visitor<std::string>
{
    /*struct LiteralVisitor : public boost::static_visitor<std::string>
    {
        EvaluationContext& m_EC;
        LiteralVisitor(EvaluationContext& EC):m_EC(EC)
        {}
        std::string operator()(long long val)const
        {
            if( !m_EC.This.IsNull() )
            { //accessing table element. E.g.: table[0]
                Member Val((*m_EC.This)[val]);
                m_EC.This = NullReference();
                return boost::apply_visitor(PrintTypeVisitor(m_EC),Val);
            }
            return "long";
        }
        std::string operator()(double val)const
        {
            return "double";
        }
        std::string operator()(utf8::ustring val)const
        {
            return "string";
        }
    };*/
    PrintTypeVisitor(EvaluationContext& EC):m_EC(EC)
    {}

//    std::string operator()(LiteralToken lt)const
//    {
//        LiteralVisitor lv(m_EC);
//        return boost::apply_visitor(lv, lt);
//    }
    std::string operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(PrintTypeVisitor(m_EC),Val);
        }
        return "long";
    }
    std::string operator()(double val)const
    {
        return "double";
    }
    std::string operator()(utf8::ustring val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(PrintTypeVisitor(m_EC),Val);
        }
        return "string";
    }
    std::string operator()(const std::shared_ptr<IOperator>& op)const
    {
        return "operator";
    }
    std::string operator()(const std::shared_ptr<IFunction>& op)const
    {
        return "function";
    }
    std::string operator()(const std::string& s)const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope();
        else
        {
            Scope = m_EC.This;
            m_EC.This = NullReference();
        }
        auto it = (*Scope).Find(s);
        if( it == (*Scope).KeyEnd() )
            throw Exceptions::NameException("Identifier " + s + " unknown");

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
//    struct LiteralVisitor : public boost::static_visitor<std::string>
//    {
//        EvaluationContext& m_EC;
//        LiteralVisitor(EvaluationContext& EC):m_EC(EC)
//        {}
//        std::string operator()(long long val)const
//        {
//            if( !m_EC.This.IsNull() )
//            { //accessing table element. E.g.: table[0]
//                Member Val((*m_EC.This)[val]);
//                m_EC.This = NullReference();
//                PrintValueVisitor visitor(m_EC);
//                return boost::apply_visitor(visitor,Val);
//            }
//            return boost::lexical_cast<std::string>(val);
//        }
//        std::string operator()(double val)const
//        {
//            return boost::lexical_cast<std::string>(val);
//        }
//        std::string operator()(utf8::ustring val)const
//        {
//            return val.raw();
//        }
//    };

    PrintValueVisitor(EvaluationContext& EC):m_EC(EC)
    {}

//    std::string operator()(const LiteralToken& lt)const
//    {
//        LiteralVisitor lv(m_EC);
//        return boost::apply_visitor(lv, lt);
//    }
    std::string operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            PrintValueVisitor visitor(m_EC);
            return boost::apply_visitor(visitor,Val);
        }
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(double val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(const utf8::ustring& val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(PrintValueVisitor(m_EC),Val);
        }
        return "\"" + val.raw() + "\"";
    }
    std::string operator()(const std::shared_ptr<IEvaluable>& op)const
    {
        return op->Representation();
    }
    std::string operator()(const std::string& s)const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope();
        else
        {
            Scope = m_EC.This;
            m_EC.This = NullReference();
        }
        auto it = (*Scope).Find(s);
        if( it == (*Scope).KeyEnd() )
            throw Exceptions::NameException("Identifier " + s + " unknown");

        return boost::apply_visitor(*this,it->second);
    }
    std::string operator()(const Reference& ref)const
    {
        if( ref.IsNull() )
            throw Exceptions::NullReferenceException("Dereferencing a nullreference");
        else
        {    //check whether the table has an @-specialmethod which makes it printable
            if( (*ref).Contains("@str"))
            {
                Internal::Types::Object obj((*ref)["@str"]);
                obj.This(ref);
                m_EC.Call(obj,ResolvedToken(NullReference()),0);
                Types::Object val = m_EC.Stack.Pop();
                utf8::ustring str = val.Visit(Utilities::Get<utf8::ustring>("@str did not return a string type"));
                return str;
            }
            else
            {
                return "Reference -> table";

            }
        }
    }
    std::string operator()(const NullReference&)const
    {
        throw Exceptions::NullReferenceException("Dereferencing a null reference");
    }
    EvaluationContext& m_EC;
};

inline std::string PrintValue(EvaluationContext& EC, const Types::Object& Val )
{
    EC.This = Val.This();
    return Val.Visit(PrintValueVisitor(EC));
}
inline std::string PrintValueNoThrow(EvaluationContext& EC, const Types::Object& Val )
{
    EC.This = Val.This();
    try
    {
        return Val.Visit(PrintValueVisitor(EC));
    }
    catch(std::exception&)
    {
        return "";
    }
}
///Used in Parser
struct PrintValueNoResolve : public boost::static_visitor<std::string>
{
//    struct LiteralVisitor : public boost::static_visitor<std::string>
//    {
//        std::string operator()(long long val)const
//        {
//            return boost::lexical_cast<std::string>(val);
//        }
//        std::string operator()(double val)const
//        {
//            return boost::lexical_cast<std::string>(val);
//        }
//        std::string operator()(const utf8::ustring& val)const
//        {
//            return val.raw();
//        }
//    };

//    std::string operator()(LiteralToken lt)const
//    {
//        LiteralVisitor lv;
//        return boost::apply_visitor(lv, lt);
//    }
    std::string operator()(long long val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(double val)const
    {
        return boost::lexical_cast<std::string>(val);
    }
    std::string operator()(const utf8::ustring& val)const
    {
        return "\"" + val.raw() + "\"";
    }
    std::string operator()(const Parsable& P) const
    {
        return P.Representation;
    }
    std::string operator()(const std::shared_ptr<IEvaluable>& op)const
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
        throw Exceptions::NullReferenceException("Dereferencing a null reference");
    }
};

struct GetNumberTokenVisitor : public boost::static_visitor< NumberToken >
{
//    struct LiteralVisitor : public boost::static_visitor<NumberToken>
//    {
//        EvaluationContext& m_EC;
//        LiteralVisitor(EvaluationContext& EC):m_EC(EC)
//        {}
//        NumberToken operator()(long long val)const
//        {
//            if( !m_EC.This.IsNull() )
//            { //accessing table element. E.g.: table[0]
//                Member Val((*m_EC.This)[val]);
//                m_EC.This = NullReference();
//                return boost::apply_visitor(GetNumberTokenVisitor(m_EC),Val);
//            }
//            return val;
//        }
//        NumberToken operator()(double val)const
//        {
//            return val;
//        }
//        NumberToken operator()(const utf8::ustring& val)const
//        {
//            throw Exceptions::TypeException("String is not convertible to a number");
//        }
//    };
    GetNumberTokenVisitor(EvaluationContext& EC):m_EC(EC)
    {}

//    NumberToken operator()(LiteralToken lt)const
//    {
//        LiteralVisitor lv(m_EC);
//        return boost::apply_visitor(lv, lt);
//    }
    NumberToken operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(GetNumberTokenVisitor(m_EC),Val);
        }
        return val;
    }
    NumberToken operator()(double val)const
    {
        return val;
    }
    NumberToken operator()(const utf8::ustring& val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(GetNumberTokenVisitor(m_EC),Val);
        }
        throw Exceptions::TypeException("String is not convertible to a number");
    }
    NumberToken operator()(const std::shared_ptr<IOperator>& op)const
    {
        throw Exceptions::TypeException("Can't take operators as operands");
    }
    NumberToken operator()(const std::shared_ptr<IFunction>& op)const
    {
        throw Exceptions::TypeException("Can't take functions as operands");
    }
    NumberToken operator()(const std::string& s)const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope();
        else
        {
            Scope = m_EC.This;
            m_EC.This = NullReference();
        }
        auto it = (*Scope).Find(s);
        if( it == (*Scope).KeyEnd() )
            throw Exceptions::NameException("Identifier " + s + " unknown");

        return boost::apply_visitor(*this,it->second);
    }
    NumberToken operator()(const Reference& R)const
    {
        if( R.IsNull() )
            throw Exceptions::NullReferenceException("Dereferencing a nullreference");
        else
            throw Exceptions::TypeException("Table is not convertible to a number");
    }
    NumberToken operator()(const NullReference&)const
    {
        throw Exceptions::NullReferenceException("Dereferencing a null reference");
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
//    struct LiteralVisitor : public boost::static_visitor<NumberToken>
//    {
//        NumberToken operator()(long long val)const
//        {
//            return val;
//        }
//        NumberToken operator()(double val)const
//        {
//            return val;
//        }
//        NumberToken operator()(const utf8::ustring& val)const
//        {
//            throw Exceptions::TypeException("String is not convertible to a number");
//        }
//    };


    GetNumberTokenNoResolveVisitor()
    {}
//    NumberToken operator()(LiteralToken lt)const
//    {
//        LiteralVisitor lv;
//        return boost::apply_visitor(lv, lt);
//    }
    NumberToken operator()(long long val)const
    {
        return val;
    }
    NumberToken operator()(double val)const
    {
        return val;
    }
    NumberToken operator()(const utf8::ustring& val)const
    {
        throw Exceptions::TypeException("String is not convertible to a number");
    }
    NumberToken operator()(const Reference& R)const
    {
        if( R.IsNull() )
            throw Exceptions::NullReferenceException("Dereferencing a nullreference");
        else
            throw Exceptions::TypeException("Table is not convertible to a number");
    }
    NumberToken operator()(const NullReference&)const
    {
        throw Exceptions::NullReferenceException("Dereferencing a null reference");
    }
    template<typename U>
    NumberToken operator()(U) const
    {
        throw Exceptions::TypeException(std::string("Expected number; Is ") + Type<U>::Name());
    }
};
inline NumberToken GetNumberToken(const ResolvedToken& Val )
{
    return boost::apply_visitor(GetNumberTokenNoResolveVisitor(),Val);
}

struct ResolveVisitor : public boost::static_visitor<ResolvedToken>
{
//    struct LiteralVisitor : public boost::static_visitor<ResolvedToken>
//    {
//        EvaluationContext& m_EC;
//        LiteralVisitor(EvaluationContext& EC):m_EC(EC)
//        {}
//
//        ResolvedToken operator()(long long val)const
//        {
//            if( !m_EC.This.IsNull() )
//            { //accessing table element. E.g.: table[0]
//                Member Val((*m_EC.This)[val]);
//                m_EC.This = NullReference();
//                return boost::apply_visitor(ResolveVisitor(m_EC),Val);
//            }
//            return val;
//        }
//        ResolvedToken operator()(double val)const
//        {
//            return val;
//        }
//        ResolvedToken operator()(const utf8::ustring& val)const
//        {
//            return val;
//        }
//    };
    EvaluationContext& m_EC;
    ResolveVisitor(EvaluationContext& EC):
        m_EC(EC)
    {
    }
//    ResolvedToken operator()(LiteralToken lt)const
//    {
//        LiteralVisitor lv(m_EC);
//        return boost::apply_visitor(lv, lt);
//    }
    ResolvedToken operator()(long long val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table[0]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(ResolveVisitor(m_EC),Val);
        }
        return val;
    }
    ResolvedToken operator()(double val)const
    {
        return val;
    }
    ResolvedToken operator()(const utf8::ustring& val)const
    {
        if( !m_EC.This.IsNull() )
        { //accessing table element. E.g.: table["foo"]
            Member Val((*m_EC.This)[val]);
            m_EC.This = NullReference();
            return boost::apply_visitor(ResolveVisitor(m_EC),Val);
        }
        return val;
    }
    ResolvedToken operator()(const std::string& Identifer) const
    {
        Types::Scope Scope((NullReference()));
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope();
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
                throw Exceptions::NameException("Identifier " + Identifer + " unknown");
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
template<typename T>
inline bool ResolvesToAndGet( EvaluationContext& EC, const Types::Object& Val, T& OutValue, const std::string& ErrorMessage = ""  )
{
    ResolvedToken token = Resolve(EC,Val);
    bool is_type = boost::apply_visitor(Is<T>(), token);
    if (is_type)
    {
        OutValue = GetWithResolve<T>(EC, Val, ErrorMessage);
    }
    return is_type;
}
}//namespace Utilities
}//namespace Internal
#endif // UTILITIES_HPP_INCLUDED
