#ifndef OPERATORS_H_INCLUDED
#define OPERATORS_H_INCLUDED
#include <cmath>
#include <boost/make_shared.hpp>
#include "../IToken.hpp" //For MinusToken, otherwise compilererror in Parser.cpp
#include "../Parsable.hpp"
#include "Utilities.hpp"
#include "Is.hpp"
#include "../Exceptions.hpp"
namespace Internal
{
class AssignmentOp : public IOperator
{
public:
    AssignmentOp():IOperator("Assignment operator","=",-10,ArityType::Binary,AssociativityType::Right)
    {}
    void Eval(EvaluationContext& EC);
};
inline void ParseAssignment(ParserContext& PC)
{// TODO (Marius#6#): Add recognition of funccall to support named parameter
    //The parser would let some tokens through, which don't make sense in an assignment. Other Tokens like faulty placed operators will be correctly handled
    //by the Parser
    if( PC.LastToken() == TokenType::Long || PC.LastToken() == TokenType::Double || PC.LastToken() == TokenType::KeywordWithValue )
        throw std::logic_error("Can't assign to a number literal");
    PC.Parse(boost::make_shared<AssignmentOp>());
    PC.LastToken() = TokenType::Assignment;
    PC.State() = ParserState::Assignment;
};
class PlusOp : public IOperator
{
    class Add
        : public boost::static_visitor<NumberToken >
    {
    public:
        template <typename T, typename U>
        auto operator()( T lhs, U rhs) -> decltype(lhs + rhs) const
        {
            return lhs + rhs;
        }
    };

public:
    PlusOp():IOperator("Plus","+",10,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Add A; //Created a lvalue here because otherwise the compiler would complain about discarding qualifiers in the function call below
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(A,lhs,rhs)));
    }
};
class BinaryMinusOp : public IOperator
{
    class Sub
        : public boost::static_visitor<NumberToken >
    {
    public:
        template <typename T, typename U>
        auto operator()( T lhs, U rhs) -> decltype(lhs - rhs) const
        {
            return lhs - rhs;
        }
    };
public:
    BinaryMinusOp():IOperator("Binary minus","-",10,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Sub S;
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(S,lhs, rhs)));
    }
};
class UnaryMinusOp : public IOperator
{
    class Sub
        : public boost::static_visitor<NumberToken >
    {
    public:
        template <typename T>
        T operator()( T Val)const
        {
            return Val * (-1);
        }
    };
public:
    UnaryMinusOp():IOperator("Unary minus","-",30,ArityType::UnaryPrefix,AssociativityType::Right)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Sub S;
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(S, rhs)));
    }
};

class MinusToken : public IToken
{
    bool QualifiesForUnary(LastCharType LastChar) const
    {
        return LastChar == LastCharType::LikeOpeningBracket || LastChar == LastCharType::BinaryOp || LastChar == LastCharType::Start;
    }
public:
    MinusToken():
        IToken("-")
    {}
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        if( QualifiesForUnary(TC.LastChar()) || (TC.LastChar() == LastCharType::None && QualifiesForUnary(TC.LastChar().Before())) )
        {
            TC.OutputQueue().push_back(boost::make_shared<UnaryMinusOp>());
            return LastCharType::UnaryPrefixOp;
        }
        else
        {
            TC.OutputQueue().push_back(boost::make_shared<BinaryMinusOp>());
            return LastCharType::BinaryOp;
        }
    }
};
class MultiOp : public IOperator
{
    class Multi
        : public boost::static_visitor<NumberToken>
    {
    public:
        template <typename T, typename U>
        auto operator()( T lhs, U rhs) -> decltype(lhs*rhs) const
        {
            return lhs * rhs;
        }
    };
public:
    MultiOp():IOperator("Multiplication","*",20,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Multi M;
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(M,lhs, rhs)));
    }
};

class DivOp : public IOperator
{
    class Divide
        : public boost::static_visitor<NumberToken>
    {
    public:
        template <typename T, typename U>
        auto operator()( T lhs, U rhs) -> decltype(lhs/rhs) const
        {
            return lhs / rhs;
        }
    };
public:
    DivOp():IOperator("Division","/",20,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Divide D;
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(D,lhs, rhs)));
    }
};

