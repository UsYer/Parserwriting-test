#ifndef VALUE_H_
#define VALUE_H_

#include <memory>
#include "boost/variant.hpp"

#include "../Parsable.hpp"
#include "../../libs/ustring/ustring.h"

namespace Internal{

	class IOperator;

	typedef boost::variant<long long, double, utf8::ustring, std::shared_ptr<IOperator>, std::string, Parsable> Value;

}
#endif