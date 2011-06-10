#include <iostream>
#include <windows.h>
#include <boost/lexical_cast.hpp>
#include "../include/MariusParser.hpp"
#include "../include/Internal/Operators.hpp"
#include "../include/Internal/Functions.hpp"
#include "../include/Internal/Is.hpp"
#include "../include/Internal/KeywordFunc.hpp"
#include "../include/Internal/KeywordEnd.hpp"
#include "../include/Internal/KeywordTry.hpp"
#include "../include/Internal/KeywordCatch.hpp"
using namespace Internal;
MariusParser::MariusParser():
    m_GlobalScope(m_MC.Save(Internal::Types::Table())),
    m_EC(Stack,m_GlobalScope,m_MC)
{
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

    m_Tokenizer.RegisterToken(new OpeningBracketToken);
    m_Tokenizer.RegisterToken(new ClosingBracketToken);
    m_Parser.RegisterBracketOperator(boost::make_shared<OpeningBracket>(), boost::make_shared<ClosingBracket>());
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
    m_Parser.RegisterBracketOperator(boost::make_shared<IndexOpeningBracket>(), ICB->GetOp());
//*/
    RegisterFunction(boost::make_shared<TypeFunc>());

    Internal::Types::Table MCScope;
    MCScope.Add("GetRefCount",boost::make_shared<GetRefCountFunc>());
    (*m_GlobalScope)["MC"] = m_MC.Save(MCScope);

    Internal::Types::Table T;
    T.Add("Max",boost::make_shared<MaxFunc>());
    T.Add("Sqrt",boost::make_shared<SqrtFunc>());
    T.Add("Ln",Internal::BindFunc("Ln","Ln",static_cast<double(*)(double)>(&std::log)));
    T.Add("Pi",3.141592653589793238462643);
    T.Add("E",2.718281828459045235360287);
    CountedReference Ref(m_MC.Save(T));
    (*m_GlobalScope)["Math"] = Ref;
    RegisterFunction(boost::make_shared<CreateNullFunc>());
    RegisterFunction(boost::make_shared<CreateTableFunc>());
    RegisterFunction(boost::make_shared<PrintFunc>());
    (*m_Evaluator.m_GlobalScope)["__CATCH__"] = boost::make_shared<GlobalExceptionHandleFunc>();
}

MariusParser::~MariusParser()
{
    //dtor
}

::Types::Object MariusParser::Evaluate(const std::string& Input)
{
    m_Tokenizer.Clear();
    m_Parser.Clear();
    m_EC.Stack.Clear();

    LARGE_INTEGER start_ticks, ende_ticks, frequenz;

    unsigned long tick_diff = 0;
    start_ticks.QuadPart = 0;
    ende_ticks.QuadPart  = 0;
    QueryPerformanceFrequency(&frequenz);

#ifdef DEBUG
    std::cout << "--- Tokenizing ---" << std::endl;
#endif
    QueryPerformanceCounter(&start_ticks);
    std::deque<UnparsedToken> UTs = m_Tokenizer.Tokenize(Input);
    QueryPerformanceCounter(&ende_ticks);

    tick_diff = ende_ticks.QuadPart - start_ticks.QuadPart;
    std::string Took("\nInput: " + Input + "\nTokenizing took:\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) +
                     " ticks and " + boost::lexical_cast<std::string>((double)tick_diff / frequenz.QuadPart) + " ms\n");
#ifdef DEBUG
    foreach(UnparsedToken& Tok, UTs)
    {
        std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),Tok) << " ";
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

    Took += "Parsing took:\t\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) + " ticks\n";
    tick_diff += ende_ticks.QuadPart - start_ticks.QuadPart;

    auto Q = m_Parser.GetOutput();
#ifdef DEBUG
    std::cout << "Output.size = " << Q.size() << std::endl;
#endif

    start_ticks.QuadPart = 0;
    ende_ticks.QuadPart  = 0;
#ifdef DEBUG
    std::cout << "--- Evaluating ---" << std::endl;
#endif
    QueryPerformanceCounter(&start_ticks);
    /*while( !Q.empty() )
    {
        Token t = Q.front();
#ifdef DEBUG
        std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),t) << " ";
#endif
        boost::apply_visitor(m_Evaluator,t);
        Q.pop();
    }*/
    m_EC.NewScope(m_EC.GlobalScope,&Q);
    m_EC.EvalScope();
    m_EC.EndScope();
    QueryPerformanceCounter(&ende_ticks);

    Took += "Evaluation took:\t" + boost::lexical_cast<std::string>(ende_ticks.QuadPart - start_ticks.QuadPart) + " ticks\n";
    tick_diff += ende_ticks.QuadPart - start_ticks.QuadPart;

    Took += "All took:\t\t" + boost::lexical_cast<std::string>(tick_diff) + " ticks"
            " and " + boost::lexical_cast<std::string>((double)tick_diff / frequenz.QuadPart) + " ms\n";
    std::cout << Took << std::endl;

    if( !m_EC.Stack.Empty() )
    {
        std::cout << "\nResult: " << Utilities::PrintValue(m_EC,m_EC.Stack.Top()) << std::endl;
        return ::Types::Object(m_EC,Utilities::Resolve(m_EC,m_EC.Stack.Top()));
    }
    else
    {
        return ::Types::Object(m_EC,NullReference());
    }
}