class ExponentOp : public IOperator
{
    class Pow
        : public boost::static_visitor<NumberToken>
    {
    public:
        template <typename T, typename U>
        double operator()( T lhs, U rhs) const
        {
            return std::pow(lhs,rhs);
        }
        NumberToken operator()( long long lhs, long long rhs) const
        {
            if( rhs >= 0 )
                return std::llround(std::pow(lhs,rhs));
            else
                return std::pow(lhs,rhs);
        }
    };
public:
    ExponentOp():IOperator("Exponentation","^",30,ArityType::Binary,AssociativityType::Right)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Pow P;
        EC.EvalStack.push_back( Types::Object(boost::apply_visitor(P,lhs, rhs)));
    }
};

class ModuloOp : public IOperator
{
    class Modulo : public boost::static_visitor<NumberToken>
    {
    public:
        NumberToken operator()( long long lhs, long long rhs) const
        {
            return lhs % rhs;
        }
        NumberToken operator()( double lhs, long long rhs) const
        {
            return std::fmod(lhs,rhs);
        }
        NumberToken operator()( long long lhs, double rhs) const
        {
            return std::fmod(lhs,rhs);
        }
        NumberToken operator()( double lhs, double rhs) const
        {
            return std::fmod(lhs,rhs);
        }
    };
public:
    ModuloOp():IOperator("Modulo","%",20,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        Modulo M;
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(M,lhs, rhs)));
    }
};

class TestOp : public IOperator
{
    class Test
        : public boost::static_visitor<NumberToken>
    {
    public:
        template <typename T>
        NumberToken operator()( T rhs) const
        {
            //std::cout << lhs++ << std::endl;
            return ++rhs;
        }
    };
public:
    TestOp():IOperator("TestOp","++",30,ArityType::UnaryPostfix,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Test(),rhs)));
    }
};

class AccesOp : public IOperator
{
public:
    AccesOp():IOperator("Accesoperator",".",50,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        std::string Identifier(EC.EvalStack.back().Visit(Utilities::Get<std::string>("Expected identifier on the right hand side of operator '.'")));
        EC.EvalStack.pop_back();
        const Types::Scope& ScopeRef(Utilities::GetWithResolve<Types::Scope>(EC, EC.EvalStack.back()));
        EC.EvalStack.pop_back();
        if( ScopeRef.IsNull() )
            throw Exceptions::NullReferenceException("Dereferencing a nullreference");
        Types::Object Val(Identifier);
        Val.This(ScopeRef);
        EC.EvalStack.push_back(Val);
    }
};

class LessOp : public IOperator
{
    struct Less : public boost::static_visitor<ValueToken>
    {
        template<typename T, typename U>
        ValueToken operator()(T LHS, U RHS) const
        {
            if( LHS < RHS )
                return LHS;
            else
                return NullReference();
        }
    };
public:
    LessOp():IOperator("Lessoperator","<",5,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Less(),lhs,rhs)));
    }
};

class GreaterOp : public IOperator
{
    struct Greater : public boost::static_visitor<ValueToken>
    {
        template<typename T, typename U>
        ValueToken operator()(T LHS, U RHS) const
        {
            if( LHS > RHS )
                return LHS;
            else
                return NullReference();
        }
    };
public:
    GreaterOp():IOperator("Greateroperator",">",5,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        NumberToken rhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        NumberToken lhs = Utilities::GetNumberToken(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Greater(),lhs,rhs)));
    }
};

class NotOp : public IOperator
{
    struct Not : public boost::static_visitor<ValueToken>
    {
        ValueToken operator()(NullReference) const
        {
            return 1ll;
        }
        ValueToken operator()(const CountedReference& Ref) const
        {
            if( Ref.IsNull() )
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
public:
    NotOp():IOperator("Notoperator","!",30,ArityType::UnaryPrefix,AssociativityType::Right)
    {}
    void Eval(EvaluationContext& EC)
    {
        Types::Object rhs = EC.EvalStack.back();
        EC.EvalStack.pop_back();
        ResolvedToken Token(Utilities::Resolve(EC,rhs));
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Not(),Token)));
    }
};


