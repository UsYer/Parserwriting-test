#ifndef FUNCTION_HPP_INCLUDED
#define FUNCTION_HPP_INCLUDED
#include "Internal/Marshal.hpp"
namespace Types
{
class Object;
class Function
{
    std::shared_ptr<Internal::IFunction> m_Function;
    Internal::Types::Scope m_SymTable;
    Internal::MemoryController& m_MC;
    template<typename T>
    void MakeArgTable(Internal::Types::Table& Args, Internal::EvaluationContext& EC, T Val) const
    {
        Args.Add(Internal::Marshal::Value<T>::ConvertInNoObject(Val,EC));
    }
    template<typename T, typename... R>
    void MakeArgTable(Internal::Types::Table& Args, Internal::EvaluationContext& EC, T Val, R... Rest) const
    {
        Args.Add(Internal::Marshal::Value<T>::ConvertInNoObject(Val,EC));
        MakeArgTable(Args,EC,Rest...);
    }
    void MakeArgTable(Internal::EvaluationContext& EC) const
    {
    }
    public:
    Function(const std::shared_ptr<Internal::IFunction>& Function, const Internal::Types::Scope& Table, Internal::MemoryController& MC):
        m_Function(Function),
        m_SymTable(Table),
        m_MC(MC)
    {

    }
    template<typename... T>
    Object operator()(T... Args)
    {
        Internal::Types::Stack LocalStack;
        Internal::EvaluationContext LocalEC(LocalStack,m_SymTable,m_MC);
        Internal::Types::Table ArgTab;
        MakeArgTable(ArgTab,LocalEC,Args...);

        m_Function->SuppliedArguments(m_MC.Save(ArgTab), sizeof...(Args));
        m_Function->Eval(LocalEC);

        if( m_Function->ReturnCount() == 0 )
            return Object(LocalEC, Internal::NullReference());
        else if( m_Function->ReturnCount() == 1 )
        {
            Object Obj(LocalEC,Internal::Utilities::Resolve(LocalEC,LocalEC.EvalStack.back()));
            LocalEC.EvalStack.pop_back();
            return Obj;
        }
        else
        {
            Internal::Types::Table Tab;
            for( unsigned i = 1; i <= m_Function->ReturnCount(); i++ )
            {
                Tab.Add(Internal::Utilities::Resolve(LocalEC,LocalEC.EvalStack.back()));
                LocalEC.EvalStack.pop_back();
            }
            return Object(LocalEC,LocalEC.MC.Save(Tab));
        }
    }
    int ArgCount() const
    {
        return m_Function->ArgCount();
    }
    unsigned ReturnCount() const
    {
        return m_Function->ReturnCount();
    }
};
}//namespace Types
namespace Marshal
{
template<>
struct Type<Types::Function>
{
    typedef std::shared_ptr<Internal::IFunction> ToInternal;
};
template<>
struct Value<Types::Function>
{
    static Types::Function ConvertOut(const std::shared_ptr<Internal::IFunction>& Val, Internal::EvaluationContext& EC)
    {
        return Types::Function(Val,EC.Scope(),EC.MC);
    }
};
}//ns Marshal
#endif // FUNCTION_HPP_INCLUDED
