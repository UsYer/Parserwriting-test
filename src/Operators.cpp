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
using namespace Internal;

struct DoAssignment : public boost::static_visitor<Member>
{
    EvaluationContext& m_EC;
    const ResolvedToken& m_RHS;
    DoAssignment(EvaluationContext& EC, const ResolvedToken& RHS):
        m_EC(EC),
        m_RHS(RHS)
    {
    }
    Member operator()(long long val) const
    {
        if( !m_EC.This.IsNull() )
        { //we have an assignment to a table element. E.g.: table[0] = value
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
        if( m_EC.This.IsNull() )
            Scope = m_EC.Scope();
        else
        {
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
        if( Ref.IsNull() )
            throw Exceptions::NullReferenceException("Assignment to a nullreference");
        return Ref;
    }
    Member operator()(const NullReference&)const
    {
        throw Exceptions::NullReferenceException("Assignment to a nullreference");
    }
};

void AssignmentOp::Eval(EvaluationContext& EC)
{
    Types::Object RHS = EC.EvalStack.back();
    EC.EvalStack.pop_back();
    Types::Object LHS = EC.EvalStack.back();
    EC.EvalStack.pop_back();
    ResolvedToken RHSToken(Utilities::Resolve(EC,RHS));
    EC.This = LHS.This();
    EC.EvalStack.push_back(Types::Object(LHS.Visit(DoAssignment(EC,RHSToken))));
}
