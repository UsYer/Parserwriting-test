
#include "..\include\Internal\RDParser.hpp"
#include "..\include\Exceptions.hpp"
#include "..\include\Internal\AST\Node.hpp"
#include "..\libs\ustring\ustring.h"
#include "../include/Internal/Utilities.hpp"

using namespace Internal;

RDParser::RDParser()
{
}


RDParser::~RDParser()
{
}


ParseContext::ParseContext(std::deque<Token>& input, std::unique_ptr<AST::Block>& ast, long long& lineno) :
		input{input},
		ast{ast},
		lineno{lineno}
{}

bool ParseContext::enter(ParseSection section){
	//auto it = std::find_if(section_stack.rbegin(), section_stack.rend(), [section](const std::pair<ParseSection, int>& item){ return item.first == section; });
	//if (it != section_stack.rend()) {
	//	if (it->second == input.size()) { // no tokens were consumed during the last time we entered this section -> left recursion detected.
	//		return false;
	//	}
	//}

	for (int i = section_stack.size() - 1; i >= 0; i--) {
		if (section_stack[i].first == section && section_stack[i].second == input.size()) {
			return false;
		}
	}

	section_stack.push_back(std::make_pair(section, input.size()));
	return true;
}

void ParseContext::leave(){
	section_stack.pop_back();
}

struct ScopedSectionGuard{
	ScopedSectionGuard(ParseContext& context, ParseSection section):
		context{ context }
	{
		entered = context.enter(section);
	}
	~ScopedSectionGuard(){
		if (entered) {
			context.leave();
		}
	}
	ParseContext& context;
	bool entered = false;
};

Token ParseContext::getToken() const {
		Token token = input.back();
		if (token.type() == TokenType::EOL) {
			++lineno;
		}
		input.pop_back();
		return token;
}
const Token& ParseContext::peekToken() const {
	return input.back();
}
bool ParseContext::isToken(TokenType token_type) const{
	return hasInput() ? (peekToken().type() == token_type) : false;
}
bool ParseContext::swallowIfToken(TokenType token_type) const{
	if (isToken(token_type)) {
		getToken();
		return true;
	} else return false;
}
bool ParseContext::hasInput() const {
	return !input.empty();
}

struct ParseInfo{
	ParseInfo(bool ok = true, const utf8::ustring& message = "", std::unique_ptr<AST::Node>&& node = nullptr) :
		ok{ ok },
		message{ message },
		node{std::move(node)}
	{}
	ParseInfo(const ParseInfo&) = delete;
	ParseInfo& operator=(ParseInfo&& other){
		ok = other.ok;
		message = other.message;
		node = std::move(other.node);
		return *this;
	}
	ParseInfo(ParseInfo&& other):
		ok{ other.ok },
		message{ std::move(other.message) },
		node{ std::move(other.node) }
	{

	}
	bool ok;
	utf8::ustring message;
	std::unique_ptr<AST::Node> node;
};

// --- free parser function prototypes: ---
ParseInfo parseBlock(ParseContext ctx);
ParseInfo parseStatement(ParseContext& context);
ParseInfo parseIntegerLiteral(ParseContext& context);
ParseInfo parseDoubleLiteral(ParseContext& context);
ParseInfo parseStringLiteral(ParseContext& context);
ParseInfo parseLiteral(ParseContext& context);
ParseInfo parseBinaryOp(ParseContext& context);
ParseInfo parseUnaryOp(ParseContext& context);
ParseInfo parseNamelist(ParseContext& context);
ParseInfo parseParlist(ParseContext& context);
ParseInfo parseFuncBody(ParseContext& context);
ParseInfo parseFunctionDef(ParseContext& context);
ParseInfo parseVar(ParseContext& context);
ParseInfo parseExpressionList(ParseContext& context);
ParseInfo parseArgs(ParseContext& context);
ParseInfo parseFunctionCall(ParseContext& context);
ParseInfo parsePrefixExp(ParseContext& context);
ParseInfo parseExpression(ParseContext& context);



std::unique_ptr<AST::Block> RDParser::parse(std::deque<Token> TokExpr){

	auto ast = std::make_unique<AST::Block>();
	long long lineno = 1;
	ParseContext parse_context{ TokExpr, ast, lineno };
	
	ParseInfo info = parseBlock(parse_context);
	if (info.ok) {
		ast->addChild(std::move(info.node));
	} else {
		throw Exceptions::ParseError(info.message, Exceptions::ParseError::Location{ "fileBla", lineno });
	}
	return ast;
}

