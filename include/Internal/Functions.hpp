#ifndef FUNCTIONS_HPP_INCLUDED
#define FUNCTIONS_HPP_INCLUDED
#include <iostream>
#include "IFunction.hpp"

namespace Internal
{
	struct EvaluationContext;
class TypeFunc : public IFunction
{
    
public:
    TypeFunc():IFunction("Typefunction","Type",1,0)
    {}
	void Eval(EvaluationContext& EC) override;
};
class MaxFunc : public IFunction
{
    
public:
    MaxFunc():
        IFunction("Maxfunction","Max", /*ArgCount::Variable*/-1,1)
    {}
	void Eval(EvaluationContext& EC) override;

};
class SqrtFunc : public IFunction
{
    public:
    SqrtFunc():
        IFunction("Returns the square root","Sqrt",1,1)
    {
    }
	void Eval(EvaluationContext& EC) override;
};
class CreateTableFunc : public IFunction
{
public:
    CreateTableFunc():
        IFunction("CreateTableFunction","Table",/*ArgCount::Variable*/-1,1)
    {
    }
	void Eval(EvaluationContext& EC) override;
};

class CreateNullFunc : public IFunction
{
public:
    CreateNullFunc():
        IFunction("CreateNullFunction","Null",0,1)
    {
    }
	void Eval(EvaluationContext& EC) override;
};
class GetRefCountFunc : public IFunction
{
public:
    GetRefCountFunc():IFunction("Gets the refcount of a reference","GetRefCount",1,1)
    {
    }
	virtual void Eval(EvaluationContext& EC) override;
};
class GetMCSizeFunc : public IFunction
{
public:
    GetMCSizeFunc():IFunction("Gets the amount of refs managed by the MC","ItemCount",0,1)
    {
    }
	virtual void Eval(EvaluationContext& EC) override;
};
class PrintFunc : public IFunction
{
public:
    PrintFunc():
        IFunction("","Print",/*ArgCount::Variable*/-1,0)
    {
    }
	void Eval(EvaluationContext& EC) override;
};
// TODO (Marius#8#): Finish global catch handler (translation from runtime to native exceptions)
class GlobalExceptionHandleFunc : public IFunction
{
public:
    GlobalExceptionHandleFunc():
        IFunction("","__GLOBALCATCH__",1,0)
    {
    }
	virtual void Eval(EvaluationContext& EC) override;
};

class CreateExceptionFunc : public IFunction
{
    long long m_Id;
    public:
    CreateExceptionFunc(const std::string& Name, long long Id):
        IFunction("",Name,-1,1),
        m_Id(Id)
    {}
	virtual void Eval(EvaluationContext& EC) override;
};
}//ns Internal
#endif // FUNCTIONS_HPP_INCLUDED