class IndexOpeningBracket : public IOperator
{

public:
    IndexOpeningBracket():IOperator("Index Opening Bracket","[",50,ArityType::UnaryPrefix,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( EC.EvalStack.size() < 2 )
            throw std::logic_error("Missing input near '['");
        long long Index = Utilities::GetWithResolve<long long>(EC,EC.EvalStack.back(),"Index is not an integer");
        EC.EvalStack.pop_back();
        Types::Object O(Index);
        O.This(Utilities::GetWithResolve<CountedReference>(EC,EC.EvalStack.back(),"Expected table left of operator '[]'"));
        EC.EvalStack.pop_back();
        EC.EvalStack.push_back(O);
    }
};

inline void ParseIndexOpeningBracket(ParserContext& PC)
{
    PC.ThrowIfUnexpected(TokenType::OpeningBracket);
    //the generic parsing would allow nothing before an openingbracket. E.g.: (5+5)*2
    //But this wouldn't make any sense for our index-bracket
    if( PC.LastToken() == TokenType::None )
        throw std::logic_error("Missing input before '['");
    else if( PC.LastToken() == TokenType::OpBinary || PC.LastToken() == TokenType::OpUnaryPrefix )
        throw std::logic_error("Unexpected operator before '['");
    PC.Parse(boost::make_shared<IndexOpeningBracket>());

}
class IndexOpeningBracketToken : public IToken
{
public:
    IndexOpeningBracketToken():
        IToken("[")
    {

    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(Parsable("[", &ParseIndexOpeningBracket));
        return LastCharType::LikeOpeningBracket;
    }
};
class IndexClosingBracket : public IOperator
{

public:
    IndexClosingBracket():IOperator("Index Closing Bracket","]",50,ArityType::UnaryPostfix,AssociativityType::Left)
    {}
    void Eval(EvaluationContext&)
    {
        //throw std::logic_error("Bracket missmatch");
    }
};


class TableOp : public IOperator
{
    struct GetTable : public boost::static_visitor<CountedReference>
    {
        EvaluationContext& m_EC;
        GetTable(EvaluationContext& EC):
            m_EC(EC)
        {
        }
        template<typename T>
        CountedReference operator()(const T& Val) const
        { //Value, so we create a new table with the LHS as the first value
            Types::Table Tab;
            Tab.Add(Val);
            if( m_EC.Signal(SignalType::FuncCall) )
                Tab.Add("__ARGCOUNT__",2ll); //New table: Must have two args eg.: (1,2)
            return m_EC.MC.Save(Tab);
        }
        CountedReference operator()(const CountedReference& Ref) const
        { //LHS is already a table so we pass it back to append the RHS
            if( m_EC.Signal(SignalType::FuncCall) )
            {
                auto it = (*Ref).Find("__ARGCOUNT__");
                if( it != (*Ref).KeyEnd() )
                    boost::get<long long&>((*Ref)["__ARGCOUNT__"]) += 1ll;
                else
                {//table declared outside the funccall, treat it as one argument
                    Types::Table Tab;
                    Tab.Add(Ref);
                    Tab.Add("__ARGCOUNT__",2ll); //New table: Must have two args eg.: t = (1,2); func(t,3) => 2 Args
                    return m_EC.MC.Save(Tab);
                }
            }

            return Ref;
        }
    };
public:
    TableOp():IOperator("Table operator",",",-15,ArityType::Binary,AssociativityType::Left)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( EC.EvalStack.size() < 2 )
            throw std::logic_error("Missing input near operator ','");
        ResolvedToken RHS = Utilities::Resolve(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        ResolvedToken LHS = Utilities::Resolve(EC, EC.EvalStack.back());
        EC.EvalStack.pop_back();
        CountedReference Table(boost::apply_visitor(GetTable(EC),LHS));
        (*Table).Add(RHS);
        EC.EvalStack.push_back(Types::Object(Table));
    }
};
inline void ParseTableOp(ParserContext& PC)
{
    PC.Parse(boost::make_shared<TableOp>());
    PC.LastToken() = TokenType::ArgSeperator;
    //The table-op doesn't accept everything a anormal binary-op would accept
    PC.UnexpectedToken() = (TokenType::OpeningBracket /*| TokenType::Value*/); // FIXME (Marius#8#): OR-ing a TokenType with a composed TokenType doesn't work
}

