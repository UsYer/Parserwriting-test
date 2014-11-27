#include "../include/Internal/Utilities.hpp"
//#include "../include/Exceptions.hpp"
#include "../MemoryManagment/include/Reference.hpp"
#include "../include/Internal/Table.hpp"
#include "../include/Internal/IOperator.hpp"
#include "../include/Internal/IFunction.hpp"
//#include "../include/Internal/Get.hpp"
#include "../include/Internal/EvaluationContext.hpp"
#include "../include/Internal/Object.hpp"

using namespace Internal;

Utilities::GetFunc::GetFunc(EvaluationContext& EC) :
m_EC(EC)
{

}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(long long) const
{
	throw Exceptions::TypeException("Expected function; Is long");
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(double)const
{
	throw Exceptions::TypeException("Expected function; Is double");
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(const std::shared_ptr<IOperator>& op)const
{
	throw Exceptions::TypeException("Expected function; Is operator " + op->Representation());
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(const utf8::ustring& str)const
{
	throw Exceptions::TypeException("Expected function; Is string");
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(const Reference& R)const
{
	if (R.IsNull())
		throw Exceptions::NullReferenceException("Calling a nullreference");
	else {    //check whether the table has an @-specialmethod which makes it callable
		if ((*R).Contains("@call"))
			return boost::apply_visitor(*this, (*R)["@call"]); //recursivly check whether @call is a function
		throw Exceptions::TypeException("Expected function; Is table");
	}
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(const NullReference&)const
{
	throw Exceptions::NullReferenceException("Calling a nullreference");
}
std::shared_ptr<IFunction> Utilities::GetFunc::operator()(const std::shared_ptr<IFunction>& op)const
{
	return op;
}

bool Utilities::IsFunc::operator()(const Reference& R)const
{
	if (!R.IsNull()) { //check whether the table has an @-specialmethod which makes it callable
		if ((*R).Contains("@Call"))
			return boost::apply_visitor(*this, (*R)["@Call"]); //recursively check whether @Call is a function
	}
	return false;
}
bool Utilities::IsFunc::operator()(const std::shared_ptr<IFunction>& Func) const
{
	return *Func != "__ALSM__";
}


Utilities::ArgCounter::ArgCounter(EvaluationContext& EC) :
m_EC(EC)
{}
long long Utilities::ArgCounter::operator()(const CountedReference& Ref) const
{
	auto it = (*Ref).Find("__ARGCOUNT__");
	if (it != (*Ref).KeyEnd())
		return boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"), it->second);
	else
		return 1;
}
long long Utilities::ArgCounter::operator()(const std::shared_ptr<IFunction>& Func) const
{
	if (*Func == "__ALSM__")
		return 0;
	else
		return 1;
}

Utilities::PrintTypeVisitor::PrintTypeVisitor(EvaluationContext& EC) :m_EC(EC)
{}

std::string Utilities::PrintTypeVisitor::operator()(long long val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(PrintTypeVisitor(m_EC), Val);
	}
	return "long";
}

std::string Utilities::PrintTypeVisitor::operator()(const utf8::ustring& val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(PrintTypeVisitor(m_EC), Val);
	}
	return "string";
}
std::string Utilities::PrintTypeVisitor::operator()(const std::string& s)const
{
	Types::Scope Scope((NullReference()));
	if (m_EC.This.IsNull())
		Scope = m_EC.Scope();
	else {
		Scope = m_EC.This;
		m_EC.This = NullReference();
	}
	auto it = (*Scope).Find(s);
	if (it == (*Scope).KeyEnd())
		throw Exceptions::NameException("Identifier " + s + " unknown");

	return boost::apply_visitor(*this, it->second);
}

std::string Utilities::PrintType(EvaluationContext& EC, Types::Object& Val)
{
	EC.This = Val.This();
	return Val.Visit(PrintTypeVisitor(EC));
}

Utilities::PrintValueVisitor::PrintValueVisitor(EvaluationContext& EC) :m_EC(EC)
{}
std::string Utilities::PrintValueVisitor::operator()(long long val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		PrintValueVisitor visitor(m_EC);
		return boost::apply_visitor(visitor, Val);
	}
	return boost::lexical_cast<std::string>(val);
}
std::string Utilities::PrintValueVisitor::operator()(double val)const
{
	return boost::lexical_cast<std::string>(val);
}
std::string Utilities::PrintValueVisitor::operator()(const utf8::ustring& val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(PrintValueVisitor(m_EC), Val);
	}
	return "\"" + val.raw() + "\"";
}
std::string Utilities::PrintValueVisitor::operator()(const std::shared_ptr<IEvaluable>& op)const
{
	return op->Representation();
}
std::string Utilities::PrintValueVisitor::operator()(const std::string& s)const
{
	Types::Scope Scope((NullReference()));
	if (m_EC.This.IsNull())
		Scope = m_EC.Scope();
	else {
		Scope = m_EC.This;
		m_EC.This = NullReference();
	}
	auto it = (*Scope).Find(s);
	if (it == (*Scope).KeyEnd())
		throw Exceptions::NameException("Identifier " + s + " unknown");

	return boost::apply_visitor(*this, it->second);
}
std::string Utilities::PrintValueVisitor::operator()(const Reference& ref)const
{
	if (ref.IsNull())
		throw Exceptions::NullReferenceException("Dereferencing a nullreference");
	else {    //check whether the table has an @-specialmethod which makes it printable
		if ((*ref).Contains("@str")) {
			Internal::Types::Object obj((*ref)["@str"]);
			obj.This(ref);
			m_EC.Call(obj, ResolvedToken(NullReference()), 0);
			Types::Object val = m_EC.Stack.Pop();
			utf8::ustring str = val.Visit(Utilities::Get<utf8::ustring>("@str did not return a string type"));
			return str;
		} else {
			return "Reference -> table";

		}
	}
}
std::string Utilities::PrintValueVisitor::operator()(const NullReference&)const
{
	throw Exceptions::NullReferenceException("Dereferencing a null reference");
}

std::string Utilities::PrintValue(EvaluationContext& EC, const Types::Object& Val)
{
	EC.This = Val.This();
	return Val.Visit(PrintValueVisitor(EC));
}

std::string Utilities::PrintValueNoThrow(EvaluationContext& EC, const Types::Object& Val)
{
	EC.This = Val.This();
	try {
		return Val.Visit(PrintValueVisitor(EC));
	}
	catch (std::exception&) {
		return "";
	}
}

std::string Utilities::PrintValueNoResolve::operator()(const utf8::ustring& val)const
{
	return "\"" + val.raw() + "\"";
}
std::string Utilities::PrintValueNoResolve::operator()(const Parsable& P) const
{
	return P.Representation;
}
std::string Utilities::PrintValueNoResolve::operator()(const std::shared_ptr<IEvaluable>& op)const
{
	return op->Representation();
}
std::string Utilities::PrintValueNoResolve::operator()(const Reference& ref)const
{
	if (ref.IsNull())
		return "Nullreference";

	return "Reference";
}
std::string Utilities::PrintValueNoResolve::operator()(const NullReference&)const
{
	throw Exceptions::NullReferenceException("Dereferencing a null reference");
}

Utilities::GetNumberTokenVisitor::GetNumberTokenVisitor(EvaluationContext& EC) :m_EC(EC)
{}

NumberToken Utilities::GetNumberTokenVisitor::operator()(long long val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(GetNumberTokenVisitor(m_EC), Val);
	}
	return val;
}

