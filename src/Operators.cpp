#include <memory>
#ifdef DEBUG
#include <iostream>
#endif
#include "../MemoryManagment/include/MemoryController.hpp"
#include "../MemoryManagment/include/Reference.hpp"
#include "../MemoryManagment/include/Exceptions.hpp"
#include "../include/Internal/Table.hpp"
#include "../include/Internal/Object.hpp"
#include "../include/Internal/Get.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/IOperator.hpp"
#include "../include/Internal/Parser.hpp"
#include "../include/Internal/Operators.hpp"
#include "../include/Internal/EvaluationContext.hpp"
using namespace Internal;

namespace{
	struct DoAssignment : public boost::static_visitor<Member>
	{
		EvaluationContext& m_EC;
		const ResolvedToken& m_RHS;
		DoAssignment(EvaluationContext& EC, const ResolvedToken& RHS) :
			m_EC(EC),
			m_RHS(RHS)
		{
		}
		Member operator()(long long val) const
		{
			if (!m_EC.This.IsNull()) { //we have an assignment to a table element. E.g.: table[0] = value
				(*m_EC.This)[val] = m_RHS;
				m_EC.This = NullReference();
				return m_RHS;
			}
			throw std::logic_error("Lhs has to be an Identifier; Is long");
		}
		CountedReference operator()(double val)const
		{
			throw std::logic_error("Lhs has to be an Identifier; Is double");
		}
		Member operator()(const std::string& s)const
		{
			Types::Scope Scope((NullReference()));
			if (m_EC.This.IsNull())
				Scope = m_EC.Scope();
			else {
				Scope = m_EC.This;
				m_EC.This = NullReference();
			}
			(*Scope)[s] = m_RHS;
			return m_RHS;
		}
		CountedReference operator()(const std::shared_ptr<IFunction>& op)const
		{
			throw std::logic_error("Lhs has to be an Identifier; Is Function ");
		}
		Member operator()(const CountedReference& Ref)const
		{
			if (Ref.IsNull())
				throw Exceptions::NullReferenceException("Assignment to a nullreference");
			return Ref;
		}
		Member operator()(const NullReference&)const
		{
			throw Exceptions::NullReferenceException("Assignment to a nullreference");
		}
	};
	struct DoNamedAssignment : public boost::static_visitor<Member>
	{
		EvaluationContext& m_EC;
		const ResolvedToken& m_RHS;
		DoNamedAssignment(EvaluationContext& EC, const ResolvedToken& RHS) :
			m_EC(EC),
			m_RHS(RHS)
		{
		}
		Member operator()(long long val) const
		{
			throw std::logic_error("Lhs has to be an Identifier; Is long");
		}
		CountedReference operator()(double val)const
		{
			throw std::logic_error("Lhs has to be an Identifier; Is double");
		}
		Member operator()(const std::string& s)const
		{
			Types::Table NamedArgs;
			Types::Table Args;
			Args[s] = m_RHS;
			NamedArgs["__NAMEDARGS__"] = m_EC.MC.Save(Args);
			return m_EC.MC.Save(NamedArgs);
		}
		CountedReference operator()(const std::shared_ptr<IFunction>& op)const
		{
			throw std::logic_error("Lhs has to be an Identifier; Is function ");
		}
		Member operator()(const CountedReference& Ref)const
		{
			if (Ref.IsNull())
				throw Exceptions::NullReferenceException("Assignment to a nullreference");
			throw std::logic_error("Lhs has to be an Identifier; Is table ");
		}
		Member operator()(const NullReference&)const
		{
			throw Exceptions::NullReferenceException("Assignment to a nullreference");
		}
	};

	class Add
		: public boost::static_visitor<ResolvedToken >
	{
	public:
		ResolvedToken operator()(long long lhs, long long rhs) const
		{
			return lhs + rhs;
		}
		ResolvedToken operator()(long long lhs, double rhs) const
		{
			return lhs + rhs;
		}
		ResolvedToken operator()(double lhs, long long rhs) const
		{
			return lhs + rhs;
		}
		ResolvedToken operator()(double lhs, double rhs) const
		{
			return lhs + rhs;
		}
		ResolvedToken operator()(const utf8::ustring& lhs, const utf8::ustring& rhs) const
		{
			return lhs + rhs;
		}
		template <typename T, typename U>
		ResolvedToken operator()(T lhs, U rhs)const
		{
			throw Exceptions::TypeException("operator + is not supported on operands with types " + utf8::ustring(Type<T>::Name()) + " and " + Type<U>::Name());
		}
	};

