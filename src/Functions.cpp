#include "../include/Internal/Functions.hpp"
#include "../include/Platform.hpp"
#include "../include/Internal/Utilities.hpp"
#include "../include/Internal/EvaluationContext.hpp"

#include "boost/variant.hpp"
using namespace Internal;

namespace{
	struct TypeVisitor : public boost::static_visitor<std::string>
	{
		std::string operator()(long long val)const
		{
			return "long";
		}
		std::string operator()(double val)const
		{
			return "double";
		}
		std::string operator()(utf8::ustring)const
		{
			return "string";
		}
		std::string operator()(const std::shared_ptr<IEvaluable>& op)const
		{
			return "function";
		}
		std::string operator()(const Reference& ref)const
		{
			return "reference -> table";
		}
		std::string operator()(const NullReference&)const
		{
			return "NullReference";
		}
	};

	struct Max : public boost::static_visitor<NumberToken>
	{
		template<typename T, typename U>
		auto operator()(T lhs, U rhs) -> decltype(lhs > rhs ? lhs : rhs)
		{
			return lhs > rhs ? lhs : rhs;
		}
	};

	struct Sqrt : boost::static_visitor<NumberToken>
	{
		template<typename T>
		NumberToken operator()(T Arg) const
		{
			return std::sqrt(Arg);
		}
	};
} // anon ns

void TypeFunc::Eval(EvaluationContext& EC)
{
	::Platform::Out() << "\n" << boost::apply_visitor(TypeVisitor{}, m_LocalScope[0]) << "\n";
}

void MaxFunc::Eval(EvaluationContext& EC)
{
	if (m_SuppliedArguments == 0) {
		EC.Stack.Push(NullReference{});
		return;
	}
	NumberToken One, Two;
	Max Maximum;
	if (m_SuppliedArguments == 1)
		One = Utilities::GetNumberToken(IFunction::GetArg(0));
	else {
		One = Utilities::GetNumberToken(IFunction::GetArg(0));
		for (unsigned i = 1; i < m_SuppliedArguments; i++) {
			Two = Utilities::GetNumberToken(IFunction::GetArg(i));
			One = boost::apply_visitor(Maximum, One, Two);
		}
	}
	EC.Stack.Push(One);
}

void SqrtFunc::Eval(EvaluationContext& EC)
{
	NumberToken Arg(Utilities::GetNumberToken(IFunction::GetArg(0)));
	EC.Stack.Push(boost::apply_visitor(Sqrt{}, Arg));
}

void CreateTableFunc::Eval(EvaluationContext& EC)
{
	if (m_SuppliedArguments > 1) //More than one arg means the arg table itself will be returned
		EC.Stack.Push(m_LocalScope[0]);
	else if (m_SuppliedArguments == 1) {
		Types::Table Tab;
		Tab[0] = IFunction::GetArg(0);
		EC.Stack.Push(EC.MC.Save(Tab));
	} else //0 args
		EC.Stack.Push(EC.MC.Save(Types::Table()));
}

void CreateNullFunc::Eval(EvaluationContext& EC)
{
	EC.Stack.Push(NullReference{});
}

void GetRefCountFunc::Eval(EvaluationContext& EC)
{
	Reference WeakRef;
	{
		//            Member Ref = boost::apply_visitor(Utilities::ResolveVisitor(EC),m_LocalScope[0]);
		WeakRef = IFunction::GetArg<CountedReference>(0).GetWeakReference();
		m_LocalScope.ClearIndexValues();
	}
	long long Refcount = EC.MC.GetRefCount(WeakRef);
	EC.Stack.Push(Refcount);
}

void GetMCSizeFunc::Eval(EvaluationContext& EC)
{
	EC.Stack.Push((long long)EC.MC.Size());
}

void PrintFunc::Eval(EvaluationContext& EC)
{
	for (unsigned i = 0; i < m_SuppliedArguments; i++) {
		::Platform::Out() << Utilities::PrintValue(EC, Types::Object(IFunction::GetArg(i))) << " ";
	}
	::Platform::Out() << std::endl;
}

void GlobalExceptionHandleFunc::Eval(EvaluationContext& EC)
{
	const auto& Exception = IFunction::GetArg<CountedReference>(0);
	auto TypeId = boost::apply_visitor(Utilities::Get<long long>(), (*Exception)["TypeId"]);
	auto message = boost::apply_visitor(Utilities::Get<utf8::ustring>(), (*Exception)["Message"]);
	auto name = boost::apply_visitor(Utilities::Get<utf8::ustring>(), (*Exception)["Name"]);
	throw Exceptions::RuntimeException(message, name, TypeId);
}

void CreateExceptionFunc::Eval(EvaluationContext& EC)
{
	Types::Table ExceptionTable;
	ExceptionTable["__EXCEPTION__"] = 1LL;
	ExceptionTable["TypeId"] = m_Id;
	ExceptionTable["Name"] = IEvaluable::Name();
	utf8::ustring message = IEvaluable::Name() + ": ";
	for (unsigned i = 0; i < m_SuppliedArguments; i++) {
		message += Utilities::PrintValue(EC, Types::Object(IFunction::GetArg(i))) + " ";
	}
	ExceptionTable["Message"] = message; //// TODO (Marius#1#): Add possibility to add exception message when creating exception at runtime.
	EC.Stack.Push(EC.MC.Save(ExceptionTable));
}