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
        {
            EC.Stack.Push(NullReference());
            return;
        }
        NumberToken One, Two;
        Max Maximum;
        if( m_SuppliedArguments == 1 )
            One = Utilities::GetNumberToken(IFunction::GetArg(0));
        else
        {
            One = Utilities::GetNumberToken(IFunction::GetArg(0));
            for( unsigned i = 1; i < m_SuppliedArguments; i++)
            {
                Two = Utilities::GetNumberToken(IFunction::GetArg(i));
                One = boost::apply_visitor(Maximum,One,Two);
            }
        }
        EC.Stack.Push(One);
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
        NumberToken Arg(Utilities::GetNumberToken(IFunction::GetArg(0)));
        EC.Stack.Push(boost::apply_visitor(Sqrt(),Arg));
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
        if( m_SuppliedArguments > 1 ) //More than one arg means the arg table itself will be returned
            EC.Stack.Push(m_LocalScope[0]);
        else if ( m_SuppliedArguments == 1 )
        {
            Types::Table Tab;
            Tab[0] = IFunction::GetArg(0);
            EC.Stack.Push(EC.MC.Save(Tab));
        }
        else //0 args
            EC.Stack.Push(EC.MC.Save(Types::Table()));
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
        EC.Stack.Push(NullReference());
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
//            Member Ref = boost::apply_visitor(Utilities::ResolveVisitor(EC),m_LocalScope[0]);
            WeakRef = IFunction::GetArg<CountedReference>(0).GetWeakReference();
            m_LocalScope.ClearIndexValues();
        }
        long long Refcount = EC.MC.GetRefCount(WeakRef);
        EC.Stack.Push(Refcount);
    }
};
class GetMCSizeFunc : public IFunction
{
public:
    GetMCSizeFunc():IFunction("Gets the amount of refs managed by the MC","ItemCount",0,1)
    {
    }
    virtual void Eval(EvaluationContext& EC)
    {
        EC.Stack.Push((long long)EC.MC.Size());
    }
};
class PrintFunc : public IFunction
{
public:
    PrintFunc():
        IFunction("","Print",/*ArgCount::Variable*/-1,0)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        for( unsigned i = 0; i < m_SuppliedArguments; i++ )
        {
            std::cout << Utilities::PrintValue(EC,Types::Object(IFunction::GetArg(i))) << " ";
        }
//        if( m_SuppliedArguments > 1 )
//        {
//            const auto& ArgTable = *boost::apply_visitor(Utilities::Get<CountedReference>(),m_LocalScope[0]);
//            for( auto it = ArgTable.IndexBegin(); it != ArgTable.IndexEnd(); it++)
//                std::cout << Utilities::PrintValue(EC,Types::Object(it->second)) << " ";
//        }
//        else if ( m_SuppliedArguments == 1 )
//        {
//            std::cout << Utilities::PrintValue(EC,Types::Object(m_LocalScope[0]));
//        }
        std::cout << std::endl;
    }
};
// TODO (Marius#8#): Finish global catch handler (translation from runtime to native exceptions)
class GlobalExceptionHandleFunc : public IFunction
{
public:
    GlobalExceptionHandleFunc():
        IFunction("","__GLOBALCATCH__",1,0)
    {
    }
    virtual void Eval( EvaluationContext& EC )
    {
        const auto& Exception = IFunction::GetArg<CountedReference>(0);
        auto TypeId = boost::apply_visitor(Utilities::Get<long long>(),(*Exception)["TypeId"]);
        throw Exceptions::RuntimeException("Unhandled runtime exception",Exceptions::ExceptionNames[TypeId],TypeId);
    }
};

class CreateExceptionFunc : public IFunction
{
    long long m_Id;
    public:
    CreateExceptionFunc(const std::string& Name, long long Id):
        IFunction("",Name,0,1),
        m_Id(Id)
    {}
    virtual void Eval( EvaluationContext& EC )
    {
        Types::Table ExceptionTable;
        ExceptionTable["__EXCEPTION__"] = 1LL;
        ExceptionTable["TypeId"] = m_Id;
        EC.Stack.Push(EC.MC.Save(ExceptionTable));
    }
};
}//ns Internal
#endif // FUNCTIONS_HPP_INCLUDED
