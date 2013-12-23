#include <queue>
#include <iostream>
#include <memory>
#include <boost/variant.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "../include/Internal/IOperator.hpp"
#include "../include/Internal/Tokenizer.hpp"
#include "../include/Internal/Types.hpp"
using namespace Internal;

bool SetOrdering::operator()(const IToken& lhs, const IToken& rhs) const
{
    return lhs.Representation() < rhs.Representation();
}
void Tokenizer::RegisterToken(IToken* Token) //not const because ptr_set requires lvalues
{
    Operators.insert(Token);
}
std::deque<UnparsedToken> Tokenizer::Tokenize(const std::string& expression)
{
    return TokenizeGreedy(expression);
}

std::deque<UnparsedToken> Tokenizer::TokenizeGreedy(const std::string& expression)
{
    std::string UnrecognizedBuffer; //Will be used for identifiers and such
    bool DecimalNumber = false;

    for( auto ch = expression.cbegin(); ch < expression.cend(); /*++ch*/ )//intentionally used less than, because otherwise we'd get some strange results
    {
        if( std::isalpha(*ch) || '_' == *ch || '@' == *ch ) //list of characters allowed in an identifier. For std::isalpha see: http://www.cplusplus.com/reference/clibrary/cctype/
        {
            ParseOperator();
            FlushNumBuffer(DecimalNumber);
            DecimalNumber = false;
            do
            {
                OperatorBuffer += *ch;
                ++ch;
            } while( ch != expression.cend() && (std::isalnum(*ch) || '_' == *ch || '@' == *ch));
            LastChar = LastCharType::Unrecognized;
#ifdef DEBUG
            std::cout << "Unrecognized: " << OperatorBuffer<< std::endl;
#endif
            continue;
        }
        else if( std::isdigit(*ch) )
        {
            if( ch != expression.cbegin() && LastChar != LastCharType::Number )
            {//If it's the first character we don't need to search for an operator before it
                ParseOperator(); //Parse Operator, because there could have been one before this number
            }
            do
            {
                m_NumberBuffer += *ch;
                ++ch;
            }
            while( ch != expression.cend() && std::isdigit(*ch) );
            LastChar = LastCharType::Number;
#ifdef DEBUG
            std::cout << "Numbuffer: " << m_NumberBuffer << std::endl;
#endif
            continue;
        }
        else if( LastChar == LastCharType::Number && *ch == '.' ) //Falls es sich um eine Kommazahl handelt
        {
            m_NumberBuffer += *ch;
            ++ch;
            DecimalNumber = true;
            LastChar = LastCharType::Dot;
            continue;
        }
        else if( *ch == '"')
        {
            ParseOperator();
            FlushNumBuffer(DecimalNumber);
            DecimalNumber = false;

            bool stringCorrectlyTerminated = false;

            auto beg = ++ch; // increment to the next character and save it as the beginning of our string.
            for( ; ch < expression.cend(); ch++ )
            {
                if( *ch == '"' && *(ch--) != '\\' )
                {
                    stringCorrectlyTerminated = true;
                }
            }

            if( !stringCorrectlyTerminated )
            {
                throw std::logic_error("SyntaxError: String not properly terminated; hit end of input.")
            }

            //assign iterators to Utf8String
        }
        /*
        else if( *ch == '(' )
        {
            ParseOperator();
            FlushNumBuffer(DecimalNumber);
            DecimalNumber = false; //!Important! reset the DecimalNumber flag because otherwise all following numbers will be regarded as doubles
            OutputQueue.push_back(std::shared_ptr<IOperator>(new OpeningBracket));
            ++ch;
            LastChar = LastCharType::Bracket;
            continue;
        }
        else if( *ch == ')' )
        {
            ParseOperator();
            FlushNumBuffer(DecimalNumber);
            DecimalNumber = false;
            OutputQueue.push_back(std::shared_ptr<IOperator>(new ClosingBracket));
            ++ch;
            LastChar = LastCharType::Bracket;
            continue;
        }*/
        else if( std::isspace(*ch) )
        {   //Ende eines zusammenhängendes Zeichens oder Zeilenende erreicht
            ParseOperator();
            FlushNumBuffer(DecimalNumber);
            DecimalNumber = false;
            if( *ch == '\n' )
            {
                OperatorBuffer += '\n'; //newline character is added on the OpBuffer, because otherwise the EOLToken will not be parsed.
            }
            do
            {
                ++ch;
            } while( ch != expression.cend() && std::isspace(*ch) );
            LastChar = LastCharType::None;
            continue;
        }
        //
        FlushNumBuffer(DecimalNumber);
        DecimalNumber = false;
        OperatorBuffer += *ch;
        ++ch;
        if( LastChar != LastCharType::Start && LastChar != LastCharType::None) //Am Anfang dürfen wir nicht gleich auf Unrecognized schalten, da MinusToken ansonsten nicht rausfinden könnte, ob das Minus am Anfang und somit unär ist
            LastChar = LastCharType::Unrecognized; //The actual char is not yet recognized
    }
    ParseOperator();
    FlushNumBuffer(DecimalNumber);
    return OutputQueue;
}