NumberToken Utilities::GetNumberTokenVisitor::operator()(const utf8::ustring& val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(GetNumberTokenVisitor(m_EC), Val);
	}
	throw Exceptions::TypeException("String is not convertible to a number");
}
NumberToken Utilities::GetNumberTokenVisitor::operator()(const std::shared_ptr<IOperator>& op)const
{
	throw Exceptions::TypeException("Can't take operators as operands");
}
NumberToken Utilities::GetNumberTokenVisitor::operator()(const std::shared_ptr<IFunction>& op)const
{
	throw Exceptions::TypeException("Can't take functions as operands");
}
NumberToken Utilities::GetNumberTokenVisitor::operator()(const std::string& s)const
{
	Types::Scope Scope((NullReference()));
	if (m_EC.This.IsNull())
		Scope = m_EC.Scope();
	else {
		Scope = m_EC.This;
		m_EC.This = NullReference();
	}
	auto it = (*Scope).Find(s);
	if (it == (*Scope).KeyEnd())
		throw Exceptions::NameException("Identifier " + s + " unknown");

	return boost::apply_visitor(*this, it->second);
}
NumberToken Utilities::GetNumberTokenVisitor::operator()(const Reference& R)const
{
	if (R.IsNull())
		throw Exceptions::NullReferenceException("Dereferencing a nullreference");
	else
		throw Exceptions::TypeException("Table is not convertible to a number");
}
NumberToken Utilities::GetNumberTokenVisitor::operator()(const NullReference&)const
{
	throw Exceptions::NullReferenceException("Dereferencing a null reference");
}

