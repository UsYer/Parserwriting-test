#include <algorithm>
#ifdef DEBUG
#include <iostream>
#endif
#include "../MemoryManagment/include/MemoryController.hpp"
#include "../MemoryManagment/include/Reference.hpp"
#include "../include/Internal/Parser.hpp"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#define foreach BOOST_FOREACH
#include "../include/Internal/FuncCaller.hpp"
#include "../include/Internal/Get.hpp"
#include "../include/Internal/Is.hpp"
#include "../include/Internal/Operators.hpp"
#include "../include/Exceptions.hpp"
using namespace Internal;

struct FindOpeningBracket
{
    const IOperator& BracketToCompare;
    FindOpeningBracket(const IOperator& Bracket):
        BracketToCompare(Bracket)
    {}
    bool operator()(const std::pair<boost::shared_ptr<IOperator>,boost::shared_ptr<IOperator>>& BracketPair) const
    {
        return BracketToCompare == *BracketPair.first;
    }
};
struct FindClosingBracket
{
    const IOperator& BracketToCompare;
    FindClosingBracket(const IOperator& Bracket):
        BracketToCompare(Bracket)
    {}
    bool operator()(const std::pair<boost::shared_ptr<IOperator>,boost::shared_ptr<IOperator>>& BracketPair) const
    {
        return BracketToCompare == *BracketPair.second;
    }
};


Parser::Parser():
        m_OpeningBracket(new class OpeningBracket),
        m_ClosingBracket(new class ClosingBracket),
        m_ArgumentSeperator(new TableOp),
        m_Context(*this)
{
    m_Context.SetUpNewScope();
}

struct Parser::Visitor : public boost::static_visitor<>
{

