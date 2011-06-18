#ifndef GET_HPP_INCLUDED
#define GET_HPP_INCLUDED
#include "../Exceptions.hpp"
template<typename T>
struct Get : public boost::static_visitor<T>
{
    std::string m_ErrorMessage;
    Get():
        m_ErrorMessage("")
    {}
    Get(const std::string& ErrorMessage):
            m_ErrorMessage(ErrorMessage)
    {}
    template<typename U>
    T operator()(U) const
    {
        if( m_ErrorMessage.empty() )
            throw Exceptions::TypeException(std::string("Expected ") + Type<T>::Name() + "; Is " + Type<U>::Name());
        else
            throw Exceptions::TypeException(m_ErrorMessage);
    }
    T operator()(T Type) const
    {
        return Type;
    }
};
#endif // GET_HPP_INCLUDED
