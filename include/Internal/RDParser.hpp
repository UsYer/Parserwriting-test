#ifndef RDPARSER_H_
#define RDPARSER_H_

#include <deque>
#include <memory>

#include "Token.hpp"

namespace Internal{

	namespace AST{
		class Block;
	}

	enum class ParseSection{
		Block,
		Statement,
		IntegerLiteral,
		DoubleLiteral,
		StringLiteral,
		Literal,
		BinaryOp,
		UnaryOp,
		Namelist,
		Parlist,
		FuncBody,
		FunctionDef,
		Var,
		ExpressionList,
		Args,
		FunctionCall,
		PrefixExp,
		Expression,
	};

	class ParseContext final{

	public:
		ParseContext(std::deque<Token>& input, std::unique_ptr<AST::Block>& ast, long long& lineno);

		bool enter(ParseSection section);
		void leave();

		Token getToken() const;
		const Token& peekToken() const;
		bool swallowIfToken(TokenType token_type) const;
		bool isToken(TokenType token_type) const;
		bool hasInput() const;

	private:
		std::deque<Token>& input;
		std::deque<std::pair<ParseSection, int /* Token count when entering a section */>> section_stack; // used to detect left recursion. If the token count did not change between entering the same sections we have left recursion.
		std::unique_ptr<AST::Block>& ast;
		long long& lineno;
	};


	class RDParser final {
	public:
		RDParser();
		~RDParser();

		std::unique_ptr<AST::Block> parse(std::deque<Token> TokExpr);

	private:
	};
}
#endif // RDPARSER_H_