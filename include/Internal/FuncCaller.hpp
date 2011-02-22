#ifndef FUNCCALLER_HPP_INCLUDED
#define FUNCCALLER_HPP_INCLUDED
#include "Utilities.hpp"
#include "Table.hpp"
#include "Get.hpp"
namespace Internal
{
class FuncCaller : public IOperator
{
    struct GetFunc: public boost::static_visitor<boost::shared_ptr<IFunction>>
    {
        EvaluationContext& m_EC;
        GetFunc(EvaluationContext& EC):
            m_EC(EC)
        {

        }
        boost::shared_ptr<IFunction> operator()(long long ) const
        {
            throw std::logic_error("Expected function; Is long");
        }
        boost::shared_ptr<IFunction> operator()(double )const
        {
            throw std::logic_error("Expected function; Is double");
        }
        boost::shared_ptr<IFunction> operator()(const boost::shared_ptr<IOperator>& op)const
        {
            throw std::logic_error("Expected function; Is operator " + op->Representation());
        }
        boost::shared_ptr<IFunction> operator()(const Reference& R)const
        {
            if( R.IsNull() )
                throw std::logic_error("Calling a nullreference");
            else
                throw std::logic_error("Expected function; Is table");
        }
        boost::shared_ptr<IFunction> operator()(NullReference)const
        {
            throw std::logic_error("Calling a nullreference");
        }
        boost::shared_ptr<IFunction> operator()(const boost::shared_ptr<IFunction>& op)const
        {
            return op;
        }
    };
public:
    FuncCaller():
        IOperator("Function caller","FuncCaller",50,ArityType::Binary,AssociativityType::Left)
    {

    }
    virtual void Eval(EvaluationContext& EC)
    {
        //long long ArgCount = boost::get<long long>(EC.EvalStack.back());
        long long ArgCount = EC.EvalStack.back().Visit(Utilities::Get<long long>("Fatal: missing number of arguments for function call"));
        EC.EvalStack.pop_back();

        auto FuncPos = EC.EvalStack.begin() + (EC.EvalStack.size() - 1 - ArgCount); //We jump directly to the identifer of the function
        ResolvedToken FuncToken(Utilities::Resolve(EC,*FuncPos)); //needs to stay as a LHS because apply_visitor the next line dos not like RHS
        boost::shared_ptr<IFunction> Func(boost::apply_visitor(GetFunc(EC),FuncToken));
        if( Func->ArgCount() != /*ArgCount::Variable*/-1 )
        {
            if( ArgCount > Func->ArgCount() )
                throw std::logic_error("Too many arguments for function '" + Func->Representation() + "'; should be " + boost::lexical_cast<std::string>(Func->ArgCount()) + " argument(s)");
            else if( ArgCount < Func->ArgCount() )
                throw std::logic_error("Too few arguments for function '" + Func->Representation() + "'; should be " + boost::lexical_cast<std::string>(Func->ArgCount()) + " argument(s)");
        }
        bool EraseAfterCall = true;
        if( ArgCount <= Func->ReturnCount() )
        {
            EC.EvalStack.erase(FuncPos);
            EraseAfterCall = false;
        }
        Func->SuppliedArguments(ArgCount);
        Func->Eval(EC);
        if(EraseAfterCall)
        {
            //Erase the function identifier from the stack
            //it's done after the function call because in general there are less returned arguments than paramters, resulting in a better performance
            //but it requires, that no elements are deleted in the middle of the stack in the function call
            EC.EvalStack.erase(FuncPos);
        }
    }
    virtual ~FuncCaller() {}
protected:

private:

};
}

#endif // FUNCCALLER_HPP_INCLUDED
