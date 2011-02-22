#ifndef FUNCTIONS_HPP_INCLUDED
#define FUNCTIONS_HPP_INCLUDED
#include <iostream>
namespace Internal
{
class TypeFunc : public IFunction
{
    struct Type : public boost::static_visitor<std::string>
    {
        std::string operator()(long long val)const
        {
            return "long";
        }
        std::string operator()(double val)const
        {
            return "double";
        }
        std::string operator()(const boost::shared_ptr<IEvaluable>& op)const
        {
            return "function";
        }
        std::string operator()(const Reference& ref)const
        {
            return "reference -> table";
        }
        std::string operator()(const NullReference&)const
        {
            return "NullReference";
        }
    };
public:
    TypeFunc():IFunction("Typefunction","Type",1,0)
    {}
    void Eval(EvaluationContext& EC)
    {
        /*Types::Object Val = EC.EvalStack.back();
        EC.EvalStack.pop_back();
        ResolvedToken Token(Utilities::Resolve(EC,Val));*/
        std::cout << "\n" << boost::apply_visitor(Type(),m_LocalScope[0]) << "\n";
    }
};
class MaxFunc : public IFunction
{
    struct Max : public boost::static_visitor<NumberToken>
    {
        template<typename T, typename U>
        auto operator()(T lhs, U rhs) -> decltype(lhs > rhs ? lhs : rhs)
        {
            return lhs > rhs ? lhs : rhs;
        }
    };
public:
    MaxFunc():
        IFunction("Maxfunction","Max", /*ArgCount::Variable*/-1,1)
    {}
    void Eval(EvaluationContext& EC)
    {
        if( m_SuppliedArguments == 0 )
            return;
        NumberToken One, Two;
        Max Maximum;
        if( m_SuppliedArguments == 1 )
            One = Utilities::GetNumberToken(m_LocalScope[0]);
        else
        {
            const auto& Args = *boost::get<const CountedReference&>(m_LocalScope[0]); //the arguments have been placed in a table, get it
            One = Utilities::GetNumberToken(Args[0]);
            for( unsigned i = 1; i < m_SuppliedArguments; i++)
            {
                Two = Utilities::GetNumberToken(Args[i]);
                One = boost::apply_visitor(Maximum,One,Two);
            }
        }
        EC.EvalStack.push_back(Types::Object(One));
    }

};
class SqrtFunc : public IFunction
{
    struct Sqrt : boost::static_visitor<NumberToken>
    {
        template<typename T>
        NumberToken operator()(T Arg) const
        {
            return std::sqrt(Arg);
        }
    };
    public:
    SqrtFunc():
        IFunction("Returns the square root","Sqrt",1,1)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        NumberToken Arg(Utilities::GetNumberToken(m_LocalScope[0]));
        EC.EvalStack.push_back(Types::Object(boost::apply_visitor(Sqrt(),Arg)));
    }
};
class CreateTableFunc : public IFunction
{
public:
    CreateTableFunc():
        IFunction("CreateTableFunction","Table",/*ArgCount::Variable*/-1,1)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        /*Types::Table Tab;
        for( int i = m_SuppliedArguments - 1; i >= 0 ; i-- )
        {
            Tab[i] = Utilities::Resolve(EC, EC.EvalStack.back());
            EC.EvalStack.pop_back();
        }*/
        if( m_SuppliedArguments > 1 )
            EC.EvalStack.push_back(Types::Object(m_LocalScope[0]));
        else if ( m_SuppliedArguments == 1 )
        {
            Types::Table Tab;
            Tab[0] = m_LocalScope[0];
            EC.EvalStack.push_back(Types::Object(EC.MC.Save(Tab)));
        }
        else //0 args
            EC.EvalStack.push_back(Types::Object(EC.MC.Save(Types::Table())));
    }
};

class CreateNullFunc : public IFunction
{
public:
    CreateNullFunc():
        IFunction("CreateNullFunction","Null",0,1)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        EC.EvalStack.push_back(Types::Object(NullReference()));
    }
};
class GetRefCountFunc : public IFunction
{
public:
    GetRefCountFunc():IFunction("Gets the refcount of a reference","GetRefCount",1,1)
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
        Reference WeakRef;
        {
            Member Ref = boost::apply_visitor(Utilities::ResolveVisitor(EC),m_LocalScope[0]);
            WeakRef = boost::apply_visitor(Utilities::Get<CountedReference>(),Ref).GetWeakReference();
            m_LocalScope.ClearIndexValues();
        }
        long long Refcount = EC.MC.GetRefCount(WeakRef);
        EC.EvalStack.push_back(Types::Object(Refcount));
    }
};
}
#endif // FUNCTIONS_HPP_INCLUDED