ParseInfo parseBlock(ParseContext ctx){
	auto block_exp = std::make_unique<AST::Block>();
	ParseInfo block_info;
	while (ctx.hasInput()) {
		block_info = parseStatement(ctx);
		if (block_info.ok) {
			block_exp->addChild(std::move(block_info.node));
		} 
		// TODO: remove parse expression
		block_info = parseExpression(ctx);
		if (block_info.ok) {
			block_exp->addChild(std::move(block_info.node));
		}
	}
	return{ block_info.ok, "", std::move(block_exp) };
}

ParseInfo parseStatement(ParseContext& context){

	return{false};
}

ParseInfo parseIntegerLiteral(ParseContext& context){
	/*Token token = context.getToken();
	if (token.type() != TokenType::Integer) {
		return{ false };
	}*/
	return{ context.isToken(TokenType::Integer) };
}

ParseInfo parseDoubleLiteral(ParseContext& context){
	/*Token token = context.getToken();
	if (token.type() != TokenType::Double) {
		return{ false };
	}*/
	return{ context.isToken(TokenType::Double) };
}

ParseInfo parseStringLiteral(ParseContext& context){
	/*Token token = context.getToken();
	if (token.type() != TokenType::String) {
		return{ false };
	}*/
	return{ context.isToken(TokenType::String) };
}

ParseInfo parseLiteral(ParseContext& context){
	if (context.isToken(TokenType::Integer) || context.isToken(TokenType::Double) || context.isToken(TokenType::String)) {
		auto token = context.getToken();
		auto literal_exp = std::make_unique<AST::Literal>(token.getValue());
		return{ true, "", std::move(literal_exp)};
	}
	return{ false, "expected literal." };
}

ParseInfo parseBinaryOp(ParseContext& context){
	ScopedSectionGuard scg{ context, ParseSection::BinaryOp };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}

	Value value;

	// Parse lhs expression
	ScopedSectionGuard scg_lhs{ context, ParseSection::Expression };
	if (!scg_lhs.entered) {
		return{ false, "left recursion detected." };
	}
	ParseInfo lhs_info = parseExpression(context);
	if (!lhs_info.ok) {
		return{ false };
	}

	// parse op
	if (context.isToken(TokenType::OpBinary)){
		value = context.getToken().getValue();
	} else {
		return{ false, "expected binary operator" };
	}


	// parse rhs expression
	ScopedSectionGuard scg_rhs{ context, ParseSection::Expression };
	if (!scg_rhs.entered) {
		return{ false, "left recursion detected." };
	}
	ParseInfo rhs_info = parseExpression(context);
	if (!rhs_info.ok) {
		return{ false };
	}
	auto bin_expr = std::make_unique<AST::BinaryOp>(value);
	bin_expr->addChild(std::move(lhs_info.node));
	bin_expr->addChild(std::move(rhs_info.node));
	return{ true, "", std::move(bin_expr) };
}

///Used in Parser
struct PrintValueNoResolve : public boost::static_visitor<std::string>
{

	std::string operator()(long long val)const
	{
		return std::to_string(val);
	}
	std::string operator()(double val)const
	{
		return std::to_string(val);
	}
	std::string operator()(const utf8::ustring& val)const{
		return "\"" + val.raw() + "\"";
	}
	std::string operator()(const Parsable& P) const{
		return "Parsable";
	}
	std::string operator()(const std::shared_ptr<IOperator>& op)const{
		return "operator";
	}
	std::string operator()(const std::string& s)const
	{
		return s;
	}
};

ParseInfo parseUnaryOp(ParseContext& context){
	ScopedSectionGuard scg{ context, ParseSection::UnaryOp };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}

	if (context.isToken(TokenType::OpUnaryPrefix)) {
		Token token = context.getToken();
		ParseInfo info = parseExpression(context);
		if (info.ok) {
			auto unary_exp = std::make_unique<AST::UnaryOp>(token.getValue());
			unary_exp->addChild(std::move(info.node));
			return{ true, "", std::move(unary_exp) };
		} else {
			Value tok = token.getValue();
			PrintValueNoResolve visitor;
			return{ false, "expected expression after unary operator " }; //+ boost::apply_visitor(visitor, tok) };
		}
	} else {
		return{ false };
	}
}

