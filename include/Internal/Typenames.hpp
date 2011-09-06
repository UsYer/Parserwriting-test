#ifndef TYPENAMES_HPP_INCLUDED
#define TYPENAMES_HPP_INCLUDED
namespace Internal
{
template<typename T>
struct Type
{
    static const char* Name()
    {
        return "unknown";
    }
};
template<>
struct Type<std::shared_ptr<IOperator>>
{
    static const char* Name()
    {
        return "operator";
    }
};
template<>
struct Type<std::shared_ptr<IFunction>>
{
    static const char* Name()
    {
        return "function";
    }
};
template<>
struct Type<long long>
{
    static const char* Name()
    {
        return "long long";
    }
};
template<>
struct Type<double>
{
    static const char* Name()
    {
        return "double";
    }
};
template<>
struct Type<std::string>
{
    static const char* Name()
    {
        return "identifier";
    }
};
template<>
struct Type<CountedReference>
{
    static const char* Name()
    {
        return "reference";
    }
};
template<>
struct Type<NullReference>
{
    static const char* Name()
    {
        return "nullreference";
    }
};
}
#endif // TYPENAMES_HPP_INCLUDED