class OpeningBracket : public IOperator
{
protected:
    class ArgListStartMarker : public IFunction
    {
        public:
        ArgListStartMarker():
            IFunction("","__ALSM__",0,0)
        {}
        void Eval(EvaluationContext&)
        {
            throw std::logic_error("ArgListStartMarker evaluated directly, shouldn't be possible");
        }
    };
    struct IsFunc : public boost::static_visitor<bool>
    {
        template <typename T>
        bool operator()(const T&) const
        {
            return false;
        }
        bool operator()(const boost::shared_ptr<IFunction>& Func) const
        {
            return *Func != "__ALSM__";
        }
    };
public:
    OpeningBracket():IOperator("","(",50,ArityType::UnaryPostfix,AssociativityType::Left) //Precedence doesn't matter because bracket ops receive special treatment
    {}
    void Eval(EvaluationContext& EC)
    {
        if( !EC.EvalStack.empty() )
        {
            ResolvedToken Tok(Utilities::Resolve(EC,EC.EvalStack.back()));
            if( boost::apply_visitor(IsFunc(), Tok) )
            {
                #ifdef DEBUG
                std::cout << "Is funccall\n";
                #endif
                EC.SetSignal(SignalType::FuncCall);
                EC.EvalStack.push_back(Types::Object(boost::make_shared<ArgListStartMarker>()));
            }
            else
            {
                EC.SetSignal(SignalType::None);//Reset signal state to prevent false detection of a funccall e.g.: func f(a) ... end; f(5*(5+5))
                #ifdef DEBUG     //                                                                                     |   |   |------|
                std::cout << "No funccall\n"; //                                                                        Set funccall   Signal funccall is set so this belong to a call !error!
                #endif                        //                                                        instead:            |--> reset Signal, so no funccall is detected with the next closingbracket
            }
        }
    }
};
class OpeningBracketFuncCall : public OpeningBracket
{
    struct ThrowIfNoFunc: public boost::static_visitor<>
    {
        ThrowIfNoFunc()
        {}
        void operator()(long long ) const
        {
            throw Exceptions::TypeException("Expected function; Is long");
        }
        void operator()(double )const
        {
            throw Exceptions::TypeException("Expected function; Is double");
        }
        void operator()(const boost::shared_ptr<IOperator>& op)const
        {
            throw Exceptions::TypeException("Expected function; Is operator " + op->Representation());
        }
        void operator()(const Reference& R)const
        {
            if( R.IsNull() )
                throw Exceptions::NullReferenceException("Calling a nullreference");
            else
                throw Exceptions::TypeException("Expected function; Is table");
        }
        void operator()(NullReference)const
        {
            throw Exceptions::NullReferenceException("Calling a nullreference");
        }
        void operator()(const boost::shared_ptr<IFunction>& op)const
        {
        }
    };
public:
    OpeningBracketFuncCall():
        OpeningBracket()
    {}
    void Eval(EvaluationContext& EC)
    {
        ResolvedToken Tok(Utilities::Resolve(EC,EC.EvalStack.back()));
        boost::apply_visitor(ThrowIfNoFunc(),Tok);
        EC.SetSignal(SignalType::FuncCall);
        EC.EvalStack.push_back(Types::Object(boost::make_shared<ArgListStartMarker>()));
    }
};
class OpeningBracketNoFuncCall : public OpeningBracket
{
public:
    OpeningBracketNoFuncCall():
        OpeningBracket()
    {}
    void Eval(EvaluationContext& EC)
    {
        EC.SetSignal(SignalType::None);//Reset signal state to prevent false detection of a funccall e.g.: func f(a) ... end; f(5*(5+5))
        #ifdef DEBUG            //                                                                                     |   |   |------|
        std::cout << "No funccall\n"; //                                                                        Set funccall   Signal funccall is set so this belong to a call !error!
        #endif                        //                                                        instead:            |--> reset Signal, so no funccall is detected with the next closingbracket

    }
};