ParseInfo parseNamelist(ParseContext& context){
	TokenType last_token = TokenType::None;
	

	if (!context.isToken(TokenType::Identifier)) {
		return{ false };
	} else {
		auto namelist_exp = std::make_unique<AST::NameList>();
		namelist_exp->addChild(std::make_unique<AST::Identifier>(context.getToken().getValue()));
		while (context.hasInput()) {
			if (context.isToken(TokenType::Identifier) && (last_token == TokenType::None || last_token == TokenType::Comma)) {
				auto id_token = context.getToken();
				namelist_exp->addChild(std::make_unique<AST::Identifier>(id_token.getValue()));
				last_token = TokenType::Identifier;
			} else if (context.isToken(TokenType::Comma) && last_token == TokenType::Identifier) {
				context.getToken(); // swallow comma, it's not needed

				last_token = TokenType::Comma;
			} else {
				break;
			}
		}
		return{ true, "", std::move(namelist_exp) };
	}
	
}

ParseInfo parseParlist(ParseContext& context){
	// parlist is the same as name list right now
	return parseNamelist(context);
}

ParseInfo parseFuncBody(ParseContext& context){
	if (context.swallowIfToken(TokenType::OpeningParenthese)) {
		

		// parse optional parameter list
		ParseInfo parlist_info = parseParlist(context);

		if (context.swallowIfToken(TokenType::ClosingParenthese)) {
			
			ParseInfo block_info = parseBlock(context);
			if (block_info.ok) {
				if (context.swallowIfToken(TokenType::KeywordEnd)) {
					
					auto funcbody_exp = std::make_unique<AST::FuncBody>();
					funcbody_exp->addChild(std::move(parlist_info.node));
					funcbody_exp->addChild(std::move(block_info.node));

					return{ true, "", std::move(funcbody_exp) };
				} else {
					return{ false, "\"end\" is missing in function definition" };
				}

			} else {
				return block_info;
			}
		} else {
			return{ false, "expected closing parentheses" };
		}

	} else {
		return{ false, "expected opening parentheses after keyword \"func\"" };
	}
}

ParseInfo parseFunctionDef(ParseContext& context){
	ScopedSectionGuard scg{ context, ParseSection::FunctionDef };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}
	
	if (context.swallowIfToken(TokenType::KeywordFunc)) {
		
		ParseInfo info = parseFuncBody(context);
		if (info.ok) {
			auto funcdef_exp = std::make_unique<AST::FuncDefAnon>();
			funcdef_exp->addChild(std::move(info.node));
			return{ true, "", std::move(funcdef_exp) };
		} else {
			return info;
		}
	} else {
		return{ false };
	}
}


//var =  Name 
//		| prefixexp "[" exp "]"
//		| prefixexp "." Name .
ParseInfo parseVar(ParseContext& context){
	auto var_expression = std::make_unique<AST::VarExp>();
	if (context.isToken(TokenType::Identifier)) {
		var_expression->addChild(std::make_unique<AST::Identifier>(context.getToken().getValue()));
		return{ true, "", std::move(var_expression) };
	}
	ParseInfo prefix_info = parsePrefixExp(context);
	if (prefix_info.ok) {
		if (context.swallowIfToken(TokenType::OpeningBracket)) {
			ParseInfo exp_info = parseExpression(context);
			if (exp_info.ok) {
				if (context.swallowIfToken(TokenType::ClosingBracket)) {
					auto tableacces_expression = std::make_unique<AST::TableAccesIndex>();
					tableacces_expression->addChild(std::move(prefix_info.node));
					tableacces_expression->addChild(std::move(exp_info.node));
					var_expression->addChild(std::move(tableacces_expression));
					return{ true, "", std::move(var_expression) };
				} else {
					return{ false, "expected closing bracket after expression" };
				}
			} else {
				return exp_info;
			}
		} else if (context.swallowIfToken(TokenType::Dot)) {
			if (context.isToken(TokenType::Identifier)) {
				auto tableacces_expression = std::make_unique<AST::TableAccesMember>();
				tableacces_expression->addChild(std::move(prefix_info.node));
				tableacces_expression->addChild(std::make_unique<AST::Identifier>(context.getToken().getValue()));
				var_expression->addChild(std::move(tableacces_expression));
				return{ true, "", std::move(var_expression) };
			} else {
				return{ false, "expected identifier after '.'" };
			}
		}
	}
	return{ false };
}