    Visitor(ParserContext& p):m_ParserContext(p)
    {
    }
    void operator()(long long val)
    {
        ThrowIfUnexpected(TokenType::Long, "Unexpected number literal");
        m_ParserContext.OutputQueue().push_back(val);
        m_ParserContext.LastToken() = TokenType::Long;
    }
    void operator()(double val)
    {
        ThrowIfUnexpected(TokenType::Double, "Unexpected number literal");
        m_ParserContext.OutputQueue().push_back(val);
        m_ParserContext.LastToken() = TokenType::Double;
    }
    void operator()(const boost::shared_ptr<IOperator>& op)
    {
        auto it = std::find_if(m_ParserContext.BracketOperators().begin(), m_ParserContext.BracketOperators().end(), FindOpeningBracket(*op));
        if( it != m_ParserContext.BracketOperators().end() )
        {
            ThrowIfUnexpected(TokenType::OpeningBracket, "Unexpected opening bracket");
            //check if there is an operator missing. Something like that: 5(5+5), which should read 5*(5+5) for example
            if( m_ParserContext.LastToken() == TokenType::Long || m_ParserContext.LastToken() == TokenType::Double )
            {
                throw std::logic_error("An operator is missing before '" + op->Representation() + "'\n");
            }
            m_ParserContext.ExpectedBracket().push(it->second);
            PopHigherPrecedenceOperators(op);
            m_ParserContext.OperatorStack().push(op);
            m_ParserContext.LastToken() = TokenType::OpeningBracket;
            return;
        }
        // TODO (Marius#6#): Extract the closingbracket treatment in a Closingbracket parsable
        else if( *op == *m_ParserContext.ClosingBracket() )
        {
            ThrowIfUnexpected(TokenType::ClosingBracket, "Unexpected closing bracket");
            if( m_ParserContext.LastToken() == TokenType::ArgSeperator )
                throw std::logic_error("Expected an expression between ',' and '" + op->Representation() + "'");
            if( m_ParserContext.ExpectedBracket().empty() ) //Closingbracket but no bracket expected? Definitely a bracket mismatch
                throw std::logic_error("No closing bracket expected");
            else if( *m_ParserContext.ExpectedBracket().top() == *op )
                m_ParserContext.ExpectedBracket().pop();
            else
                throw std::logic_error("Expected '" + m_ParserContext.ExpectedBracket().top()->Representation() + "'");
            while( !m_ParserContext.OperatorStack().empty() )
            {
                // TODO (Marius#6#): Check whether this if is really necessary. i think not and it's kinda buggy because it doesn't take the argcount into Consideration
                if( boost::apply_visitor(Is<FuncCallerTag>(), m_ParserContext.OperatorStack().top()) )
                {
                    //m_ParserContext.OutputQueue().push_back(m_ParserContext.OperatorStack().top());
                    //m_ParserContext.OutputQueue().push_back(m_ParserContext.LastFuncIdentifiers->top());
                    //m_ParserContext.LastFuncIdentifiers->pop();
                    //m_ParserContext.OutputQueue().push_back(boost::shared_ptr<IEvaluable>(new FuncCaller));
                    //m_ParserContext.OperatorStack().pop();
                }
                else//Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
                {
                    //it's an operator
                    auto Temp = boost::get<const boost::shared_ptr<IOperator>&>(m_ParserContext.OperatorStack().top());
                    if( *Temp == *m_ParserContext.OpeningBracket() )
                    {
                        //it's the opening bracket, we're done
                        //Pop the left parenthesis from the stack, but not onto the output queue.
//                        m_ParserContext.OutputQueue().push_back(Temp);
                        m_ParserContext.OperatorStack().pop();
                        //Check if this bracketsequence belongs to a function call:
                        if( !m_ParserContext.OperatorStack().empty() && boost::apply_visitor(Is<FuncCallerTag>(), m_ParserContext.OperatorStack().top()) )
                        {
                            m_ParserContext.OperatorStack().pop();
                            FinishFunctionCall();
                        }
                        else
                        { //The openingbracket doesn't belong to a funccall, but we still have to restore the former state, because it could have been altered when the openingbracket was parsed
                            m_ParserContext.State().Restore();
                        }
                        m_ParserContext.LastToken() = TokenType::ClosingBracket;
//#ifdef DEBUG
//                        std::cout << "ArgCounter: " << m_ParserContext.ArgCounterStack().top() << std::endl;
//#endif
                        m_ParserContext.OutputQueue().push_back(op);
                        return;
                    }
//                    else if( *Temp == *m_ParserContext.ArgumentSeperator() )
//                    {
//                        if( m_ParserContext.State() != ParserState::FunctionCall ) //The ','-op is a normal operator outside a functioncall
//                            m_ParserContext.OutputQueue().push_back(Temp);
//                        m_ParserContext.OperatorStack().pop();
//                    }
                    else
                    {
                        m_ParserContext.OutputQueue().push_back(Temp);
                        m_ParserContext.OperatorStack().pop();
                    }
                }
            }
            //if it reaches here, there is a bracket mismatch
            throw std::logic_error("bracket mismatch");//If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        }
        it = std::find_if(m_ParserContext.BracketOperators().begin(), m_ParserContext.BracketOperators().end(), FindClosingBracket(*op));
        if( it != m_ParserContext.BracketOperators().end() )
        {
            ThrowIfUnexpected(TokenType::ClosingBracket, "Unexpected closing bracket");
            if( m_ParserContext.ExpectedBracket().empty() ) //Closingbracket but no bracket expected? Definitely a bracket mismatch
                throw std::logic_error("No closing bracket expected");
            else if( *m_ParserContext.ExpectedBracket().top() == *it->second )
                m_ParserContext.ExpectedBracket().pop();
            else
                throw std::logic_error("Expected '" + m_ParserContext.ExpectedBracket().top()->Representation() + "'");

            while( !m_ParserContext.OperatorStack().empty() )
            {
                auto Temp = boost::get<const boost::shared_ptr<IOperator>&>(m_ParserContext.OperatorStack().top());
                m_ParserContext.OutputQueue().push_back(Temp);
                if( *Temp == *(it->first) )
                {
                    m_ParserContext.OutputQueue().push_back(it->second); //push the closing bracket on the outqueue
                    m_ParserContext.OperatorStack().pop();
                    m_ParserContext.LastToken() = TokenType::ClosingBracket;
                    return;
                }
                m_ParserContext.OperatorStack().pop();
            }
            //if it reaches here, there is a bracket mismatch
            throw std::logic_error("bracket mismatch");//If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        }
        /*
        else if( *op == *m_ParserContext.ArgumentSeperator() )
        {
            if( m_ParserContext.State() == ParserState::FunctionCall )
            {
                if( !m_ParserContext.ArgExistsStack().top() )
                    throw std::logic_error("Expected an expression before ,");
                //All operators between the beginning of the funccall and the argsep have to be pushed on the outqueue because they would be misplaced otherwise
                //It's okay to check for an opening bracket, because there will only be the funccall opening bracket on the opstack as long as there is no bracket mismatch
                //Math.Max((5+5)^(2+1),5)
                //         |<->| |<->|
                //        |<--------->|
                //      All fine
                //Math.Max((5+5)^(2+1,5)
                //         |<->| |<--
                //               |<->|
                //      We're in trouble
                while( !m_ParserContext.OperatorStack().empty() && *boost::get<const boost::shared_ptr<IOperator>&>(m_ParserContext.OperatorStack().top()) != *m_ParserContext.OpeningBracket() )
                {
                    m_ParserContext.OutputQueue().push_back(boost::get<const boost::shared_ptr<IOperator>&>(m_ParserContext.OperatorStack().top()));
                    m_ParserContext.OperatorStack().pop();
                }
                m_ParserContext.ArgExistsStack().top() = false;
                m_ParserContext.LastToken() = TokenType::ArgSeperator;
                //The ArgSep-Op is not pushed on the OpStack because it's not necassary in a funccall, as far as I can see
                //m_ParserContext.OperatorStack().push(op);
            }
            else
            { //if we treat the argsep as a normal operator it needs to be parsed accordingly
                ParseBinaryOperator(op);
            }
            return;
        }
        */
#ifdef DEBUG
        std::cout << "Push operator ";
#endif
        if( op->Arity() == IOperator::ArityType::UnaryPostfix )
        {
#ifdef DEBUG
            std::cout << "UnaryPostfix" << std::endl;
#endif
            ThrowIfUnexpected(TokenType::OpUnaryPostfix, "Unexpected operator '" + op->Representation() + "'");
            if( m_ParserContext.LastToken() == TokenType::None )
                throw std::logic_error("missing input before operator '" + op->Representation() + "'");

            m_ParserContext.OutputQueue().push_back(op);
            m_ParserContext.LastToken() = TokenType::OpUnaryPostfix;
        }
        else if( op->Arity() == IOperator::ArityType::UnaryPrefix )
        {
#ifdef DEBUG
            std::cout << "UnaryPrefix" << std::endl;
#endif
            ThrowIfUnexpected(TokenType::OpUnaryPrefix, "Unexpected operator '" + op->Representation() + "'");
            m_ParserContext.OperatorStack().push(op);
            m_ParserContext.LastToken() = TokenType::OpUnaryPrefix;
        }
        else    //Binary Operator
        {
#ifdef DEBUG
            std::cout << "Binary" << std::endl;
#endif
            ThrowIfUnexpected(TokenType::OpBinary, "Unexpected operator '" + op->Representation() + "'");
            ParseBinaryOperator(op);
        }
    }
    void operator()(const std::string& s)
    {
        m_ParserContext.OutputQueue().push_back(s);
        m_ParserContext.LastToken() = TokenType::Identifier;
    }
    void operator()(const Parsable& P)
    {
        P.Parse(m_ParserContext);
    }
private:
    ParserContext& m_ParserContext;
    //-----------------------------------------------------------
    enum Behaviour
    {
        EraseFromOutput,
        EraseFromOpStack,
        None
    };
    bool StartFunctionCall(const std::string& Identifier, Behaviour Behav = Behaviour::None)
    {
        (*this)(boost::shared_ptr<IOperator>(new FuncCaller));
        m_ParserContext.OperatorStack().push(FuncCallerTag());
        //new functioncall, so we set up the new states
        m_ParserContext.State() = ParserState::FunctionCall;
        m_ParserContext.ArgCounterStack().push(0);
        m_ParserContext.ArgExistsStack().push(false);
        return true;
    }
    void FinishFunctionCall(/*const boost::shared_ptr<IFunction>& Func,*/ Behaviour Behav = Behaviour::None)
    {
        //We push the number of arguments on the stack so the function can get it from there
        m_ParserContext.OutputQueue().push_back(static_cast<long long>(m_ParserContext.ArgCounterStack().top()));

        if( Behav == Behaviour::EraseFromOpStack )
            m_ParserContext.OperatorStack().pop(); //Remove Function from the OpStack

        //The function is on the outqueue, we're no longer in a function call, hence we reset the states
        m_ParserContext.State().Restore();
        if( !m_ParserContext.ArgCounterStack().empty() )
            m_ParserContext.ArgCounterStack().pop();
        if( !m_ParserContext.ArgExistsStack().empty() )
            m_ParserContext.ArgExistsStack().pop();
    }
    void PopHigherPrecedenceOperators(const boost::shared_ptr<IOperator>& Op)
    {
        #ifdef DEBUG
                    std::cout << "PopHigherPrecedence(" << Op->Representation() << "):\n";
        #endif
        while( !m_ParserContext.OperatorStack().empty() ) //While there are items on the stack
        {
            auto StackTop = m_ParserContext.OperatorStack().top();
            if( boost::apply_visitor(Is<FuncCallerTag>(), StackTop) ) //and the item on top is an operator
                break;
            auto StackOperator = boost::get<const boost::shared_ptr<IOperator>&>(StackTop);
            if( *StackOperator == *m_ParserContext.OpeningBracket() )
                break;
            //there's nothing to do if the current object on top of the stack is a paranthese
            auto it = std::find_if(m_ParserContext.BracketOperators().begin(), m_ParserContext.BracketOperators().end(), FindOpeningBracket(*StackOperator));
            if( it != m_ParserContext.BracketOperators().end() )
                break;

            if( Op->Associativity() == IOperator::AssociativityType::Left )
            {
                if( StackOperator->Precedence() >= Op->Precedence() )
                {
#ifdef DEBUG
                    std::cout << "pop to out: " << StackOperator->Representation() << "\n";
#endif
                    m_ParserContext.OutputQueue().push_back(StackOperator);
                    m_ParserContext.OperatorStack().pop();
                }
                else break;
            }
            else if( Op->Associativity() == IOperator::AssociativityType::Right )
            {
                if( StackOperator->Precedence() > Op->Precedence() )
                {
#ifdef DEBUG
                    std::cout << "pop to out: " << StackOperator->Representation() << "\n";
#endif
                    m_ParserContext.OutputQueue().push_back(StackOperator);
                    m_ParserContext.OperatorStack().pop();
                }
                else break;
            }
        }
    }
    void ParseBinaryOperator(const boost::shared_ptr<IOperator>& Op)
    {
        if( m_ParserContext.LastToken() == TokenType::None || m_ParserContext.LastToken() == TokenType::OpBinary ||
            m_ParserContext.LastToken() == TokenType::ArgSeperator || m_ParserContext.LastToken() == TokenType::OpeningBracket
          )
        {
            throw std::logic_error("There is an operand missing near operator " + Op->Representation());
        }
        PopHigherPrecedenceOperators(Op);
        m_ParserContext.OperatorStack().push(Op);
        m_ParserContext.LastToken() = TokenType::OpBinary;
    }
    void ThrowIfUnexpected(TokenType TType, const std::string& ErrMessage = "Unexpected Token") const
    {
        if( m_ParserContext.UnexpectedToken() == TType )
        {
            m_ParserContext.UnexpectedToken() = TokenType::None;
            throw std::logic_error(ErrMessage);
        }
    }
};

void ParserContext::Parse(const UnparsedToken& Tok) const
{
    Parser::Visitor V(m_Parser.m_Context);
    boost::apply_visitor(V,Tok);
}

void Parser::Parse( std::deque<UnparsedToken> TokExpr )
{
#ifdef DEBUG
    std::cout << "Tokenized Expression size: " << TokExpr.size() << std::endl;
#endif
    using Exceptions::ParseError;
    ParseError::Location Loc("Shell",1);
    m_Context.InputQueue(&TokExpr);
    m_Context.LastToken() = TokenType::None; //Before parsing, set the LastToken to a defined starttype
    Visitor v(m_Context);
    unsigned OriginalSize = TokExpr.size();
    while( !TokExpr.empty() )
    {
        UnparsedToken Tok = TokExpr.front();
        TokExpr.pop_front();
        try
        {
            boost::apply_visitor(v,Tok);
        }
        catch( std::exception& e )
        {
            while( m_Context.EndScope() );//In case of an error reset all scopes so that in the next call of Parse() the output goes in the right outputqueue
                //No-op
            throw ParseError("At Token " + boost::lexical_cast<std::string>(OriginalSize - TokExpr.size()) + ": " + e.what(),Loc);
        }
    } //while
    //if the last operator is a prefix or a binary signal an error, because it always needs more input to evaluate
    if( m_Context.LastToken() == TokenType::OpUnaryPrefix || m_Context.LastToken() == TokenType::OpBinary )
        throw ParseError("Missing input after operator",Loc);
    //Check if there are closing brackets missing
    if( m_ExpectedBracket.size() > 0 )
        throw ParseError("Expected '" + m_ExpectedBracket.top()->Representation() + "' before end of input",Loc);
    //No more tokens to be read; pop all operators left from the OpStack to the OutputQueue
    if( m_OperatorStack.size() == 1 )
    {
        while( !m_OperatorStack.top().empty() )
        {
            m_OutputQueue.push_back(boost::apply_visitor(Utilities::Get<boost::shared_ptr<IOperator>>(),m_OperatorStack.top().top()));
            m_OperatorStack.top().pop();
        }
    }
    /* FIXME (Marius#3#): Provide better error message */
    else throw ParseError("Scope missmatch: " + boost::lexical_cast<std::string>(m_OperatorStack.size()),Loc);
}
