#ifndef IS_HPP_INCLUDED
#define IS_HPP_INCLUDED
namespace Internal
{
template<typename T>
struct Is
{
    //static_visitor compliance:
    typedef bool result_type;

    template<typename U>
    bool operator()(const U&) const
    {
        return false;
    }
    bool operator()(const T&) const
    {
        return true;
    }
};
}
#endif // IS_HPP_INCLUDED
