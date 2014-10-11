#ifndef TOKEN_H_
#define TOKEN_H_

#include <memory>
#include "Value.hpp"

namespace Internal{
	enum class TokenType{
		OpUnaryPostfix, // ++
		OpUnaryPrefix, // - not !
		OpBinary, // + - * / ^ , 
		Integer,
		Double,
		Number = Integer | Double,
		Comma,
		Dot,
		String,
		OpeningParenthese,
		ClosingParenthese,
		OpeningBracket,
		ClosingBracket,
		KeywordCatch,
		KeywordEnd,
		KeywordFunc,
		KeywordNull,
		KeywordThrow,
		Identifier,
		EOL,
		None,
		Unrecognized
	};

	class Token
	{
	public:
		Token(TokenType token_type, const Value& value);
		~Token();

		TokenType type() const {
			return m_token_type;
		}
		Value getValue() const;

	private:
		TokenType m_token_type;
		Value m_value;
	};
}
#endif //TOKEN_H_