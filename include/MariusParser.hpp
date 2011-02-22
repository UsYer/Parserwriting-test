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
namespace Internal
{
struct Evaluator : public boost::static_visitor<>
{
private:
public:
    Evaluator();
    void operator()(long long val)
    {
        Stack.push_back(Types::Object(val));
    }
    void operator()(double val)
    {
        Stack.push_back(Types::Object(val));
    }
    void operator()(const boost::shared_ptr<IEvaluable>& Evaluable);
    void operator()(const std::string& s)
    {
        Stack.push_back(Types::Object(s));
    }
    void operator()(const Reference& Ref)
    {
        Stack.push_back(Types::Object(Ref));
    }
    void operator()(const NullReference& Ref)
    {
        Stack.push_back(Types::Object(Ref));
    }
    ::Types::Function GetFunction(const std::string& Identifier) const
    {
            auto it = (*m_GlobalScope).Find(Identifier);
            if( it == (*m_GlobalScope).KeyEnd() )
            {
                const boost::shared_ptr<IFunction>& Func(boost::apply_visitor(Utilities::Get<const boost::shared_ptr<IFunction>&>(),it->second));
                //Used const_casts here because there were otherwise some strange compiler errors
                return ::Types::Function(Func, const_cast<Types::Scope&>(m_GlobalScope), const_cast<MemoryController&>(m_MC));
            }
            throw std::logic_error("Function \"" + Identifier + "\" unknown");
    }
    MemoryController m_MC;
    Types::Scope m_GlobalScope;
    Types::Scope m_ActiveScope;
    Types::Stack Stack;
    EvaluationContext m_EC;
};


//class IToken;//Forward declaration for RegisterOperator

}
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
    void RegisterFunction(const boost::shared_ptr<Internal::IFunction>& p)
    {
        (*m_Evaluator.m_GlobalScope)[p->Representation()] = p;
    }
    void RegisterFunction(const std::string& Name, const std::string& Representation, int ArgCount, unsigned ReturnCount,const Internal::CallbackFunction& Func)
    {
        boost::shared_ptr<Internal::IFunction> P(new Internal::BindFunction(Name, Representation, ArgCount, ReturnCount,Func));
        (*m_Evaluator.m_GlobalScope)[Representation] = P;
    }
    template<typename T>
    void RegisterFunction(const std::string& Name, const std::string& Representation, T Func)
    {
        RegisterFunction(Internal::BindFunc(Name,Representation,Func));
    }
    Types::Object Evaluate(const std::string& Input);

    ::Types::Function GetFunction(const std::string& Identifier) const
    {
            return m_Evaluator.GetFunction(Identifier);
    }
    Types::Object GlobalScope()
    {
        return Types::Object(m_Evaluator.m_EC,m_Evaluator.m_GlobalScope);
    }
protected:
private:
    Internal::Tokenizer m_Tokenizer;
    Internal::Parser m_Parser;
    Internal::Evaluator m_Evaluator;
};

#endif // MARIUSPARSER_H