ParseInfo parseExpressionList(ParseContext& context){
	TokenType last_token = TokenType::None;

	ParseInfo expr_info = parseExpression(context);
	if (!expr_info.ok) {
		return expr_info;
	} else {
		auto exprlist_expr = std::make_unique<AST::ExpressionList>();
		exprlist_expr->addChild(std::move(expr_info.node));
		while (context.hasInput()) {
			if (context.swallowIfToken(TokenType::Comma)) {
				expr_info = parseExpression(context);
				if (expr_info.ok) {
					exprlist_expr->addChild(std::move(expr_info.node));
				} else {
					return{ false, "expected expression after ','" };
				}
			} else {
				break;
			}
		}
		return{ true, "", std::move(exprlist_expr) };
	}

}

ParseInfo parseArgs(ParseContext& context){
	if (context.swallowIfToken(TokenType::OpeningBracket)) {
		ParseInfo exp_info = parseExpressionList(context);
		if (exp_info.node != nullptr && !exp_info.ok) { // if there is an expression list then it has to be parsed correctly
			return exp_info;
		}
		if (context.swallowIfToken(TokenType::ClosingBracket)) {
			return{ true, "", std::move(exp_info.node) };
		} else {
			return{ false, "expected closing bracket after expression" };
		}
	} else {
		return{ false };
	}
}

// functioncall = prefixexp args .
ParseInfo parseFunctionCall(ParseContext& context){
	ScopedSectionGuard scg{ context, ParseSection::FunctionCall };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}
	ParseInfo prefixexp_info = parsePrefixExp(context);
	if (prefixexp_info.ok) {
		ParseInfo args_info = parseArgs(context);
		if (args_info.ok) {
			auto funccall_exp = std::make_unique<AST::FuncCall>();
			funccall_exp->addChild(std::move(prefixexp_info.node));
			funccall_exp->addChild(std::move(args_info.node));
			return{ true, "", std::move(funccall_exp) };
		}
	}
	return{ false };
}

//prefixexp = var
//				| functioncall
//				| "(" exp ")" .
ParseInfo parsePrefixExp(ParseContext& context){
	ScopedSectionGuard scg{ context, ParseSection::PrefixExp };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}
	ParseInfo prefix_info = parseVar(context);
	if (prefix_info.ok) {
		auto prefix_exp = std::make_unique<AST::PrefixExp>();
		prefix_exp->addChild(std::move(prefix_info.node));
		return{ true, "", std::move(prefix_exp) };
	} 
	prefix_info = parseFunctionCall(context);
	if (prefix_info.ok) {
		auto prefix_exp = std::make_unique<AST::PrefixExp>();
		prefix_exp->addChild(std::move(prefix_info.node));
		return{ true, "", std::move(prefix_exp) };
	} else if (context.swallowIfToken(TokenType::OpeningParenthese)) {
		ParseInfo expr_info = parseExpression(context);
		if (expr_info.ok) {
			if (context.isToken(TokenType::ClosingParenthese)) {
				context.getToken();
				auto prefix_exp = std::make_unique<AST::PrefixExp>();
				prefix_exp->addChild(std::move(expr_info.node));
				return{ true, "", std::move(prefix_exp) };
			}
		}
	}
	return{ false };
}

ParseInfo parseExpression(ParseContext& context){
	/*ScopedSectionGuard scg{ context, ParseSection::Expression };
	if (!scg.entered) {
		return{ false, "left recursion detected." };
	}*/
	
	ParseInfo expr_info = parseBinaryOp(context);
	if (expr_info.ok) {
		return expr_info;
	} 
	expr_info = parseUnaryOp(context);
	if (expr_info.ok) {
		return expr_info;
	}
	expr_info = parseFunctionDef(context);
	if (expr_info.ok) {
		return expr_info;
	}
	expr_info = parsePrefixExp(context);
	if (expr_info.ok) {
		return expr_info;
	}
	expr_info = parseLiteral(context);
	if (expr_info.ok) {
		return expr_info;
	}
	/*expr_info = parseTableConstructor(context);
	if (expr_info.ok) {
		return expr_info;
	}*/
	return {false};
}