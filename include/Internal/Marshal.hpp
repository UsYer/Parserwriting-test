#ifndef MARSHAL_HPP_INCLUDED
#define MARSHAL_HPP_INCLUDED
#include <type_traits>

namespace Marshal
{
template<typename T>
struct Type
{
    typedef T ToInternal;
};
template<typename T>
struct Value
{
    static T ConvertOut(const T& Val, const Internal::EvaluationContext& )
    {
        return Val;
    }
    static T ConvertIn(const T& Val, const Internal::EvaluationContext&)
    {
        return Val;
    }
};
}//ns Marshal

namespace Internal
{
namespace Marshal
{
template <typename T>
class MakePlain
{
    public:
    typedef typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type Type;
};
template<typename T>
struct Type
{
    typedef typename ::Marshal::Type<typename MakePlain<T>::Type>::ToInternal ToInternal;
};
template<typename T>
struct Value
{
    typedef typename MakePlain<T>::Type Type;
    template <typename U>
    static Type ConvertOut(const U& Val,  Internal::EvaluationContext& EC)
    {
        return ::Marshal::Value<Type>::ConvertOut(Val, EC);
    }
    static Internal::Types::Object ConvertIn(const T& Val,  Internal::EvaluationContext& EC)
    {
        return Internal::Types::Object(::Marshal::Value<Type>::ConvertIn(Val, EC));
    }
    static ResolvedToken ConvertInNoObject(const T& Val,  Internal::EvaluationContext& EC)
    {
        return ::Marshal::Value<Type>::ConvertIn(Val, EC);
    }
};
}//ns Marshal
}//ns Internal
#endif // MARSHAL_HPP_INCLUDED