bool Tokenizer::ParseOperator()
{
    if( OperatorBuffer.empty() )
    {
#ifdef DEBUG
        std::cout << "OpBuffer empty, nothing to be done\n";
#endif
        return false;
    }
#ifdef DEBUG
    std::cout << "ParseOperator(): " << OperatorBuffer << "\n";
#endif
    for( auto it = Operators.rbegin(); it != Operators.rend(); it++ )//Biggest to smallest
    {
        std::string Rep(it->Representation()); //should be move-constructed
        std::size_t found = OperatorBuffer.find(Rep);
        if( found != std::string::npos )
        {
            if(found > 0) //Is there something left of the operator
            {
                std::string Identifier(OperatorBuffer.substr(0,found));
#ifdef DEBUG
                std::cout << "Identifier: " << Identifier << "\n";
#endif
                //Walk down to the left of the operator
                ParseOperator(Identifier);//Make sure, that there're no shadowed operators in the Identifier left of the operator
                if( !Identifier.empty() )
                    OutputQueue.push_back(Identifier);
            }
            OperatorBuffer.erase(0,found + Rep.length());//Erase the identifier in front of the operator, if there was one, and the operator itself
            LastChar = it->Tokenize(m_Context);
#ifdef DEBUG
            std::cout << "Operator: " << Rep << "\n";
#endif
            //Walk down right of the operator
            ParseOperator(); //Recursive call because there may be more operators in the buffer e.g. +test+
            //While the first + was recognized the next operator after identifier has yet to be recognized
            return true;
        }
    }
    //No operator recognized, but since the OpBuffer is not empty it has to be just an identifier
    std::string Identifier;
    Identifier.swap(OperatorBuffer);
#ifdef DEBUG
    std::cout << "Identifier: " << Identifier << "\n";
#endif
    OutputQueue.push_back(Identifier);//clean the OpBuffer and push the Identifier
    LastChar = LastCharType::Identifier;
    return false;
}

bool Tokenizer::ParseOperator(std::string& Expr)
{
    if( Expr.empty() )
    {
#ifdef DEBUG
        std::cout << "Expression empty, nothing to be done" << std::endl;
#endif
        return false;
    }
#ifdef DEBUG
    std::cout << "ParseOperator(" << Expr << ")" << std::endl;
#endif
    bool HasFound = false;
    for( auto it = Operators.rbegin(); it != Operators.rend(); it++ )//Biggest to smallest
    {
        std::string Rep(it->Representation()); //should be move-constructed
        std::size_t found = Expr.find(Rep);
        while( found != std::string::npos )
        {
            if(found > 0) //There is something left of the operator, check it for operators
            {
                std::string Identifier(Expr.substr(0,found));
#ifdef DEBUG
                std::cout << "Identifier: " << Identifier << std::endl;
#endif
                //Make sure, that there're no shadowed operators in the Identifier. E.g.: searching for '++': )++ ')' would be shadowed
                ParseOperator(Identifier);
                if( !Identifier.empty() )
                    OutputQueue.push_back(Identifier);
            }
            Expr.erase(0,found + Rep.length());//Erase the identifier in front of the operator, if there was one, and the operator itself
            LastChar = it->Tokenize(m_Context);
#ifdef DEBUG
            std::cout << "Operator: " << Rep << std::endl;
#endif
            HasFound = true;
            //look for the same operator after this one. E.g: '))'
            found = Expr.find(Rep);
        }
    }
    return HasFound;
}

void Tokenizer::FlushNumBuffer(bool DecimalNumber)
{
    if( !m_NumberBuffer.empty() )
    {
#ifdef DEBUG
        std::cout << "Flushing NumBuffer: " << m_NumberBuffer << std::endl;
#endif
        try
        {
            if( DecimalNumber )
                OutputQueue.push_back(boost::lexical_cast<double>(m_NumberBuffer));
            else
                OutputQueue.push_back(boost::lexical_cast<long long>(m_NumberBuffer));
        }
        catch(boost::bad_lexical_cast& e)
        {
            throw std::logic_error(e.what());
        }
        m_NumberBuffer.clear();
    }
}
