#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED
#include <boost/lexical_cast.hpp>
#include "Tokens.hpp"
#include "../Exceptions.hpp"
//#include "Table.hpp"
//#include "Object.hpp"
//#include "Is.hpp"
#include "Get.hpp"
//#include "EvaluationContext.hpp"
//#include "IOperator.hpp"
//#include "IFunction.hpp"

namespace utf8{
    class ustring;
}

namespace Internal
{
	class IEvaluable;
    class IFunction;
    class IOperator;
    class Reference;
    class NullReference;
    struct EvaluationContext;
	class Reference;
	class CountedReference;
    namespace Types{
        class Object;
    }
namespace Utilities
{

struct GetFunc: public boost::static_visitor<std::shared_ptr<IFunction>>
{
    EvaluationContext& m_EC;
    GetFunc(EvaluationContext& EC);
    std::shared_ptr<IFunction> operator()(long long) const;
    std::shared_ptr<IFunction> operator()(double)const;
    std::shared_ptr<IFunction> operator()(const std::shared_ptr<IOperator>& op)const;
    std::shared_ptr<IFunction> operator()(const utf8::ustring& str)const;
    std::shared_ptr<IFunction> operator()(const Reference& R)const;
    std::shared_ptr<IFunction> operator()(const NullReference&)const;
    std::shared_ptr<IFunction> operator()(const std::shared_ptr<IFunction>& op)const;
};
struct IsFunc : public boost::static_visitor<bool>
{
	template <typename T>
	bool operator()(const T&) const
	{
		return false;
	}
	bool operator()(const Reference& R)const;
	bool operator()(const std::shared_ptr<IFunction>& Func) const;
};
struct ArgCounter : public boost::static_visitor<long long>
{
    EvaluationContext& m_EC;
    ArgCounter(EvaluationContext& EC);

    template <typename T>
    long long operator()(const T&) const
    {
        return 1;
    }
    long long operator()(const CountedReference& Ref) const;
    long long operator()(const std::shared_ptr<IFunction>& Func) const;
};
struct PrintTypeVisitor : public boost::static_visitor<std::string>
{
    PrintTypeVisitor(EvaluationContext& EC);

    std::string operator()(long long val)const;
    std::string operator()(double val)const
    {
        return "double";
    }
    std::string operator()(const utf8::ustring& val)const;
    std::string operator()(const std::shared_ptr<IOperator>& op)const
    {
        return "operator";
    }
    std::string operator()(const std::shared_ptr<IFunction>& op)const
    {
        return "function";
    }
    std::string operator()(const std::string& s)const;
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
std::string PrintType(EvaluationContext& EC, Types::Object& Val);

struct PrintValueVisitor : public boost::static_visitor<std::string>
{

    PrintValueVisitor(EvaluationContext& EC);
    std::string operator()(long long val)const;
    std::string operator()(double val)const;
    std::string operator()(const utf8::ustring& val)const;
    std::string operator()(const std::shared_ptr<IEvaluable>& op)const;
    std::string operator()(const std::string& s)const;
    std::string operator()(const Reference& ref)const;
    std::string operator()(const NullReference&)const;
    EvaluationContext& m_EC;
};

std::string PrintValue(EvaluationContext& EC, const Types::Object& Val);
std::string PrintValueNoThrow(EvaluationContext& EC, const Types::Object& Val);
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
    std::string operator()(const utf8::ustring& val)const;
    std::string operator()(const Parsable& P) const;
    std::string operator()(const std::shared_ptr<IEvaluable>& op)const;
    std::string operator()(const std::string& s)const
    {
        return s;
    }
    std::string operator()(const Reference& ref)const;
    std::string operator()(const NullReference&)const;
};

struct GetNumberTokenVisitor : public boost::static_visitor< NumberToken >
{

    GetNumberTokenVisitor(EvaluationContext& EC);

    NumberToken operator()(long long val)const;
    NumberToken operator()(double val)const
    {
        return val;
    }
    NumberToken operator()(const utf8::ustring& val)const;
    NumberToken operator()(const std::shared_ptr<IOperator>& op)const;
    NumberToken operator()(const std::shared_ptr<IFunction>& op)const;
    NumberToken operator()(const std::string& s)const;
    NumberToken operator()(const Reference& R)const;
    NumberToken operator()(const NullReference&)const;
    EvaluationContext& m_EC;
};

NumberToken GetNumberToken(EvaluationContext& EC, Types::Object& Val);

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
    NumberToken operator()(const utf8::ustring& val)const;
    NumberToken operator()(const Reference& R)const;
    NumberToken operator()(const NullReference&)const;
    template<typename U>
    NumberToken operator()(U) const
    {
        throw Exceptions::TypeException(std::string("Expected number; Is ") + Type<U>::Name());
    }
};
NumberToken GetNumberToken(const ResolvedToken& Val);

struct ResolveVisitor : public boost::static_visitor<ResolvedToken>
{

    EvaluationContext& m_EC;
	ResolveVisitor(EvaluationContext& EC);

	ResolvedToken operator()(long long val)const;
    ResolvedToken operator()(double val)const
    {
        return val;
    }
	ResolvedToken operator()(const utf8::ustring& val)const;
	ResolvedToken operator()(const std::string& Identifer) const;
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
ResolvedToken Resolve(EvaluationContext& EC, const Types::Object& Val);

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
