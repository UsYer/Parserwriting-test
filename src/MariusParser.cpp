#include <iostream>
#include <windows.h>
#include <boost/lexical_cast.hpp>
#include "../include/MariusParser.hpp"
#include "../include/Internal/Parser.hpp"
#include "../include/Internal/Tokenizer.hpp"
#include "../include/Internal/Utilities.hpp"
#include "../include/Internal/Operators.hpp"
#include "../include/Internal/Functions.hpp"
#include "../include/Internal/Is.hpp"
#include "../include/Internal/KeywordFunc.hpp"
#include "../include/Internal/KeywordEnd.hpp"
#include "../include/Internal/KeywordTry.hpp"
#include "../include/Internal/KeywordCatch.hpp"
#include "../include/Internal/KeywordNull.hpp"
#include "../include/Internal/KeywordReturn.hpp"

struct MariusParser::Impl
{
    Impl():
        m_GlobalScope(m_MC.Save(Internal::Types::Table())),
        m_EC(Stack,m_GlobalScope,m_MC)
    {
        using namespace Internal;
        //Set up builtin operators and functions
        m_Tokenizer.RegisterToken(new ParsedOperatorToken(Internal::AssignmentOp(),&ParseAssignment)); //Needs to set LastToken and ParserState while parsing
        m_Tokenizer.RegisterToken(new ParsedOperatorToken(TableOp(),&ParseTableOp));
        m_Tokenizer.RegisterToken(new OperatorToken<PlusOp>);
        m_Tokenizer.RegisterToken(new MinusToken); //Minus needs some extra work while tokenizing
        m_Tokenizer.RegisterToken(new OperatorToken<MultiOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<DivOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<TestOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<ExponentOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<ModuloOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<AccesOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<LessOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<GreaterOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<NotOp>);
        m_Tokenizer.RegisterToken(new OperatorToken<ThrowOp>);

        m_Tokenizer.RegisterToken(new KeywordToken("func",&Keyword::Function));
        m_Tokenizer.RegisterToken(new KeywordToken("end",&Keyword::End));
        m_Tokenizer.RegisterToken(new KeywordToken("try",&Keyword::Try));
        m_Tokenizer.RegisterToken(new KeywordToken("catch",&Keyword::Catch));
        m_Tokenizer.RegisterToken(new KeywordToken("null",&Keyword::Null));
        m_Tokenizer.RegisterToken(new KeywordToken("return",&Keyword::Return));

        m_Tokenizer.RegisterToken(new OpeningBracketToken);
        m_Tokenizer.RegisterToken(new ClosingBracketToken);
        m_Parser.RegisterBracketOperator(std::make_shared<OpeningBracket>(), std::make_shared<ClosingBracket>());
    /*
        auto IOB = new GenericOpeningBracketToken<IndexOpeningBracket>;
        auto ICB = new GenericClosingBracketToken<IndexClosingBracket>;
        m_Tokenizer.RegisterToken(IOB);
        m_Tokenizer.RegisterToken(ICB);
        m_Parser.RegisterBracketOperator(IOB->GetOp(), ICB->GetOp());
    //*/
    //*
        auto ICB = new GenericClosingBracketToken<IndexClosingBracket>;
        m_Tokenizer.RegisterToken(new IndexOpeningBracketToken);
        m_Tokenizer.RegisterToken(ICB);
        m_Parser.RegisterBracketOperator(std::make_shared<IndexOpeningBracket>(), ICB->GetOp());
    //*/
        RegisterFunction(std::make_shared<TypeFunc>());

        Internal::Types::Table MCScope;
        MCScope.Add("GetRefCount",std::make_shared<GetRefCountFunc>());
        MCScope.Add("ItemCount",std::make_shared<GetMCSizeFunc>());
        (*m_GlobalScope)["MC"] = m_MC.Save(MCScope);

        Internal::Types::Table T;
        T.Add("Max",std::make_shared<MaxFunc>());
        T.Add("Sqrt",std::make_shared<SqrtFunc>());
        T.Add("Ln",Internal::BindFunc("Ln","Ln",static_cast<double(*)(double)>(&std::log)));
        T.Add("Pi",3.141592653589793238462643);
        T.Add("E",2.718281828459045235360287);
        CountedReference Ref(m_MC.Save(T));
        (*m_GlobalScope)["Math"] = Ref;
        RegisterFunction(std::make_shared<CreateNullFunc>());
        RegisterFunction(std::make_shared<CreateTableFunc>());
        RegisterFunction(std::make_shared<PrintFunc>());
        (*m_GlobalScope)["__CATCH__"] = std::make_shared<GlobalExceptionHandleFunc>();
        RegisterFunction(std::make_shared<CreateExceptionFunc>("RuntimeException",1));
        RegisterFunction(std::make_shared<CreateExceptionFunc>("TypeException",2));
        RegisterFunction(std::make_shared<CreateExceptionFunc>("NameException",3));
        RegisterFunction(std::make_shared<CreateExceptionFunc>("NullReferenceException",4));
    }
    void RegisterToken(IToken* Tok)
    {
        m_Tokenizer.RegisterToken(Tok);
    }
    void RegisterFunction(const Internal::Types::Function& p)
    {
        (*m_GlobalScope)[p->Representation()] = p;
    }
    void RegisterFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount,const Internal::CallbackFunction& Func)
    {
        (*m_GlobalScope)[Representation] = std::make_shared<Internal::BindFunction>(Name, Representation, ArgCount, ReturnCount,Func);
    }
    ::Types::Function GetFunction(const std::string& Identifier) const
    {
        auto it = (*m_GlobalScope).Find(Identifier);
        if( it == (*m_GlobalScope).KeyEnd() )
        {
            auto Func = boost::apply_visitor(Internal::Utilities::Get<const Internal::Types::Function&>(),it->second);
            //Used const_casts here because there were otherwise some strange compiler errors
            return ::Types::Function(Func, const_cast<Internal::Types::Scope&>(m_GlobalScope), const_cast<Internal::MemoryController&>(m_MC));
        }
        throw std::logic_error("Function \"" + Identifier + "\" unknown");
    }
    ::Types::Object GlobalScope()
    {
        return ::Types::Object(m_EC,m_GlobalScope);
    }
    ::Types::Object Evaluate(const std::string& Input, BenchData* BD = nullptr)
    {
        //Even though all these components will be cleared as needed in the process they are also cleared here.
        //This is necessary because an exception could be thrown anywhere while parsing and the later clear commands would not be issued
        m_Tokenizer.Clear();
        m_Parser.Clear();
        m_EC.Stack.Clear();
        while( m_EC.EndScope() );
            //No-op. resetting Evalscope to global scope
        LARGE_INTEGER start_ticks, ende_ticks, frequenz;

        unsigned long tick_sum = 0;
        start_ticks.QuadPart = 0;
        ende_ticks.QuadPart  = 0;
        QueryPerformanceFrequency(&frequenz);

    #ifdef DEBUG
        std::cout << "--- Tokenizing ---" << std::endl;
    #endif
        QueryPerformanceCounter(&start_ticks);
        std::deque<Internal::UnparsedToken> UTs = m_Tokenizer.Tokenize(Input);
        m_Tokenizer.Clear();
        QueryPerformanceCounter(&ende_ticks);

        auto tickTokenizing = tick_sum = ende_ticks.QuadPart - start_ticks.QuadPart;
//        "Tokenizing took:\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) +
//                         " ticks and " + boost::lexical_cast<std::string>((double)tick_sum / frequenz.QuadPart) + " ms\n");
    #ifdef DEBUG
        for(Internal::UnparsedToken& Tok : UTs)
        {
            std::cout << boost::apply_visitor(Internal::Utilities::PrintValueNoResolve(),Tok) << " ";
        }
        std::cout << std::endl;
    #endif
        start_ticks.QuadPart = 0;
        ende_ticks.QuadPart  = 0;
    #ifdef DEBUG
        std::cout << "--- Parsing ---" << std::endl;
    #endif
        QueryPerformanceCounter(&start_ticks);
        m_Parser.Parse(UTs);
        QueryPerformanceCounter(&ende_ticks);

//        Took += "Parsing took:\t\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) + " ticks\n";
        auto tickParsing = ende_ticks.QuadPart - start_ticks.QuadPart;
        tick_sum += tickParsing;

        auto Q = m_Parser.GetOutput();
        m_Parser.Clear();
    #ifdef DEBUG
        std::cout << "Output.size = " << Q.size() << std::endl;
    #endif

        start_ticks.QuadPart = 0;
        ende_ticks.QuadPart  = 0;
    #ifdef DEBUG
        std::cout << "--- Evaluating ---" << std::endl;
    #endif
        QueryPerformanceCounter(&start_ticks);
        m_EC.SetGlobalScopeInstructions(&Q);
        m_EC.EvalScope();
        QueryPerformanceCounter(&ende_ticks);

//        Took += "Evaluation took:\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) + " ticks\n";
        auto tickEval = ende_ticks.QuadPart - start_ticks.QuadPart;
        tick_sum += tickEval;

//        Took += "All took:\t\t" + boost::lexical_cast<std::string>(tick_sum) + " ticks"
//                " and " + boost::lexical_cast<std::string>((double)tick_sum / frequenz.QuadPart) + " ms\n";

        if( BD )
        {
            BD->TicksTokenize = tickTokenizing;
            BD->TicksParse = tickParsing;
            BD->TicksEval = tickEval;
            BD->TicksSum = tick_sum;
            BD->Frequency = frequenz.QuadPart;
        }
        if( !m_EC.Stack.Empty() )
        {
            std::cout << "\nResult: " << Internal::Utilities::PrintValueNoThrow(m_EC,m_EC.Stack.Top()) << std::endl;
            ::Types::Object Result(m_EC,Internal::Utilities::Resolve(m_EC,m_EC.Stack.Pop()));
            m_EC.Stack.Clear();
            while( m_EC.EndScope() );
            //No-op. resetting Evalscope to global scope
            return Result;
        }
        else
        {
            while( m_EC.EndScope() );
            //No-op. resetting Evalscope to global scope
            return ::Types::Object(m_EC,Internal::NullReference());
        }
    }
    Internal::Tokenizer m_Tokenizer;
    Internal::Parser m_Parser;
    Internal::MemoryController m_MC;
    Internal::Types::Scope m_GlobalScope;
    Internal::Types::Stack Stack;
    Internal::EvaluationContext m_EC;
};

MariusParser::MariusParser():
    m_Impl(new Impl)
{

}

MariusParser::~MariusParser()
{
    delete m_Impl;
}
void MariusParser::RegisterToken(IToken* Tok)
{
    m_Impl->RegisterToken(Tok);
}
void MariusParser::RegisterFunction(const Internal::Types::Function& p)
{
    m_Impl->RegisterFunction(p);
}
void MariusParser::RegisterFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount,const Internal::CallbackFunction& Func)
{
    m_Impl->RegisterFunction(Name, Representation, ArgCount, ReturnCount,Func);
}
::Types::Function MariusParser::GetFunction(const std::string& Identifier) const
{
    return m_Impl->GetFunction(Identifier);
}
::Types::Object MariusParser::GlobalScope()
{
    return m_Impl->GlobalScope();
}
::Types::Object MariusParser::Evaluate(const std::string& Input, BenchData* BD)
{
    return m_Impl->Evaluate(Input, BD);
}
