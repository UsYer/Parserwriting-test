#ifndef STACK_HPP_INCLUDED
#define STACK_HPP_INCLUDED
#include <deque>
namespace Internal
{
template<typename T, typename ContainerType = std::deque<t>>
class Stack
{
    public:
    typedef T ValueType;
    typedef typename ContainerType::iterator Iterator;
    typedef typename ContainerType::const_iterator ConstIterator;
    typedef typename ContainerType::reverse_iterator ReverseIterator;
    typedef typename ContainerType::const_reverse_iterator ConstReverseIterator;
    Iterator Begin()
    {
        return Container.begin();
    }
    Iterator End()
    {
        return Container.end();
    }
    Iterator RBegin()
    {
        return Container.rbegin();
    }
    Iterator REnd()
    {
        return Container.rend();
    }
    private:
    ContainerType Container;
};
}
#endif // STACK_HPP_INCLUDED