inline void ParseOpeningBracket(ParserContext& PC)
{
    PC.ThrowIfUnexpected(TokenType::OpeningBracket);
 // Compare with openingbracket parsing in the parser
    if( PC.LastToken() == TokenType::Identifier )
    {   //the generic bracket parsing algorithm will take care of the Bracket, nothing more to do for us here
        auto OB = boost::make_shared<OpeningBracketFuncCall>();
        PC.Parse(OB);
        PC.OutputQueue().push_back(OB);
        #ifdef DEBUG
        std::cout << "OpeningBracketFuncCall\n";
        #endif
        //PC.OperatorStack().pop();
    }
    else if( PC.LastToken() == TokenType::Assignment || PC.LastToken() == TokenType::Long || PC.LastToken() == TokenType::Double)
    {
        PC.Parse(boost::make_shared<OpeningBracketNoFuncCall>());
        //PC.OutputQueue().push_back(OB);
        #ifdef DEBUG
        std::cout << "OpeningBracketNoFuncCall\n";
        #endif
    }
    else
    {
        auto OB = boost::make_shared<OpeningBracket>();
        PC.Parse(OB);
        PC.OutputQueue().push_back(OB);
        #ifdef DEBUG
        std::cout << "OpeningBracket\n";
        #endif
    }
}
class OpeningBracketToken : public IToken
{
public:
    OpeningBracketToken():
        IToken("(")
    {

    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(Parsable("(", &ParseOpeningBracket));
//        TC.OutputQueue().push_back(&ParseOpeningBracket);
        return LastCharType::LikeOpeningBracket;
    }
};
class ClosingBracket : public IOperator
{
    struct IsMarker : public boost::static_visitor<bool>
    {
        template <typename T>
        bool operator()(const T&) const
        {
            return false;
        }
        bool operator()(const boost::shared_ptr<IFunction>& Func) const
        {
            return *Func == "__ALSM__";
        }
    };
    struct GetFunc: public boost::static_visitor<boost::shared_ptr<IFunction>>
    {
        EvaluationContext& m_EC;
        GetFunc(EvaluationContext& EC):
            m_EC(EC)
        {

        }
        boost::shared_ptr<IFunction> operator()(long long ) const
        {
            throw Exceptions::TypeException("Expected function; Is long");
        }
        boost::shared_ptr<IFunction> operator()(double )const
        {
            throw Exceptions::TypeException("Expected function; Is double");
        }
        boost::shared_ptr<IFunction> operator()(const boost::shared_ptr<IOperator>& op)const
        {
            throw Exceptions::TypeException("Expected function; Is operator " + op->Representation());
        }
        boost::shared_ptr<IFunction> operator()(const Reference& R)const
        {
            if( R.IsNull() )
                throw Exceptions::NullReferenceException("Calling a nullreference");
            else
                throw Exceptions::TypeException("Expected function; Is table");
        }
        boost::shared_ptr<IFunction> operator()(NullReference)const
        {
            throw Exceptions::NullReferenceException("Calling a nullreference");
        }
        boost::shared_ptr<IFunction> operator()(const boost::shared_ptr<IFunction>& op)const
        {
            return op;
        }
    };
    struct ArgCounter : public boost::static_visitor<unsigned>
    {
        EvaluationContext& m_EC;
        ArgCounter(EvaluationContext& EC):
            m_EC(EC)
        {}
        template <typename T>
        unsigned operator()(const T& ) const
        {
            return 1;
        }
        unsigned operator()(const CountedReference& Ref) const
        {
            auto it = (*Ref).Find("__ARGCOUNT__");
            if( it != (*Ref).KeyEnd() )
                return boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"),it->second);
            else
                return 1;
        }
        unsigned operator()(const boost::shared_ptr<IFunction>& Func) const
        {
            if( *Func == "__ALSM__" )
                return 0;
            else
                return 1;
        }
    };
public:
    ClosingBracket():IOperator("Closing Bracket",")",-100,ArityType::UnaryPostfix,AssociativityType::None)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( EC.Signal(SignalType::FuncCall) )
        {
            EC.DropSignal(); //We're not in a FuncCall anymore if the body of the func gets evaluated
            ResolvedToken Args(Utilities::Resolve(EC,EC.EvalStack.back()));
            EC.EvalStack.pop_back();
            //Check, whether there are arguments. If there's immediatly an argliststartmarker then there are no args
            long long ArgCount = boost::apply_visitor(ArgCounter(EC),Args);
            #ifdef DEBUG
            std::cout << "No. of provided Args for funccall: " << ArgCount << std::endl;
            #endif
            if( ArgCount >= 1 ) //there should still be an ArgListStartMarker
            {
                if( !EC.EvalStack.back().Visit(IsMarker()) )
                {
                    throw std::logic_error("Missing ArgListMarker");
                }
                EC.EvalStack.pop_back();
            }
            const Types::Object& FuncObj(EC.EvalStack.back());
            EC.EvalStack.pop_back();
            ResolvedToken FuncToken(Utilities::Resolve(EC,FuncObj));
            const boost::shared_ptr<IFunction>& Func(boost::apply_visitor(GetFunc(EC),FuncToken));

            Func->SuppliedArguments(Args,ArgCount);
            Func->This(FuncObj.This());
            try
            { //Make sure native exceptions are properly transformed to internal exceptions
                Func->Eval(EC);
            }
            catch( Exceptions::RuntimeException& e )
            {
                Func->This(NullReference());
                EC.Throw(e);
                EC.EndScope();
            }
            Func->This(NullReference()); //Very Important! Reset the this-ref to prevent a crash on shutdown. The MC gets deleted before the functions
                                        //who would still hold a ref to their this-scopes. When  funcs get deleted the and the ref does it's decref, the MC is already gone -> Crash!
        }
        else
            EC.DropSignal();
    }
};
inline void ParseClosingBracket(ParserContext& PC)
{
    //All the parsing has to be done here, because we have to pop the Openingbracket from stack but
    //not on the outputqueue
    PC.ThrowIfUnexpected(TokenType::ClosingBracket, "Unexpected closing bracket");
    if( PC.ExpectedBracket().empty() ) //Closingbracket but no bracket expected? Definitely a bracket mismatch
        throw std::logic_error("No closing bracket expected");
    else if( *PC.ExpectedBracket().top() != ")" )
        throw std::logic_error("Expected '" + PC.ExpectedBracket().top()->Representation() + "'");
    else
        PC.ExpectedBracket().pop();
    if( PC.LastToken() == TokenType::ArgSeperator )
        throw std::logic_error("Expected an expression between ',' and ')'");

    while( !PC.OperatorStack().empty() )
    {
        auto Temp = PC.OperatorStack().top();
        if( *Temp == *PC.OpeningBracket() )
        {
            //it's the opening bracket, we're done
            //Pop the left parenthesis from the stack, but not onto the output queue.
            PC.OperatorStack().pop();
            PC.State().Restore();
            PC.LastToken() = TokenType::ClosingBracket;
            PC.OutputQueue().push_back(boost::make_shared<ClosingBracket>());
            return;
        }
        else
        {
            PC.OutputQueue().push_back(Temp);
            PC.OperatorStack().pop();
        }
    }
    //if it reaches here, there is a bracket mismatch
    throw std::logic_error("bracket mismatch");//If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
}

class ClosingBracketToken : public IToken
{
public:
    ClosingBracketToken():
        IToken(")")
    {

    }
    virtual LastCharType Tokenize(TokenizeContext& TC) const
    {
        TC.OutputQueue().push_back(Parsable(")", &ParseClosingBracket));
        return LastCharType::LikeClosingBracket;
    }
};

class ThrowOp : public IOperator
{
public:
    ThrowOp():
        IOperator("","throw", -15,ArityType::UnaryPrefix,AssociativityType::Right)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        if( EC.Stack.Empty() )
            throw std::logic_error("Missing argument for throw");
        auto Ex = Utilities::GetWithResolve<CountedReference>(EC, EC.Stack.Pop(), "\"throw\" argument has to be an exception type");
        if( !(*Ex).Contains("__EXCEPTION__") )
        {
            EC.Throw(Exceptions::TypeException("\"throw\" argument has to be an exception type"));
            EC.EndScope();
            return;
        }
        EC.Throw(Ex);
        EC.EndScope();
    }
};

}//ns Internal
#endif // OPERATORS_H_INCLUDED