	class Sub
		: public boost::static_visitor<NumberToken >
	{
	public:
		template <typename T, typename U>
		auto operator()(T lhs, U rhs) -> decltype(lhs - rhs) const
		{
			return lhs - rhs;
		}
	};

	class UnarySub
		: public boost::static_visitor<NumberToken >
	{
	public:
		template <typename T>
		T operator()(T Val)const
		{
			return Val * (-1);
		}
	};

	bool QualifiesForUnary(LastCharType LastChar)
	{
		return LastChar == LastCharType::LikeOpeningBracket || LastChar == LastCharType::BinaryOp || LastChar == LastCharType::Start;
	}

	class Multi
		: public boost::static_visitor<NumberToken>
	{
	public:
		template <typename T, typename U>
		auto operator()(T lhs, U rhs) -> decltype(lhs*rhs) const
		{
			return lhs * rhs;
		}
	};

	class Divide
		: public boost::static_visitor<NumberToken>
	{
	public:
		template <typename T, typename U>
		auto operator()(T lhs, U rhs) -> decltype(lhs / rhs) const
		{
			return lhs / rhs;
		}
	};

	class Pow
		: public boost::static_visitor<NumberToken>
	{
	public:
		template <typename T, typename U>
		double operator()(T lhs, U rhs) const
		{
			return std::pow(lhs, rhs);
		}
		NumberToken operator()(long long lhs, long long rhs) const
		{
			if (rhs >= 0)
				return std::llround(std::pow(lhs, rhs));
			else
				return std::pow(lhs, rhs);
		}
	};


	class Modulo : public boost::static_visitor<NumberToken>
	{
	public:
		NumberToken operator()(long long lhs, long long rhs) const
		{
			return lhs % rhs;
		}
		NumberToken operator()(double lhs, long long rhs) const
		{
			return std::fmod(lhs, rhs);
		}
		NumberToken operator()(long long lhs, double rhs) const
		{
			return std::fmod(lhs, rhs);
		}
		NumberToken operator()(double lhs, double rhs) const
		{
			return std::fmod(lhs, rhs);
		}
	};

	class Test
		: public boost::static_visitor<NumberToken>
	{
	public:
		template <typename T>
		NumberToken operator()(T rhs) const
		{
			return ++rhs;
		}
	};

	struct Less : public boost::static_visitor<ValueToken>
	{
		template<typename T, typename U>
		ValueToken operator()(T LHS, U RHS) const
		{
			if (LHS < RHS)
				return LHS;
			else
				return NullReference();
		}
	};

	struct Greater : public boost::static_visitor<ValueToken>
	{
		template<typename T, typename U>
		ValueToken operator()(T LHS, U RHS) const
		{
			if (LHS > RHS)
				return LHS;
			else
				return NullReference();
		}
	};

	struct Not : public boost::static_visitor<ValueToken>
	{
		ValueToken operator()(NullReference) const
		{
			return 1ll;
		}
		ValueToken operator()(const CountedReference& Ref) const
		{
			if (Ref.IsNull())
				return 1ll;
			else
				return NullReference();
		}
		template<typename T>
		ValueToken operator()(T) const
		{
			return NullReference();
		}
	};

	struct GetTable : public boost::static_visitor<CountedReference>
	{
		EvaluationContext& m_EC;
		GetTable(EvaluationContext& EC) :
			m_EC(EC)
		{
		}
		template<typename T>
		CountedReference operator()(const T& Val) const
		{ //Value, so we create a new table with the LHS as the first value
			Types::Table Tab;
			Tab.Add(Val);
			if (m_EC.Signal(SignalType::FuncCall))
				Tab.Add("__ARGCOUNT__", 2ll); //New table: Must have two args eg.: (1,2)
			return m_EC.MC.Save(Tab);
		}
		CountedReference operator()(const CountedReference& Ref) const
		{ //LHS is already a table so we pass it back to append the RHS
			if (m_EC.Signal(SignalType::FuncCall)) {
				auto it = (*Ref).Find("__ARGCOUNT__");
				if (it != (*Ref).KeyEnd())
					boost::get<long long&>(it->second) += 1ll;
				else {//table declared outside the funccall, treat it as one argument
					Types::Table Tab;
					Tab.Add(Ref);
					Tab.Add("__ARGCOUNT__", 2ll); //New table: Must have two args eg.: t = (1,2); func(t,3) => 2 Args
					return m_EC.MC.Save(Tab);
				}
			}

			return Ref;
		}
	};

