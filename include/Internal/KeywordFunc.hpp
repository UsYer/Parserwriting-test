#ifndef KEYWORDFUNCTION_HPP_INCLUDED
#define KEYWORDFUNCTION_HPP_INCLUDED
namespace Internal
{
namespace Keyword
{
namespace Detail
{
class RuntimeFunc: public IFunction
{
    class FuncEvaluator : public boost::static_visitor<>
    {
        Types::Stack m_Stack;
        EvaluationContext m_EC;

        public:
        FuncEvaluator(MemoryController& MC, Types::Scope& LocalScope):
            m_EC(m_Stack, LocalScope, MC)
        {}
        void operator()(long long Val)
        {
            m_Stack.push_back(Types::Object(Val));
        }
        void operator()(double val)
        {
            m_Stack.push_back(Types::Object(val));
        }
        void operator()(const boost::shared_ptr<IEvaluable>& Evaluable)
        {
            Evaluable->Eval(m_EC);
        }
        void operator()(const std::string& s)
        {
            m_Stack.push_back(Types::Object(s));
        }
    };
    public:
    std::deque<Token> m_FuncInstructions;
    RuntimeFunc(const std::string& Name, const std::string& Representation, const std::vector<std::string>& Args, unsigned ReturnCount):
        IFunction(Name, Representation, Args.size(), ReturnCount)
    {
        foreach( const std::string& Arg, Args )
        {
            m_LocalScope[Arg];
        }
    }
    virtual void Eval(EvaluationContext& EC)
    {
        CountedReference LocalScopeRef(EC.MC.Save(m_LocalScope));
        (*LocalScopeRef)["__PARENT__"] = EC.Scope;
        (*LocalScopeRef)["This"] = m_This;
        FuncEvaluator FE(EC.MC,LocalScopeRef);
        foreach(Token& T,m_FuncInstructions)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(FE,T);
        }
    }
};
struct ParseFuncBeginning : public boost::static_visitor<std::string>
{
    const ParserContext& m_Context;
    ParseFuncBeginning(const ParserContext& Context):
        m_Context(Context)
    {

    }
    std::string operator()(long long) const
    {
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(double) const
    {
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(const std::string& Identifier ) const
    {
        m_Context.InputQueue().pop_front();
        return Identifier;
    }
    std::string operator()(const boost::shared_ptr<IOperator>& Op ) const
    {
        if( *m_Context.OpeningBracket() == *Op )
        {
            return "";
        }
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
    std::string operator()(const Parsable& P) const
    {
        if( P.Representation == *m_Context.OpeningBracket() )
        {
            return "";
        }
        throw std::logic_error("Expected identifier or opening bracket after keyword \"func\"");
    }
};
template <typename T>
struct Swallow : boost::static_visitor<bool>
{
    std::deque<UnparsedToken>& m_Input;
    Swallow(std::deque<UnparsedToken>& Input):
        m_Input(Input)
    {

    }
    bool operator()(const T&) const
    {
        m_Input.pop_front();
        return true; //Swallowed a token
    }
    template <typename U>
    bool operator()(const U&) const
    {
        return false; //Did not swallow a token
    }
};

struct SwallowOpeningBracket : public boost::static_visitor<>
{
//    std::deque<UnparsedToken>& m_Input;
//    const boost::shared_ptr<IOperator>& m_OpeningBracket;
//    SwallowOpeningBracket(std::deque<UnparsedToken>& Input, const boost::shared_ptr<IOperator>& OpeningBracket):
//        m_Input(Input),
//        m_OpeningBracket(OpeningBracket)
    ParserContext& m_PC;
    SwallowOpeningBracket(ParserContext& PC):
        m_PC(PC)
    {
    }
    void operator()(const boost::shared_ptr<IOperator>& Op) const
    {
        if( *m_PC.OpeningBracket() == *Op )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error("Expected opening bracket");
    }
    void operator()(const Parsable& P) const
    {
        if( *m_PC.OpeningBracket() == P.Representation )
        {
            m_PC.InputQueue().pop_front();
            return;
        }
        throw std::logic_error("Expected opening bracket");
    }
    template <typename T>
    void operator()(const T&) const
    {
        throw std::logic_error("Expected opening bracket");
    }
};
///See enum struct TokenType in Parser.hpp
const char* Expected(TokenType Type)
{
    switch(Type)
    {
        case TokenType::OpUnaryPrefix:
        case TokenType::OpUnaryPostfix:
        case TokenType::OpBinary:
            return "operator";
        break;
        case TokenType::Identifier:
            return "identifier";
        break;
        case TokenType::ArgSeperator:
            return "argument seperator";
        break;
        case TokenType::None:
            return "none";
        break;
    }
    return "unknown";
}
struct ParseArgumentList : public boost::static_visitor<void>
{
    ParseArgumentList(const ParserContext& ParserContext):
        m_Expected(TokenType::None),
        m_Finished(false),
        m_ParserContext(ParserContext)
    {

    }
    template <typename T>
    void operator()(T) const
    {
        throw std::logic_error("Number literals are not allowed in the argument list");
    }
    void operator()(const boost::shared_ptr<IOperator>& Op)
    {
        CheckForOperator(Op->Representation());
    }
    void operator()(const Parsable& P)
    {
        CheckForOperator(P.Representation);
    }
    void operator()(const std::string& Identifier)
    {
        if( m_Expected != TokenType::Identifier && m_Expected != TokenType::None ) //TokenType::None means we just started so we can't throw then
                throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + " before Identifier \"" + Identifier + "\"");
        //check for duplicate argumentnames
        auto it = std::find(m_Args.begin(),m_Args.end(),Identifier);
        if( it != m_Args.end() )
            throw std::logic_error("Duplicate argument \"" + Identifier + "\" in funtion definition");

        m_Args.push_back(Identifier);
        m_Expected = TokenType::ArgSeperator;
    }

    TokenType m_Expected;
    std::vector<std::string> m_Args;
    bool m_Finished;
    private:
    const ParserContext& m_ParserContext;
    void CheckForOperator(const std::string& Rep)
    {
        if( *m_ParserContext.ClosingBracket() == Rep )
        {
            if( m_Expected == TokenType::Identifier)
                throw std::logic_error(std::string("Expected Identifier; got closing bracket"));
            //the arg list is finished with the occurences of a closing bracket
            m_Expected = TokenType::None;
            m_Finished = true;
            return;
        }
        else if( *m_ParserContext.ArgumentSeperator() == Rep )
        {
            if( m_Expected != TokenType::ArgSeperator )
                throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + "; got argument seperator");

            //Argsep is expected so we swallow it and expect a new identifier
            m_Expected = TokenType::Identifier;
        }
        else
        {
            throw std::logic_error(std::string("Expected ") + Expected(m_Expected) + "; got " + Rep);
        }
    }
};

class FuncRegistrar : public IEvaluable
{
    boost::shared_ptr<IFunction> m_Func;
    public:
    FuncRegistrar(const boost::shared_ptr<IFunction>& Func):
        IEvaluable("","FuncRegistrar"),
        m_Func(Func)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( EC.Signal(SignalType::FuncCall) )
        {
            EC.DropSignal();
            throw std::logic_error("Named function definition is not allowed in a function call");
        }
        (*EC.Scope)[m_Func->Representation()] = m_Func;
    }
};
/**
Holds an anonymous function after it's been created.

Necessary, because the function would otherwise be evaluated directly when encoutered.
Instead, the FuncHolder gets evaluated which then pushes the real func on the stack.
Now something like this is possible: func = function() ... end
*/
class AnonFuncHolder : public IEvaluable
{
    boost::shared_ptr<IFunction> m_Func;
    public:
    AnonFuncHolder(const boost::shared_ptr<IFunction>& Func):
        IEvaluable("","AnonFuncHolder"),
        m_Func(Func)
    {}
    void Eval(EvaluationContext& EC)
    {
        EC.EvalStack.push_back(Types::Object(m_Func));
    }
};
} //namespace Detail

/* TODO (Marius#9#): Implement returning values from functions
example: function test([args...]) return [...] end
*/
///Parses a function defined at runtime
void Function(ParserContext& Context)
{
    if( Context.InputQueue().empty() )
        throw std::logic_error("Missing input after \"func\"");
    //Check the identifier of the funtion:
    std::string Identifier = boost::apply_visitor(Detail::ParseFuncBeginning(Context),Context.InputQueue().front());
    if( Context.State() == ParserState::FunctionCall && !Identifier.empty() )
        throw std::logic_error("Named function definition is not allowed in a function call");
    #ifdef DEBUG
    std::cout << "FuncName: " << Identifier << "\n";
    #endif
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected opening bracket");
    //Parse the argumentlist of the function
    //But first swallow the opening bracket
    boost::apply_visitor(Detail::SwallowOpeningBracket(Context),Context.InputQueue().front());
    if( Context.InputQueue().empty() )
        throw std::logic_error("Expected input after opening bracket");
    //Now we start parsing the arguments
    Detail::ParseArgumentList ArgListParser(Context);
    while( !Context.InputQueue().empty() && !ArgListParser.m_Finished)
    {
        boost::apply_visitor(ArgListParser,Context.InputQueue().front());
        Context.InputQueue().pop_front();
    }
    if( ArgListParser.m_Expected != TokenType::None )
        throw std::logic_error("Missung input in argument list; expected closing bracket\n");

    #ifdef DEBUG
        std::cout << "No. of args: " << ArgListParser.m_Args.size() << "\n";
        std::cout << "Args: ";
        for( size_t i = 0; i < ArgListParser.m_Args.size(); i++ )
        {
            std::cout << ArgListParser.m_Args[i] + ", ";
        }
    #endif
    boost::shared_ptr<Detail::RuntimeFunc> RTFunc(new Detail::RuntimeFunc("",Identifier,ArgListParser.m_Args,0));
    //Push the func !before! setting up the new scope, because it would be otherwise registered in the new scope, which is the func itself. Weird :D
    if( Identifier.empty() )
        Context.OutputQueue().push_back(boost::shared_ptr<IEvaluable>(new Detail::AnonFuncHolder(RTFunc)));
    else
        Context.OutputQueue().push_back(boost::shared_ptr<IEvaluable>(new Detail::FuncRegistrar(RTFunc)));
    Context.LastToken() = TokenType::KeywordWithValue;
    Context.SetUpNewScope(&RTFunc->m_FuncInstructions);
}
}//namespace Keyword
}//namespace Internal


#endif // KEYWORDFUNCTION_HPP_INCLUDED
