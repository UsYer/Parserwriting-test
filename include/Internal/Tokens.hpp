#ifndef TOKENS_HPP_INCLUDED
#define TOKENS_HPP_INCLUDED
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"
#include "../Parsable.hpp"
namespace Internal
{
class IEvaluable;
class IOperator;
class IFunction;
//class Parsable;
typedef boost::variant<long long,double,boost::shared_ptr<IOperator>,std::string,Parsable> UnparsedToken;
struct FuncCallerTag
    {};
typedef boost::variant<long long,double,boost::shared_ptr<IEvaluable>,std::string> Token;
typedef boost::variant<long long,double,std::string,CountedReference, NullReference, boost::shared_ptr<IFunction>> ValueToken; //Has to remain an IFunction because otherwise the funccaller couldn't operate on it
typedef boost::variant<long long,double> NumberToken;
typedef boost::variant<CountedReference,NullReference> CountedRefOrNull;
typedef boost::variant<Reference,NullReference> RefOrNull;
typedef boost::variant<CountedReference,NullReference,boost::shared_ptr<IFunction>> CountedRefOrNullOrFunc;
typedef boost::variant<long long,double,CountedReference,NullReference,boost::shared_ptr<IFunction>> ResolvedToken;
typedef ResolvedToken Member;
typedef boost::variant<boost::shared_ptr<IOperator>, boost::shared_ptr<IFunction>> Evaluables;
typedef boost::variant<boost::shared_ptr<IOperator>, FuncCallerTag> OperatorOrFuncCaller;
}

#endif // TOKENS_HPP_INCLUDED
