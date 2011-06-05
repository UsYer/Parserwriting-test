#include <boost/lexical_cast.hpp>
#ifdef DEBUG
#include <iostream>
#endif
#include "../include/Internal/Types.hpp"
#include "../include/Internal/IFunction.hpp"
#include "../include/Internal/Utilities.hpp"
using namespace Internal;
namespace
{
struct ArgAssigner : public boost::static_visitor<long long>
{
    Types::Table& m_FuncScope;
    IFunction& m_Func;
    ArgAssigner(Types::Table& FuncScope, IFunction& Func):
        m_FuncScope(FuncScope),
        m_Func(Func)
    {}
    template <typename T>
    long long operator()(const T& Arg) const
    {
        AssignOneArg(Arg);
        return 1;
    }
    long long operator()(const CountedReference& Ref) const
    {
        auto it = (*Ref).Find("__ARGCOUNT__");
        if( it != (*Ref).KeyEnd() )
        {
            long long ArgCount = boost::apply_visitor(Utilities::Get<long long>("ArgCount field of the argument table has to be an integer"),it->second);
            if( m_Func.ArgCount() != /*ArgCount::Variable*/-1 )
            {
                if( ArgCount > m_Func.ArgCount() )
                    throw std::logic_error("Too many arguments for function '" + m_Func.Representation() + "'; should be " + boost::lexical_cast<std::string>(m_Func.ArgCount()) + " argument(s)");
                else if( ArgCount < m_Func.ArgCount() )
                    throw std::logic_error("Too few arguments for function '" + m_Func.Representation() + "'; should be " + boost::lexical_cast<std::string>(m_Func.ArgCount()) + " argument(s)");
            }
            if( m_FuncScope.KeySize() == 0 ) //No named parameters means we have a nativ function, just put it in the Index area
            {
                auto IndexIt = (*Ref).IndexBegin();
                for(unsigned i = 0; i < ArgCount; i++, std::advance(IndexIt,1) )
                {
                    m_FuncScope[i] = IndexIt->second;
                }
            }
            else
            {
                /*
                Example: function f(a,b) ... end => m_LocalScope only consists of keys: a,b
                f(1,2) => ArgTable consists only of indices: (1,2)
                map the args to the expected paramteter:
                auto KeyIt = m_LocalScope.KeyBegin() => a
                auto IndexIt = ArgTable.IndexBegin() => 1
                KeyIt->second = IndexIt->second => a = 1
                goes on with the next arg
                */
                auto IndexIt = (*Ref).IndexBegin();//passed arguments
                auto KeyIt = m_FuncScope.KeyBegin();//expected parameter
                for( unsigned i = 0; i < ArgCount; std::advance(KeyIt,1), std::advance(IndexIt,1), i++ )
                {
                    KeyIt->second = IndexIt->second;
                }
            }
            return ArgCount;
        }
        else
        { //the table counts as one argument
            AssignOneArg(Ref);
            return 1;
        }
    }
    private:
    template <typename T>
    void AssignOneArg(const T& Arg) const
    {
        if( m_Func.ArgCount() != /*ArgCount::Variable*/-1 )
        {
            if( 1 > m_Func.ArgCount() )
                throw std::logic_error("Too many arguments for function '" + m_Func.Representation() + "'; should be " + boost::lexical_cast<std::string>(m_Func.ArgCount()) + " argument(s)");
            else if( 1 < m_Func.ArgCount() )
                throw std::logic_error("Too few arguments for function '" + m_Func.Representation() + "'; should be " + boost::lexical_cast<std::string>(m_Func.ArgCount()) + " argument(s)");
        }
        if( m_FuncScope.KeySize() == 0 ) //No named parameters means we have a nativ function, just put it in the Index area
            m_FuncScope[0] = Arg;
        else
            m_FuncScope.KeyBegin()->second = Arg;
    }
};
}
void IFunction::SuppliedArguments(const Member& Args)
{
    m_SuppliedArguments = boost::apply_visitor(ArgAssigner(m_LocalScope,*this),Args);
}
void IFunction::SuppliedArguments(const Member& Args, int ArgCount)
{
    if( m_ArgCount != -1)
    { //if the function does not take a variable amount of args check if it's the right amount
        if( ArgCount > m_ArgCount )
            throw std::logic_error("Too many arguments for function '" + Representation() + "'; should be " + boost::lexical_cast<std::string>(m_ArgCount) + " argument(s)");
        else if( ArgCount < m_ArgCount )
            throw std::logic_error("Too few arguments for function '" + Representation() + "'; should be " + boost::lexical_cast<std::string>(m_ArgCount) + " argument(s)");
    }
    m_SuppliedArguments = ArgCount;
    if( !ArgCount ) //nothing to do for zero args
        return;
    if( ArgCount == 1 )
    {
        #ifdef DEBUG
        std::cout << "funccall received 1 Arg\n";
        #endif
        if( m_LocalScope.KeySize() == 0 ) //No named parameters means we have a native function, just put it in the Index area
        {
            #ifdef DEBUG
            std::cout << "native func, assigning: LS[0] = " << boost::apply_visitor(Utilities::PrintValueNoResolve(),Args) << "\n";
            #endif
            m_LocalScope[0] = Args;
        }
        else
        {
            #ifdef DEBUG
            std::cout << "runtime func, assigning: " << m_LocalScope.KeyBegin()->first << "=" << boost::apply_visitor(Utilities::PrintValueNoResolve(),Args) << "\n";
            #endif
            m_LocalScope.KeyBegin()->second = Args;
        }
    }
    else //more than one arg, read the values from the table inside Args
    {
        const auto& ArgTable = boost::apply_visitor(Utilities::Get<CountedReference>("fatal: multiple Args but no table provided"),Args);
        if( m_LocalScope.KeySize() == 0 ) //No named parameters means we have a nativ function, just put it in the Index area
        {
            #ifdef DEBUG
            std::cout << "native func\n";
            #endif
            (*ArgTable).ClearKeyValues(); //remove __ARGCOUNT__ from the arg table, because it would count as a named parameter in the next call
            m_LocalScope[0] = ArgTable;
        }
        else
        {
            #ifdef DEBUG
            std::cout << "runtime func\n";
            #endif
            /*
            Example: function f(a,b) ... end => m_LocalScope only consists of keys: a,b
            f(1,2) => ArgTable consists only of indices: (1,2)
            map the args to the expected paramteter:
            auto KeyIt = m_LocalScope.KeyBegin() => a
            auto IndexIt = ArgTable.IndexBegin() => 1
            KeyIt->second = IndexIt->second => a = 1
            goes on with the next arg
            */
            auto IndexIt = (*ArgTable).IndexBegin();//passed arguments
            auto KeyIt = m_LocalScope.KeyBegin();//expected parameter
            for( unsigned i = 0; i < ArgCount; std::advance(KeyIt,1), std::advance(IndexIt,1), i++ )
            {
                #ifdef DEBUG
                std::cout << KeyIt->first << " = " << boost::apply_visitor(Utilities::PrintValueNoResolve(),IndexIt->second) << "\n";
                #endif
                KeyIt->second = IndexIt->second;
            }
        }
    }
//    m_SuppliedArguments = ArgCount;
}