	class ArgListStartMarker : public IFunction
	{
	public:
		ArgListStartMarker() :
			IFunction("", "__ALSM__", 0, 0)
		{}
		void Eval(EvaluationContext&)
		{
			throw std::logic_error("ArgListStartMarker evaluated directly, shouldn't be possible");
		}
	};

	struct ThrowIfNoFunc : public boost::static_visitor<>
	{
		ThrowIfNoFunc()
		{}
		void operator()(long long) const
		{
			throw Exceptions::TypeException("Expected function; Is long");
		}
		void operator()(double)const
		{
			throw Exceptions::TypeException("Expected function; Is double");
		}
		void operator()(const std::shared_ptr<IOperator>& op)const
		{
			throw Exceptions::TypeException("Expected function; Is operator " + op->Representation());
		}
		void operator()(const utf8::ustring& str)const
		{
			throw Exceptions::TypeException("Expected function; Is string");
		}
		void operator()(const Reference& R)const
		{
			if (R.IsNull())
				throw Exceptions::NullReferenceException("Calling a nullreference");
			else { //check whether the table has an @-specialmethod which makes it callable
				if ((*R).Contains("@call"))
					boost::apply_visitor(*this, (*R)["@call"]);//recursivly check whether @Call is a function
				else
					throw Exceptions::TypeException("Expected function; Is table");
			}
		}
		void operator()(NullReference)const
		{
			throw Exceptions::NullReferenceException("Calling a nullreference");
		}
		void operator()(const std::shared_ptr<IFunction>& op)const
		{
		}
	};

	struct IsMarker : public boost::static_visitor<bool>
	{
		template <typename T>
		bool operator()(const T&) const
		{
			return false;
		}
		bool operator()(const std::shared_ptr<IFunction>& Func) const
		{
			return *Func == "__ALSM__";
		}
	};
}

void Internal::ParseAssignment(ParserContext& PC)
{// TODO (Marius#6#): Add recognition of funccall to support named parameter
	//The parser would let some tokens through, which don't make sense in an assignment. Other Tokens like faulty placed operators will be correctly handled
	//by the Parser
	if (PC.LastToken() == TokenType::Long || PC.LastToken() == TokenType::Double || PC.LastToken() == TokenType::KeywordWithValue || PC.LastToken() == TokenType::String)
		throw std::logic_error("Can't assign to a literal");
	PC.Parse(UnparsedToken(std::make_shared<AssignmentOp>()));
	PC.LastToken() = TokenType::Assignment;
	PC.State() = ParserState::Assignment;
};

// TODO (Marius#8#): Prevent more than one assignment in a row like "f(arg=result=1)"
void AssignmentOp::Eval(EvaluationContext& EC)
{
    Types::Object RHS = EC.EvalStack.back();
    EC.EvalStack.pop_back();
    Types::Object LHS = EC.EvalStack.back();
    EC.EvalStack.pop_back();
    ResolvedToken RHSToken(Utilities::Resolve(EC,RHS));
    EC.This = LHS.This();
    if( EC.Signal(SignalType::FuncCall) )
        EC.Stack.Push(LHS.Visit(DoNamedAssignment(EC,RHSToken)));
    else
        EC.Stack.Push(LHS.Visit(DoAssignment(EC,RHSToken)));
}

void PlusOp::Eval(EvaluationContext& EC)
{
	//NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	ResolvedToken rhs = Utilities::Resolve(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	//NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	ResolvedToken lhs = Utilities::Resolve(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Add A; //Created a lvalue here because otherwise the compiler would complain about discarding qualifiers in the function call below
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(A, lhs, rhs)));
}

void BinaryMinusOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Sub S;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(S, lhs, rhs)));
}

void UnaryMinusOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	UnarySub S;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(S, rhs)));
}

LastCharType MinusToken::Tokenize(TokenizeContext& TC) const
{
	if (QualifiesForUnary(TC.LastChar()) || (TC.LastChar() == LastCharType::None && QualifiesForUnary(TC.LastChar().Before()))) {
		TC.OutputQueue().push_back(std::make_shared<UnaryMinusOp>());
		return LastCharType::UnaryPrefixOp;
	} else {
		TC.OutputQueue().push_back(std::make_shared<BinaryMinusOp>());
		return LastCharType::BinaryOp;
	}
}

void MultiOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Multi M;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(M, lhs, rhs)));
}

void DivOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Divide D;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(D, lhs, rhs)));
}

void ExponentOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Pow P;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(P, lhs, rhs)));
}

void ModuloOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	Modulo M;
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(M, lhs, rhs)));
}

void TestOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Test(), rhs)));
}

void AccesOp::Eval(EvaluationContext& EC)
{
	std::string Identifier(EC.EvalStack.back().Visit(Utilities::Get<std::string>("Expected identifier on the right hand side of operator '.'")));
	EC.EvalStack.pop_back();
	const Types::Scope& ScopeRef(Utilities::GetWithResolve<Types::Scope>(EC, EC.EvalStack.back()));
	EC.EvalStack.pop_back();
	if (ScopeRef.IsNull())
		throw Exceptions::NullReferenceException("Dereferencing a nullreference");
	Types::Object Val(Identifier);
	Val.This(ScopeRef);
	EC.EvalStack.push_back(Val);
}

void LessOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Less(), lhs, rhs)));
}

void GreaterOp::Eval(EvaluationContext& EC)
{
	NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Greater(), lhs, rhs)));
}

void NotOp::Eval(EvaluationContext& EC)
{
	Types::Object rhs = EC.EvalStack.back();
	EC.EvalStack.pop_back();
	ResolvedToken Token(Utilities::Resolve(EC, rhs));
	EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Not(), Token)));
}

void IndexOpeningBracket::Eval(EvaluationContext& EC)
{
	if (EC.EvalStack.size() < 2)
		throw std::logic_error("Missing input near '['");
	long long index;
	utf8::ustring key;
	Types::Object index_or_key = EC.Stack.Pop();

	if (!Utilities::ResolvesToAndGet<long long>(EC, index_or_key, index) && !Utilities::ResolvesToAndGet<utf8::ustring>(EC, index_or_key, key)) {
		throw Exceptions::TypeException("operator '[]' expects integer or string type");
	}
	index_or_key.This(Utilities::GetWithResolve<CountedReference>(EC, EC.Stack.Pop(), "Expected table left of operator '[]'"));
	EC.Stack.Push(index_or_key);
}

void Internal::ParseIndexOpeningBracket(ParserContext& PC)
{
	PC.ThrowIfUnexpected(TokenType::OpeningBracket);
	//the generic parsing would allow nothing before an openingbracket. E.g.: (5+5)*2
	//But this wouldn't make any sense for our index-bracket
	if (PC.LastToken() == TokenType::None)
		throw std::logic_error("Missing input before '['");
	else if (PC.LastToken() == TokenType::OpBinary || PC.LastToken() == TokenType::OpUnaryPrefix)
		throw std::logic_error("Unexpected operator before '['");
	PC.Parse(std::make_shared<IndexOpeningBracket>());
}

LastCharType IndexOpeningBracketToken::Tokenize(TokenizeContext& TC) const
{
	TC.OutputQueue().push_back(Parsable("[", &ParseIndexOpeningBracket));
	return LastCharType::LikeOpeningBracket;
}

