
#include "../../MemoryManagment/include/MemoryController.hpp"
#include "../../MemoryManagment/include/Reference.hpp"

#include "..\include\Internal\Token.hpp"
using namespace Internal;

Token::Token(TokenType token_type, const Value& value) :
m_token_type{ token_type },
m_value{value}
{
}


Token::~Token()
{
}

Value Token::getValue() const {
	return m_value;
}