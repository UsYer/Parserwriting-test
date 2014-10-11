#ifndef TOKENS_HPP_INCLUDED
#define TOKENS_HPP_INCLUDED
#include <boost/variant.hpp>
#include <memory>
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"
#include "../Parsable.hpp"
#include "../../libs/ustring/ustring.h"
namespace Internal
{
class IEvaluable;
class IOperator;
class IFunction;

//typedef boost::variant<long long, double, utf8::ustring> LiteralToken;

typedef boost::variant<long long, double, utf8::ustring,std::shared_ptr<IOperator>,std::string,Parsable> UnparsedToken;
struct FuncCallerTag
    {};
// Token after Parsing:
typedef boost::variant<long long, double, utf8::ustring,std::shared_ptr<IEvaluable>,std::string> ParsedToken;

// Token produced after parsing, used at runtime:
typedef boost::variant<long long, double, utf8::ustring,std::string,CountedReference, NullReference, std::shared_ptr<IFunction>> ValueToken; //Has to remain an IFunction because otherwise the funccaller couldn't operate on it

// Convenience tokens used throughout the runtime:
typedef boost::variant<long long,double> NumberToken;
typedef boost::variant<CountedReference,NullReference> CountedRefOrNull;
typedef boost::variant<Reference,NullReference> RefOrNull;
typedef boost::variant<CountedReference,NullReference,std::shared_ptr<IFunction>> CountedRefOrNullOrFunc;
typedef boost::variant<long long, double, utf8::ustring,CountedReference,NullReference,std::shared_ptr<IFunction>> ResolvedToken;
typedef ResolvedToken Member;
typedef boost::variant<std::shared_ptr<IOperator>, std::shared_ptr<IFunction>> Evaluables;
typedef boost::variant<std::shared_ptr<IOperator>, FuncCallerTag> OperatorOrFuncCaller;
}

#endif // TOKENS_HPP_INCLUDED
