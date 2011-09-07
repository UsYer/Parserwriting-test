#ifndef OBJECT_HPP_INCLUDED
#define OBJECT_HPP_INCLUDED
#include "Internal/Utilities.hpp"
namespace Internal
{
    class EvaluationContext;
}
namespace Types
{
class Function;
class Object
{
    struct TableAcces : public boost::static_visitor<const Internal::CountedReference&>
    {
        template<typename T>
        const Internal::CountedReference& operator()(const T&)const
        {
            throw std::logic_error("Type is not a table");
        }
        const Internal::CountedReference& operator()(const Internal::CountedReference& Ref)const
        {
            return Ref;
        }
    };
    template<typename KeyType>
    class TableProxy
    {
        const Internal::CountedReference m_TableRef;
        //Attention: Made the key to reference on a const object, because otherwise there would be an error with array initialisation in case the key is a static string like "test"
        const KeyType& m_Key;
        Internal::EvaluationContext& m_EC;
        public:
        TableProxy(const Internal::CountedReference& Ref, const KeyType& Key, Internal::EvaluationContext& EC):
            m_TableRef(Ref),
            m_Key(Key),
            m_EC(EC)
        {}
        TableProxy& operator=(const Object& Value)
        {
            (*m_TableRef)[m_Key] = Value.m_Value;
            return *this;
        }
        TableProxy& operator=(const TableProxy& Value)
        {
            (*m_TableRef)[m_Key] = (*Value.m_TableRef)[Value.m_Key];
            return *this;
        }
        template<typename T>
        TableProxy& operator=(const T& Value)
        {
            (*m_TableRef)[m_Key] = Value;
            return *this;
        }
        template<typename T>
        TableProxy<T> operator[](const T& Key) const
        {
            //Internal::CountedReference Ref(boost::apply_visitor(TableAcces(),m_Value));
            return TableProxy<T>(boost::apply_visitor(TableAcces(),(*m_TableRef)[m_Key]),Key,m_EC);
        }
        operator Object()
        {
            return Object(m_EC, (*m_TableRef)[m_Key]);
        }
        template <typename T>
        operator T() const;
    };
    public:
        // TODO (Marius#6#): Make Object copyable (EC ref is the problem. Would it hurt to use a pointer instead?)
        /** Default constructor */
        Object(Internal::EvaluationContext& EC,const Internal::Member& Value):
            m_Value(Value),
            m_EC(EC)
        {}
        template <typename T>
        typename T::result_type Visit(T & StaticVisitor)
        {
            return boost::apply_visitor(StaticVisitor,m_Value);
        }
        template <typename T>
        typename T::result_type Visit(const T & StaticVisitor) const
        {
            return boost::apply_visitor(StaticVisitor,m_Value);
        }
        template <typename T>
        operator T() const;

        template<typename T>
        TableProxy<T> operator[](const T& Key)
        {
            //Internal::CountedReference Ref(boost::apply_visitor(TableAcces(),m_Value));
            return TableProxy<T>(boost::apply_visitor(TableAcces(),m_Value),Key,m_EC);
        }
        template <typename... T>
        Object operator()(T... Args) const
        {
            return Function(boost::apply_visitor(Internal::Utilities::Get<std::shared_ptr<Internal::IFunction>>(),m_Value),m_EC.Scope(),m_EC.MC)(Args...);
        }
    protected:
    private:
        Internal::Member m_Value; //!< Member variable "m_Value"
        Internal::EvaluationContext& m_EC;
};
}//ns Types

//Placed the methods that use Marshal here because otherwise we'd be stuck in a cyclic dependency between Object.hpp and Marshal.hpp
#include "Internal/Marshal.hpp"
template <typename KeyType>
template <typename T>
::Types::Object::TableProxy<KeyType>::operator T() const
{
    using namespace Internal::Marshal;
    return Value<T>::ConvertOut(boost::apply_visitor(Internal::Utilities::Get<typename Type<T>::ToInternal>(),(*m_TableRef)[m_Key]),m_EC);
}
template <typename T>
::Types::Object::operator T() const
{
    using namespace Internal::Marshal;
    return Value<T>::ConvertOut(boost::apply_visitor(Internal::Utilities::Get<typename Type<T>::ToInternal>(),m_Value),m_EC);
}
namespace Marshal
{
template<>
struct Value<Types::Object>
{
    struct GetValue : public boost::static_visitor<Internal::Member>
    {
        template<typename T>
        Internal::Member operator()(const T& Val) const
        {
            return Val;
        }
    };
    static Types::Object ConvertOut(const Internal::ResolvedToken& Val, Internal::EvaluationContext& EC)
    {
        return Types::Object(EC,Val);
    }
    static Internal::Member ConvertIn(const Types::Object& Obj, Internal::EvaluationContext& EC)
    {
        return Obj.Visit(GetValue());
    }
};
}
//Specialising for userside Object, because it can hold every internal type
namespace Internal{ namespace Utilities{
//Workaround for compiler error: ":: cannot appear as the first token in a template list"
//But we need to explicitly specify that we want the userside Types::Object
typedef ::Types::Object UserObject;
template<>
struct Get<UserObject> : public boost::static_visitor< ResolvedToken>
{
    std::string m_ErrorMessage;
    Get():
        m_ErrorMessage("")
    {}
    Get(const std::string& ErrorMessage):
            m_ErrorMessage(ErrorMessage)
    {}

     ResolvedToken operator()(const ResolvedToken& Tok) const
    {
        return Tok;
    }
};
}//ns Utilities
}//ns Internal

#endif // OBJECT_HPP_INCLUDED