void TableOp::Eval(EvaluationContext& EC)
{
	if (EC.EvalStack.size() < 2)
		throw std::logic_error("Missing input near operator ','");
	ResolvedToken RHS = Utilities::Resolve(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	ResolvedToken LHS = Utilities::Resolve(EC, EC.EvalStack.back());
	EC.EvalStack.pop_back();
	CountedReference Table{ boost::apply_visitor(GetTable(EC), LHS) };
	(*Table).Add(RHS);
	EC.EvalStack.push_back(Types::Object(Table));
}

void Internal::ParseTableOp(ParserContext& PC)
{
	PC.Parse(std::make_shared<TableOp>());
	PC.LastToken() = TokenType::ArgSeperator;
	//The table-op doesn't accept everything a anormal binary-op would accept
	PC.UnexpectedToken() = (TokenType::OpeningBracket /*| TokenType::Value*/); // FIXME (Marius#8#): OR-ing a TokenType with a composed TokenType doesn't work
}

void OpeningBracket::Eval(EvaluationContext& EC)
{
	if (!EC.EvalStack.empty()) {
		ResolvedToken Tok(Utilities::Resolve(EC, EC.EvalStack.back()));
		if (boost::apply_visitor(Utilities::IsFunc(), Tok)) {
#ifdef DEBUG
			std::cout << "Is funccall\n";
#endif
			EC.SetSignal(SignalType::FuncCall);
			EC.EvalStack.push_back(Types::Object(std::make_shared<ArgListStartMarker>()));
		} else {
			EC.SetSignal(SignalType::None);//Reset signal state to prevent false detection of a funccall e.g.: func f(a) ... end; f(5*(5+5))
#ifdef DEBUG     //                                                                                     |   |   |------|
			std::cout << "No funccall\n"; //                                                                        Set funccall   Signal funccall is set so this belong to a call !error!
#endif                        //                                                        instead:            |--> reset Signal, so no funccall is detected with the next closingbracket
		}
	}
}

void OpeningBracketFuncCall::Eval(EvaluationContext& EC)
{
	ResolvedToken Tok(Utilities::Resolve(EC, EC.EvalStack.back()));
	boost::apply_visitor(ThrowIfNoFunc(), Tok);
	EC.SetSignal(SignalType::FuncCall);
	EC.EvalStack.push_back(Types::Object(std::make_shared<ArgListStartMarker>()));
}

void OpeningBracketNoFuncCall::Eval(EvaluationContext& EC)
{
	EC.SetSignal(SignalType::None);//Reset signal state to prevent false detection of a funccall e.g.: func f(a) ... end; f(5*(5+5))
#ifdef DEBUG            //                                                                                     |   |   |------|
	std::cout << "No funccall\n"; //                                                                        Set funccall   Signal funccall is set so this belong to a call !error!
#endif                        //                                                        instead:            |--> reset Signal, so no funccall is detected with the next closingbracket

}

void Internal::ParseOpeningBracket(ParserContext& PC)
{
	PC.ThrowIfUnexpected(TokenType::OpeningBracket);
	// Compare with openingbracket parsing in the parser
	if (PC.LastToken() == TokenType::Identifier) {   //the generic bracket parsing algorithm will take care of the Bracket, nothing more to do for us here
		auto OB = std::make_shared<OpeningBracketFuncCall>();
		PC.Parse(OB);
		PC.OutputQueue().push_back(OB);
#ifdef DEBUG
		std::cout << "OpeningBracketFuncCall\n";
#endif
		//PC.OperatorStack().pop();
	} else if (PC.LastToken() == TokenType::Assignment || PC.LastToken() == TokenType::Long || PC.LastToken() == TokenType::Double) {
		PC.Parse(std::make_shared<OpeningBracketNoFuncCall>());
		//PC.OutputQueue().push_back(OB);
#ifdef DEBUG
		std::cout << "OpeningBracketNoFuncCall\n";
#endif
	}
	//    else if(PC.LastToken() == TokenType::OpeningBracket )
	//    {
	//        auto OB = std::make_shared<OpeningBracket>();
	//        PC.Parse(OB);
	//        //PC.OutputQueue().push_back(OB);
	//#ifdef DEBUG
	//        std::cout << "OpeningBracket\n";
	//#endif
	//    }
	else {
		auto OB = std::make_shared<class OpeningBracket>();
		PC.Parse(OB);
		PC.OutputQueue().push_back(OB);
#ifdef DEBUG
		std::cout << "OpeningBracket\n";
#endif
	}
}

LastCharType OpeningBracketToken::Tokenize(TokenizeContext& TC) const
{
	TC.OutputQueue().push_back(Parsable("(", &ParseOpeningBracket));
	//        TC.OutputQueue().push_back(&ParseOpeningBracket);
	return LastCharType::LikeOpeningBracket;
}

void ClosingBracket::Eval(EvaluationContext& EC)
{
	if (EC.Signal(SignalType::FuncCall)) {
		EC.DropSignal(); //We're not in a FuncCall anymore if the body of the func gets evaluated
		ResolvedToken Args = Utilities::Resolve(EC, EC.Stack.Pop());
		//Check, whether there are arguments. If there's immediately an argliststartmarker then there are no args
		long long ArgCount = boost::apply_visitor(Utilities::ArgCounter(EC), Args);
#ifdef DEBUG
		std::cout << "No. of provided Args for funccall: " << ArgCount << std::endl;
#endif
		if (ArgCount >= 1) //there should still be an ArgListStartMarker
		{
			if (!EC.Stack.Top().Visit(IsMarker())) {
				throw std::logic_error("Missing ArgListMarker");
			}
			EC.Stack.Pop();
		}
		const Types::Object& FuncObj = EC.Stack.Pop();
		EC.Call(FuncObj, Args, ArgCount);
		//            ResolvedToken FuncToken = Utilities::Resolve(EC,FuncObj);
		//            const std::shared_ptr<IFunction>& Func = boost::apply_visitor(Utilities::GetFunc(EC),FuncToken);
		//
		//            Func->SuppliedArguments(Args,ArgCount);
		//            Func->This(FuncObj.This());
		//            try
		//            { //Make sure native exceptions are properly transformed to internal exceptions
		//                Func->Eval(EC);
		//            }
		//            catch( Exceptions::RuntimeException& e )
		//            {
		//                Func->This(NullReference());
		//                EC.Throw(e);
		//                EC.EndScope();
		//            }
		//            Func->This(NullReference()); //Very Important! Reset the this-ref to prevent a crash on shutdown. The MC gets deleted before the functions
		//                                        //who would still hold a ref to their this-scopes. When  funcs get deleted the and the ref does it's decref, the MC is already gone -> Crash!
	} else
		EC.DropSignal();
}

void Internal::ParseClosingBracket(ParserContext& PC)
{
	//All the parsing has to be done here, because we have to pop the Openingbracket from stack but
	//not on the outputqueue
	PC.ThrowIfUnexpected(TokenType::ClosingBracket, "Unexpected closing bracket");
	if (PC.ExpectedBracket().empty()) //Closingbracket but no bracket expected? Definitely a bracket mismatch
		throw std::logic_error("No closing bracket expected");
	else if (*PC.ExpectedBracket().top() != ")")
		throw std::logic_error("Expected '" + PC.ExpectedBracket().top()->Representation() + "'");
	else
		PC.ExpectedBracket().pop();
	if (PC.LastToken() == TokenType::ArgSeperator)
		throw std::logic_error("Expected an expression between ',' and ')'");

	while (!PC.OperatorStack().empty()) {
		auto Temp = PC.OperatorStack().top();
		if (*Temp == *PC.OpeningBracket()) {
			//it's the opening bracket, we're done
			//Pop the left parenthesis from the stack, but not onto the output queue.
			PC.OperatorStack().pop();
			PC.State().Restore();
			PC.LastToken() = TokenType::ClosingBracket;
			PC.OutputQueue().push_back(std::make_shared<class ClosingBracket>());
			return;
		} else {
			PC.OutputQueue().push_back(Temp);
			PC.OperatorStack().pop();
		}
	}
	//if it reaches here, there is a bracket mismatch
	throw std::logic_error("bracket mismatch");//If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
}

LastCharType ClosingBracketToken::Tokenize(TokenizeContext& TC) const
{
	TC.OutputQueue().push_back(Parsable(")", &ParseClosingBracket));
	return LastCharType::LikeClosingBracket;
}

void ThrowOp::Eval(EvaluationContext& EC)
{
	if (EC.Stack.Empty())
		throw std::logic_error("Missing argument for throw");
	auto Ex = Utilities::GetWithResolve<CountedReference>(EC, EC.Stack.Pop(), "\"throw\" argument has to be an exception type");
	if (!(*Ex).Contains("__EXCEPTION__")) {
		EC.Throw(Exceptions::TypeException("\"throw\" argument has to be an exception type"));
		EC.EndScope();
		return;
	}
	EC.Throw(Ex);
	EC.EndScope();
}