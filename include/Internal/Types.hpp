#ifndef TOKEN_H
#define TOKEN_H
#include <deque>
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"

namespace Internal
{
class IOperator;
class IFunction;
namespace Types
{
    typedef CountedReference Scope;
    class Object;
    typedef std::deque<Object> Stack;
    typedef boost::shared_ptr<IOperator> Operator;
    typedef boost::shared_ptr<IFunction> Function;
}
}// ns Internal
/*
namespace Types
{

}*/

#endif // TOKEN_H
