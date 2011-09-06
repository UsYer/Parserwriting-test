#ifndef TOKENS_HPP_INCLUDED
#define TOKENS_HPP_INCLUDED
#include <boost/variant.hpp>
#include <memory>
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"
#include "../Parsable.hpp"
namespace Internal
{
class IEvaluable;
class IOperator;
class IFunction;
//class Parsable;
typedef boost::variant<long long,double,std::shared_ptr<IOperator>,std::string,Parsable> UnparsedToken;
struct FuncCallerTag
    {};
typedef boost::variant<long long,double,std::shared_ptr<IEvaluable>,std::string> Token;
typedef boost::variant<long long,double,std::string,CountedReference, NullReference, std::shared_ptr<IFunction>> ValueToken; //Has to remain an IFunction because otherwise the funccaller couldn't operate on it
typedef boost::variant<long long,double> NumberToken;
typedef boost::variant<CountedReference,NullReference> CountedRefOrNull;
typedef boost::variant<Reference,NullReference> RefOrNull;
typedef boost::variant<CountedReference,NullReference,std::shared_ptr<IFunction>> CountedRefOrNullOrFunc;
typedef boost::variant<long long,double,CountedReference,NullReference,std::shared_ptr<IFunction>> ResolvedToken;
typedef ResolvedToken Member;
typedef boost::variant<std::shared_ptr<IOperator>, std::shared_ptr<IFunction>> Evaluables;
typedef boost::variant<std::shared_ptr<IOperator>, FuncCallerTag> OperatorOrFuncCaller;
}

#endif // TOKENS_HPP_INCLUDED
