#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <memory>

#include "../Value.hpp"

namespace Internal{
	namespace AST{
		class Node
		{
		public:
			Node(){}
			virtual ~Node(){}

			long long getLineNo() const{
				return m_line_no;
			}
			void addChild(std::unique_ptr<Node>&& child){
				m_children.push_back(std::forward<std::unique_ptr<Node>>(child));
			}
		private:
			long long m_line_no;
			std::vector<std::unique_ptr<Node>> m_children;
		};

		class Block : public Node{
		public:
			Block() :Node()
			{}
			virtual ~Block(){}
		};

		class Expression : public Node{

		public:
			Expression();
			Expression(Value val);
			virtual ~Expression(){}
			Value getValue() const;

		private:
			Value m_value;
		};

		class PrefixExp : public Expression{

		};

		class ExpressionList : public Expression{

		};

		class VarExp : public Expression{

		};

		class Statement : public Node{

		};

		class NameList : public Node{
		public:
			virtual ~NameList(){}
		};

		class TableAccesIndex : public Expression{
			static const int table_idx = 0;
			static const int arg_idx = 1;
		};

		class TableAccesMember : public Expression{
			static const int table_idx = 0;
			static const int arg_idx = 1;
		};

		// -- operators
		class UnaryOp : public Expression{
		public:
			UnaryOp(Value value);
		};

		class BinaryOp : public Expression{
		public:
			BinaryOp(Value value);
		};

		class Literal : public Expression {
		public:
			Literal(Value value);
		};

		class Identifier : public Expression{
		public:
			Identifier(Value value);
		};

		class Term : public Expression {
		public:

		private:

		};

		class Factor : public Expression {

		};

		// -- Function related
		class FuncDefAnon : public Expression {
		public:
		};

		class FuncDefNamed : public Statement {

		};

		class FuncBody : public Node {
			static const int parlist_idx = 0;
			static const int block_idx = 1;
		public:
			virtual ~FuncBody(){}
		};

		class FuncCall : public Expression{
			static const int func_idx = 0;
			static const int args_idx = 1;
		};

		// -- Exception Handling
		class Throw : public Statement{

		};
		class TryCatch : public Statement {

		};
	}
}
#endif NODE_H_