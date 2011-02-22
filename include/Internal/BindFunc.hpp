#ifndef BINDFUNC_HPP_INCLUDED
#define BINDFUNC_HPP_INCLUDED
#include "Utilities.hpp"
#include "Marshal.hpp"
#include <array>
#include <boost/function_types/result_type.hpp>
//From: http://bytes.com/topic/c/answers/872075-iterate-over-tuple#post3502501
/*template<class TupleType, size_t N>
struct do_iterate
{
    static void call(EvaluationContext& EC, std::vector<NumberToken>& Args)
    {
        NumberToken Arg(boost::apply_visitor(Utilities::GetNumberToken(Table),EvalStack.top())); EvalStack.pop();
        Args.push_back(boost::apply_visitor(Get<typename std::tuple_element<N, TupleType>::type>(),Arg));

        do_iterate<TupleType, N-1>::call(EvalStack,Table,Args);
        //std::get<N>(t).do_sth();
    }
};

template<class TupleType>
struct do_iterate<TupleType, 0>
{
    static void call(EvaluationContext& EC, std::vector<NumberToken>& Args)
    {
        NumberToken Arg(boost::apply_visitor(Utilities::GetNumberToken(Table),EvalStack.top())); EvalStack.pop();
        Args.push_back(boost::apply_visitor(Get<typename std::tuple_element<0, TupleType>::type>(),Arg));
        //std::get<0>(t).do_sth();
    }
};

template<class TupleType >
void iterate_tuple(EvaluationContext& EC, std::vector<NumberToken>& Args)
{
    do_iterate<TupleType, std::tuple_size<TupleType>::value-1>::call(EvalStack,Table,Args);
}*/
namespace Internal
{
template<typename T>
struct ReturnCounter
{
    enum {Value = 1};
};
template<>
struct ReturnCounter<void>
{
    enum {Value = 0};
};
template<typename... U>
struct ReturnCounter<std::tuple<U...>>
{
    enum {Value = std::tuple_size<std::tuple<U...>>::value};
};
/*
template<typename R, unsigned short A, typename... Args>
struct FuncMetaData
{
    typedef R ReturnType;
    typedef std::tuple<Args...> ArgTuple;
    static const unsigned short Argc = A;
    static const std::tuple<Args...> Arguments;
};
*/
//------------------
template<size_t Step>
struct do_iterate
{
    template<class ArgArray, typename Scope>
    static inline void call(EvaluationContext& EC, ArgArray& Args, const Scope& FuncScope)
    {
        Args[Step] = FuncScope[Step];
        do_iterate<Step-1>::call(EC,Args, FuncScope);
    }
};
template<>
struct do_iterate<0>
{
    template<class ArgArray, typename Scope>
    static inline void call(EvaluationContext& EC, ArgArray& Args, const Scope& FuncScope)
    {
        Args[0] = FuncScope[0];
    }
};
///Get the right scope where the arguments are placed. if ArgCount > 1: FuncScope[0] is a table with all the args. ArgCount == 1 FuncScope[0] is the arg itself
template<size_t>
struct GetScope
{
    template< typename Scope >
    static const Scope& ArgScope(const Scope& FuncScope)
    {
        return *boost::get<const CountedReference&>(FuncScope[0]);
    }
};
template<>
struct GetScope<1>
{
    template< typename Scope >
    static const Scope& ArgScope(const Scope& FuncScope)
    {
        return FuncScope;
    }
};
/*
///Gets the arguments and copies them in an array for the real function call
template<class ArgArray, typename Scope>
void inline GetArgs(EvaluationContext& EC, ArgArray& Args, const Scope& FuncScope)
{
    do_iterate<std::tuple_size<ArgArray>::value-1>::call(EC,Args, GetScope<std::tuple_size<ArgArray>::value>::ArgScope(FuncScope));
}
*/
//----------------
///Calls the native function and pushes the result if it doesn't return void
template<typename ReturnType>
struct DoCall
{
    template<typename Func, typename... Args>
    static void Call(EvaluationContext& EC, const Func& F, Args... A)
    {
        //Call the native function f and convert the return value to something compatible for internal usage
        EC.EvalStack.push_back(Marshal::Value<ReturnType>::ConvertIn(F(A...),EC));
    }
};
///specialisation for a function returning void
template<>
struct DoCall<void>
{
    template<typename Func, typename... Args>
    static void Call(EvaluationContext& EC, const Func& F, Args... A)
    {
        F(A...);
    }
};
/*
template<typename ArgTuple, std::size_t ArgCount>
struct Invoker;
//Sorry, too many arguments

template<typename ArgTuple>
struct Invoker<ArgTuple, 0>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func);
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 1>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,1> Args;
        GetArgs(EC,Args,FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;

        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC)
        );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 2>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,2> Args;
        GetArgs(EC,Args,FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC)
        );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 3>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,3> Args;
        GetArgs(EC,Args,FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC)
        );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 4>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,4> Args;
        GetArgs(EC,Args, FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        typedef typename std::tuple_element<3, ArgTuple>::type TypeFour;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC),
          Marshal::Value<TypeFour>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFour>::ToInternal>(),Args[3]),EC)
         );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 5>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,5> Args;
        GetArgs(EC,Args, FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        typedef typename std::tuple_element<3, ArgTuple>::type TypeFour;
        typedef typename std::tuple_element<4, ArgTuple>::type TypeFive;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC),
          Marshal::Value<TypeFour>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFour>::ToInternal>(),Args[3]),EC),
          Marshal::Value<TypeFive>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFive>::ToInternal>(),Args[4]),EC)
         );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 6>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,6> Args;
        GetArgs(EC,Args,FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        typedef typename std::tuple_element<3, ArgTuple>::type TypeFour;
        typedef typename std::tuple_element<4, ArgTuple>::type TypeFive;
        typedef typename std::tuple_element<5, ArgTuple>::type TypeSix;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC),
          Marshal::Value<TypeFour>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFour>::ToInternal>(),Args[3]),EC),
          Marshal::Value<TypeFive>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFive>::ToInternal>(),Args[4]),EC),
          Marshal::Value<TypeSix>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeSix>::ToInternal>(),Args[5]),EC)
         );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 7>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,7> Args;
        GetArgs(EC,Args,FuncScope);
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        typedef typename std::tuple_element<3, ArgTuple>::type TypeFour;
        typedef typename std::tuple_element<4, ArgTuple>::type TypeFive;
        typedef typename std::tuple_element<5, ArgTuple>::type TypeSix;
        typedef typename std::tuple_element<6, ArgTuple>::type TypeSeven;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC),
          Marshal::Value<TypeFour>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFour>::ToInternal>(),Args[3]),EC),
          Marshal::Value<TypeFive>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFive>::ToInternal>(),Args[4]),EC),
          Marshal::Value<TypeSix>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeSix>::ToInternal>(),Args[5]),EC),
          Marshal::Value<TypeSeven>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeSeven>::ToInternal>(),Args[6]),EC)
         );
        return;
    }
};
template<typename ArgTuple>
struct Invoker<ArgTuple, 8>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, const Scope& FuncScope)
    {
        std::array<ResolvedToken,8> Args;
        GetArgs(EC,Args,FuncScope);
        //List the types of the arguments of the native function for convenience
        typedef typename std::tuple_element<0, ArgTuple>::type TypeOne;
        typedef typename std::tuple_element<1, ArgTuple>::type TypeTwo;
        typedef typename std::tuple_element<2, ArgTuple>::type TypeThree;
        typedef typename std::tuple_element<3, ArgTuple>::type TypeFour;
        typedef typename std::tuple_element<4, ArgTuple>::type TypeFive;
        typedef typename std::tuple_element<5, ArgTuple>::type TypeSix;
        typedef typename std::tuple_element<6, ArgTuple>::type TypeSeven;
        typedef typename std::tuple_element<7, ArgTuple>::type TypeEight;
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<TypeOne>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeOne>::ToInternal>(),Args[0]),EC),
          Marshal::Value<TypeTwo>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeTwo>::ToInternal>(),Args[1]),EC),
          Marshal::Value<TypeThree>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeThree>::ToInternal>(),Args[2]),EC),
          Marshal::Value<TypeFour>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFour>::ToInternal>(),Args[3]),EC),
          Marshal::Value<TypeFive>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeFive>::ToInternal>(),Args[4]),EC),
          Marshal::Value<TypeSix>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeSix>::ToInternal>(),Args[5]),EC),
          Marshal::Value<TypeSeven>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeSeven>::ToInternal>(),Args[6]),EC),
          Marshal::Value<TypeEight>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<TypeEight>::ToInternal>(),Args[7]),EC)
         );
        return;
    }
};
*/
template<typename T, size_t ArgCount>
struct ScopeAccessor
{
    ScopeAccessor(T& FuncScope):
        m_ArgCount(ArgCount),
        m_FuncScope(FuncScope)
    {}
    ResolvedToken& GetArg()
    {
        return m_FuncScope[--m_ArgCount];
    }
    size_t m_ArgCount;
    T& m_FuncScope;
};
template<typename... Args>
struct VariadicInvoker
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, Scope& FuncScope)
    {
        auto RealScope = GetScope<sizeof...(Args)>::ArgScope(FuncScope);
        ScopeAccessor<Scope, sizeof...(Args)> Accessor(RealScope);

        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func,
          Marshal::Value<Args>::ConvertOut(boost::apply_visitor(Utilities::Get<typename Marshal::Type<Args>::ToInternal>(),Accessor.GetArg()),EC)...
         );
        return;
    }
};
///Specialization for no arguments
template<>
struct VariadicInvoker<>
{
    template<typename T, typename Scope>
    static void Invoke( T Func, EvaluationContext& EC, Scope& FuncScope)
    {
        DoCall<typename boost::function_types::result_type<T>::type>::Call(EC,Func);
        return;
    }
};
template<typename T, typename ReturnType, typename... Args>
class UserDefFunction : public IFunction
{
public:
    virtual ~UserDefFunction() {}
    UserDefFunction(const std::string& Name, const std::string& Representation, T Func):
        IFunction(Name, Representation, sizeof...(Args), ReturnCounter<ReturnType>::Value),
        Function(Func)
    {
    }
    void Eval(EvaluationContext& EC)
    {
        try
        {
            //Invoker<typename Meta::ArgTuple, Meta::Argc>::Invoke(Function,EC,IFunction::m_LocalScope);
            VariadicInvoker<Args...>::Invoke(Function,EC,IFunction::m_LocalScope);
        }
        catch(std::logic_error& E)
        {
            throw std::logic_error("In function \"" + m_Representation + "\" " + E.what());
        }
    }
private:
    T Function;
};
template<typename T,typename... U>
boost::shared_ptr<IFunction> BindFunc(const std::string& Name, const std::string& Representation, T (*UserFunc)(U...) )
{
    return boost::shared_ptr<IFunction>(new UserDefFunction<T (*)(U...),T,U...> (Name, Representation,UserFunc));
}
}//namespace Internal
#endif // BINDFUNC_HPP_INCLUDED
