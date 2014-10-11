#ifndef OPERATORS_H_INCLUDED
#define OPERATORS_H_INCLUDED
#include <cmath>
#include "../IToken.hpp" //For MinusToken, otherwise compiler error in Parser.cpp
#include "../Parsable.hpp"
#include "Utilities.hpp"
#include "Is.hpp"
#include "../Exceptions.hpp"
namespace Internal
{
class AssignmentOp : public IOperator
{
public:
    AssignmentOp():IOperator("Assignment operator","=",-10,ArityType::Binary,AssociativityType::Right)
    {}
    void Eval(EvaluationContext& EC) override;
};

void ParseAssignment(ParserContext& PC);


class PlusOp : public IOperator
{

public:
    PlusOp():IOperator("Plus","+",10,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};
class BinaryMinusOp : public IOperator
{
public:
    BinaryMinusOp():IOperator("Binary minus","-",10,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};
class UnaryMinusOp : public IOperator
{
public:
    UnaryMinusOp():IOperator("Unary minus","-",30,ArityType::UnaryPrefix,AssociativityType::Right)
    {}
	void Eval(EvaluationContext& EC) override;
};

class MinusToken : public IToken
{
public:
    MinusToken():
        IToken("-")
    {}
	virtual LastCharType Tokenize(TokenizeContext& TC) const override;
};
class MultiOp : public IOperator
{
    
public:
    MultiOp():IOperator("Multiplication","*",20,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class DivOp : public IOperator
{
public:
    DivOp():IOperator("Division","/",20,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class ExponentOp : public IOperator
{
public:
    ExponentOp():IOperator("Exponentiation","^",30,ArityType::Binary,AssociativityType::Right)
    {}
	void Eval(EvaluationContext& EC) override;
};

class ModuloOp : public IOperator
{
public:
    ModuloOp():IOperator("Modulo","%",20,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class TestOp : public IOperator
{
public:
    TestOp():IOperator("TestOp","++",30,ArityType::UnaryPostfix,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class AccesOp : public IOperator
{
public:
    AccesOp():IOperator("Accesoperator",".",50,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class LessOp : public IOperator
{
    
public:
    LessOp():IOperator("Lessoperator","<",5,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class GreaterOp : public IOperator
{
public:
    GreaterOp():IOperator("Greateroperator",">",5,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

class NotOp : public IOperator
{
public:
    NotOp():IOperator("Notoperator","!",30,ArityType::UnaryPrefix,AssociativityType::Right)
    {}
	void Eval(EvaluationContext& EC) override;
};


class IndexOpeningBracket : public IOperator
{

public:
    IndexOpeningBracket():IOperator("Index Opening Bracket","[",50,ArityType::UnaryPrefix,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};

void ParseIndexOpeningBracket(ParserContext& PC);

class IndexOpeningBracketToken : public IToken
{
public:
    IndexOpeningBracketToken():
        IToken("[")
    {

    }
	virtual LastCharType Tokenize(TokenizeContext& TC) const override;
};
class IndexClosingBracket : public IOperator
{

public:
    IndexClosingBracket():IOperator("Index Closing Bracket","]",50,ArityType::UnaryPostfix,AssociativityType::Left)
    {}
    void Eval(EvaluationContext&) override
    {
        //throw std::logic_error("Bracket missmatch");
    }
};


class TableOp : public IOperator
{
    
public:
    TableOp():IOperator("Table operator",",",-15,ArityType::Binary,AssociativityType::Left)
    {}
	void Eval(EvaluationContext& EC) override;
};
void ParseTableOp(ParserContext& PC);

class OpeningBracket : public IOperator
{
protected:
    
public:
    OpeningBracket():IOperator("","(",50,ArityType::UnaryPostfix,AssociativityType::Left) //Precedence doesn't matter because bracket ops receive special treatment
    {}
	void Eval(EvaluationContext& EC) override;
};
class OpeningBracketFuncCall : public OpeningBracket
{
    
public:
    OpeningBracketFuncCall():
        OpeningBracket()
    {}
	void Eval(EvaluationContext& EC) override;
};
class OpeningBracketNoFuncCall : public OpeningBracket
{
public:
    OpeningBracketNoFuncCall():
        OpeningBracket()
    {}
	void Eval(EvaluationContext& EC) override;
};

void ParseOpeningBracket(ParserContext& PC);

class OpeningBracketToken : public IToken
{
public:
    OpeningBracketToken():
        IToken("(")
    {

    }
	virtual LastCharType Tokenize(TokenizeContext& TC) const override;
};
class ClosingBracket : public IOperator
{
public:
    ClosingBracket():IOperator("Closing Bracket",")",-100,ArityType::UnaryPostfix,AssociativityType::None)
    {}
	void Eval(EvaluationContext& EC) override;
};
void ParseClosingBracket(ParserContext& PC);

class ClosingBracketToken : public IToken
{
public:
    ClosingBracketToken():
        IToken(")")
    {

    }
	virtual LastCharType Tokenize(TokenizeContext& TC) const override;
};

class ThrowOp : public IOperator
{
public:
    ThrowOp():
        IOperator("","throw", -15,ArityType::UnaryPrefix,AssociativityType::Right)
    {
    }
	void Eval(EvaluationContext& EC) override;
};

}//ns Internal
#endif // OPERATORS_H_INCLUDED
