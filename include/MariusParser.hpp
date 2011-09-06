#ifndef MARIUSPARSER_H
#define MARIUSPARSER_H
#define BOOST_VARIANT_LIMIT_TYPES 8
#include "Internal/Parser.hpp"
#include "Internal/Tokenizer.hpp"
#include "Internal/Utilities.hpp"
#include "Object.hpp"
#include "Function.hpp"
#include "Internal/BindFunc.hpp"
#include "Exceptions.hpp"
class MariusParser
{
public:
    /** Default constructor */
    MariusParser();
    /** Default destructor */
    ~MariusParser();
    /** \brief Registers a token via the IToken interface
     *
     * Ownership of the pointer will be taken, so you don't have to delete it yourself
     *
     * \param Tok Operator to register
     * \return void
     *
     */
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
    template<typename T>
    void RegisterFunction(const std::string& Name, const std::string& Representation, T Func)
    {
        RegisterFunction(Internal::BindFunc(Name,Representation,Func));
    }
    Types::Object Evaluate(const std::string& Input);

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
    Types::Object GlobalScope()
    {
        return Types::Object(m_EC,m_GlobalScope);
    }
protected:
private:
    Internal::Tokenizer m_Tokenizer;
    Internal::Parser m_Parser;
    Internal::MemoryController m_MC;
    Internal::Types::Scope m_GlobalScope;
    Internal::Types::Stack Stack;
    Internal::EvaluationContext m_EC;
};

#endif // MARIUSPARSER_H