NumberToken Utilities::GetNumberToken(EvaluationContext& EC, Types::Object& Val)
{
	EC.This = Val.This();
	return Val.Visit(GetNumberTokenVisitor(EC));
}

NumberToken Utilities::GetNumberTokenNoResolveVisitor::operator()(const utf8::ustring& val)const
{
	throw Exceptions::TypeException("String is not convertible to a number");
}
NumberToken Utilities::GetNumberTokenNoResolveVisitor::operator()(const Reference& R)const
{
	if (R.IsNull())
		throw Exceptions::NullReferenceException("Dereferencing a nullreference");
	else
		throw Exceptions::TypeException("Table is not convertible to a number");
}
NumberToken Utilities::GetNumberTokenNoResolveVisitor::operator()(const NullReference&)const
{
	throw Exceptions::NullReferenceException("Dereferencing a null reference");
}

NumberToken Utilities::GetNumberToken(const ResolvedToken& Val)
{
	return boost::apply_visitor(GetNumberTokenNoResolveVisitor(), Val);
}

Utilities::ResolveVisitor::ResolveVisitor(EvaluationContext& EC) :
m_EC(EC)
{
}

ResolvedToken Utilities::ResolveVisitor::operator()(long long val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table[0]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(ResolveVisitor(m_EC), Val);
	}
	return val;
}

ResolvedToken Utilities::ResolveVisitor::operator()(const utf8::ustring& val)const
{
	if (!m_EC.This.IsNull()) { //accessing table element. E.g.: table["foo"]
		Member Val((*m_EC.This)[val]);
		m_EC.This = NullReference();
		return boost::apply_visitor(ResolveVisitor(m_EC), Val);
	}
	return val;
}
ResolvedToken Utilities::ResolveVisitor::operator()(const std::string& Identifer) const
{
	Types::Scope Scope((NullReference()));
	if (m_EC.This.IsNull())
		Scope = m_EC.Scope();
	else {
		Scope = m_EC.This;
		m_EC.This = NullReference();
	}
	auto it = (*Scope).Find(Identifer);
	/*
	if( it == (*Scope).KeyEnd() )
	throw std::logic_error("Identifier " + Identifer + " unknown");
	*/
	while (it == (*Scope).KeyEnd()) {
		auto Parent = (*Scope).Find("__PARENT__");
		if (Parent == (*Scope).KeyEnd()) //We reached global scope
			throw Exceptions::NameException("Identifier " + Identifer + " unknown");
		Scope = boost::apply_visitor(Get<Types::Scope>(), Parent->second);
		it = (*Scope).Find(Identifer);
	}
	return it->second;
}

ResolvedToken Utilities::Resolve(EvaluationContext& EC, const Types::Object& Val)
{
	EC.This = Val.This();
	return Val.Visit(ResolveVisitor(EC));
}