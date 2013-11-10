#ifndef STATESAVER_HPP_INCLUDED
#define STATESAVER_HPP_INCLUDED
#include <stack>
#include <cassert>
template<typename T>
class StateSaver
{
public:
    StateSaver() = delete;
    StateSaver(T InitialState)
    {
        StateStack.push(InitialState);
    }
    operator const T&() const
    {
        assert( !StateStack.empty() );
        return StateStack.top();
    }
    T operator =(T NewState)
    {
        StateStack.push(NewState);
        return StateStack.top();
    }
	bool operator == (T rhs) const
	{
		return StateStack.top() == rhs;

	}
	bool operator != (T rhs) const
	{
		return !((*this) == rhs);
	}
    void Restore()
    {
        if( StateStack.size() > 1 ) //Preserve initial state
            StateStack.pop();
    }
    void Reset()
    {
        while( StateStack.size() > 1 ) //Preserve initial state
            StateStack.pop();
    }
private:
    std::stack<T> StateStack;
};

#endif // STATESAVER_HPP_INCLUDED
