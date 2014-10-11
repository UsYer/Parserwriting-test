
#include "..\include\Internal\AST\Node.hpp"

using namespace Internal;

//Node::Node()
//{
//}
//
//
//Node::~Node()
//{
//}

AST::Expression::Expression() :
m_value{}
{}

AST::Expression::Expression(Value val):
m_value{val}
{}

Value AST::Expression::getValue() const {
	return m_value;
}

AST::UnaryOp::UnaryOp(Value value) :
Expression{ value }
{}

AST::BinaryOp::BinaryOp(Value value):
	Expression{ value }
{}

AST::Literal::Literal(Value value) :
Expression{ value }
{}

AST::Identifier::Identifier(Value value) :
Expression{ value }
{}