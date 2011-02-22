#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "Tokens.hpp"
#include "Types.hpp"
namespace Internal
{
namespace Types
{
class Object
{
    public:
        /** Default constructor */
        Object(const ValueToken& Value):
            m_Value(Value),
            m_This(NullReference())
        {}
        /** Access m_Value
         * \return The current value of m_Value
         */
        ValueToken Value()const { return m_Value; }
        template <typename T>
        typename T::result_type Visit(T & StaticVisitor)
        {
            return boost::apply_visitor(StaticVisitor,m_Value);
        }
        template <typename T>
        typename T::result_type Visit(T && StaticVisitor) const
        {
            return boost::apply_visitor(StaticVisitor,m_Value);
        }
        void This( const Scope& NewThis )
        {
            m_This = NewThis;
        }
        Scope This() const
        {
            return m_This;
        }
    protected:
    private:
        ValueToken m_Value; //!< Member variable "m_Value"
        Scope m_This;
};
}
}
#endif // OBJECT_HPP
