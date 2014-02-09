#include <iostream>
#include "../include/MariusParser.hpp"
#include <UnitTest++.h>
using namespace std;
MariusParser MP;
TEST(UnknownIdentifer)
{
    std::cout << "UnknownIdentifer\n";
    CHECK_THROW(MP.Evaluate("unknown+5"), Exceptions::RuntimeException);
}
TEST(SimplePlainAssignment)
{
    std::cout << "SimplePlainAssignment\n";
    MP.Evaluate("test=1");
    long long result = MP.GlobalScope()["test"];
    CHECK_EQUAL(1,result);
}
TEST(PlainAssignment1)
{
    std::cout << "PlainAssignment1\n";
    MP.Evaluate("test1=1");
    long long result = MP.GlobalScope()["test1"];
    CHECK_EQUAL(1,result);
}
TEST(PlainAssignment2)
{
    std::cout << "PlainAssignment2\n";
    MP.Evaluate("_2=2");
    long long result = MP.GlobalScope()["_2"];
    CHECK_EQUAL(2,result);
}
TEST(PlainAssignment3)
{
    std::cout << "PlainAssignment3\n";
    MP.Evaluate("test_3=3");
    long long result = MP.GlobalScope()["test_3"];
    CHECK_EQUAL(3,result);
}
TEST(ValueFollowsValue)
{
    std::cout << "ValueFollowsValue" << std::endl;
    CHECK_THROW(MP.Evaluate("5 test"),Exceptions::ParseError);
}
TEST(WeirdSyntax1)
{
    std::cout << "WeirdSyntax1" << std::endl;
    CHECK_THROW(MP.Evaluate("MC,[0]55"),Exceptions::ParseError);
}
TEST(WeirdSyntax2)
{
    std::cout << "WeirdSyntax2" << std::endl;
    CHECK_THROW(MP.Evaluate("MC.[0]"),Exceptions::ParseError);
}
TEST(AssignmentToLiteral)
{
    std::cout << "AssignmentToLiteral" << std::endl;
    CHECK_THROW(MP.Evaluate("5=1"),Exceptions::ParseError);
}
//Checks for distinction of unary and binary minus
TEST(UnaryMinusFirstToken)
{
    std::cout << "UnaryMinusFirstToken\n";
    long long result = MP.Evaluate("-5");
    CHECK_EQUAL(-5,result);
}
TEST(UnaryMinusFirstTokenWithBrackets)
{
    std::cout << "UnaryMinusFirstTokenWithBrackets\n";
    long long result = MP.Evaluate("-(5 - -2)");
    CHECK_EQUAL(-7,result);
}
TEST(UnaryAndBinaryMinusSpace)
{
    std::cout << "UnaryAndBinaryMinusSpace\n";
    long long result = MP.Evaluate("5 - -5");
    CHECK_EQUAL(10,result);
}
TEST(UnaryAndBinaryMinusNoSpace)
{
    std::cout << "UnaryAndBinaryMinusNoSpace\n";
    long long result = MP.Evaluate("5--5");
    CHECK_EQUAL(10,result);
}
TEST(UnaryAndBinaryMinusSpaceBetween)
{
    std::cout << "UnaryAndBinaryMinusSpaceBetween\n";
    long long result = MP.Evaluate("5- -5");
    CHECK_EQUAL(10,result);
}
TEST(UnaryMinusInExponent)
{
    std::cout << "UnaryMinusInExponent\n";
    double result = MP.Evaluate("5^-(2- -5)");
    CHECK_CLOSE(1.28e-5,result,0.00000001);
}
//Cover Crash of the parser when there is a closingbracket mismatch
TEST(CatchClosingBracketMismatch)
{
    std::cout << "CatchClosingBracketMismatch\n";
    CHECK_THROW(MP.Evaluate("Type())"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("T[]]"),Exceptions::ParseError);
}
TEST(ChangeInternalValue)
{
    std::cout << "ChangeInternalValue\n";
    MP.Evaluate("internal=1");
    Types::Object Obj(MP.GlobalScope()["internal"]);
    CHECK_EQUAL(1,static_cast<long long>(Obj));
    MP.GlobalScope()["internal"] = 2ll;
    Types::Object Obj2 = MP.GlobalScope()["internal"];
    CHECK_EQUAL(2,static_cast<long long>(Obj2));
}
TEST(ChangeInternalValueWithObject)
{
    std::cout << "ChangeInternalValueWithObject\n";
    CHECK_EQUAL(2,static_cast<long long>(MP.GlobalScope()["internal"]));
    long long test = MP.GlobalScope()["test"];
    CHECK_EQUAL(1,test);//should be 1 from the SimplePlainAssignmentTest
    MP.GlobalScope()["internal"] = test;
    CHECK_EQUAL(1,static_cast<long long>(MP.GlobalScope()["internal"]));
}
TEST(CreateInternalValueFromOutside)
{
    std::cout << "CreateInternalValueFromOutside\n";
    MP.GlobalScope()["created_from_outside"] = 1ll;
    long long created_from_outside = MP.GlobalScope()["created_from_outside"];
    CHECK_EQUAL(1,created_from_outside);
}
TEST(ToughFuncCall)
{
    std::cout << "ToughFuncCall\n";
    MP.Evaluate("result = Math.Max(Math.Max(Math.Pi,4),1)");
    double result = MP.GlobalScope()["result"];
    CHECK_EQUAL(4,result);
}
TEST(ToughCalculation)
{
    std::cout << "ToughCalculation\n";
    MP.Evaluate("pi = Math.Pi");
    MP.Evaluate("e = Math.E");
    MP.Evaluate("sqrt = Math.Sqrt");
    double result = MP.Evaluate("(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*"
                "(((-pi)+(-pi)-(-9)*(6*5))/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)"
                "+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/((e+(-2)+(-e)*((((-3)*9+(-e)))+"
                "(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-e)*(-e)))"
                "-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))"
                "*(((3+2-8))*(7+6+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))"
                "+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)"
                "-pi))/4+(pi)*(-9)))))))+(-pi)");
    CHECK_CLOSE(-12.2301,result,0.001);
}
TEST(RightParsingOfBracketsWithArgSep)
{
    std::cout << "RightParsingOfBracketsWithArgSep\n";
    MP.Evaluate("result = Math.Max((5+5)^(2+1),5)");
    long long result = MP.GlobalScope()["result"];
    CHECK_EQUAL(1000,result);
}
TEST(NegativeExponent)
{//Make sure negative exponents yield the correct result
    std::cout << "NegativeExponent\n";
    double result = MP.Evaluate("5*10^-3");
    CHECK_CLOSE(0.005,result,0.000001);
}
TEST(RightParsingOfMultipleBrackets)
{
    std::cout << "RightParsingOfMultipleBrackets\n";
    MP.Evaluate("result = ((5+5))+5");
    long long result = MP.GlobalScope()["result"];
    CHECK_EQUAL(15,result);
}
long long ExposedFunc(long long One, long long Two)
{
    std::cout << One << " + " << Two << std::endl;
    return One + Two;
}
TEST(SimpleExposedFunc)
{
    MP.RegisterFunction("Exposed Function","EF",&ExposedFunc);
    MP.Evaluate("test = EF(2,2)");
    long long result = MP.GlobalScope()["test"];
    CHECK_EQUAL(4,result);
}
void TesterMarshal(Types::Function& Func)
{
    long long Result = Func(2ll,2ll);
    std::cout << "In TestMarshal, result of Func(2,2): " << Result << "\n";
}
TEST(MarshalFunction)
{
    MP.RegisterFunction("","FMT",TesterMarshal);
    MP.Evaluate("FMT(EF)");
}
void TesterObjectParamter(const Types::Object& MaxFunc)
{
    double Result = MaxFunc(3ll,3.1);
    std::cout << "In TestObjectParamter, result of MaxFunc(3,3.1): " << Result << "\n";
}
TEST(ObjectParameter)
{
    MP.RegisterFunction("","TOP",TesterObjectParamter);
    MP.Evaluate("TOP(Math.Max)");
}
Internal::NullReference ReturnNullFunc(long long, double)
{
    return Internal::NullReference();
}
/*
This Test checks if an exception is thrown when the user expects a certain type from a function call but gets a nullreference instead.
See TesterObjectParamter function which is used as the origin of the exception
*/
TEST(NullRefAsReturnValue)
{
    MP.RegisterFunction("","NullRefAsReturnValue",ReturnNullFunc);

    CHECK_THROW(MP.Evaluate("TOP(NullRefAsReturnValue)"), Exceptions::RuntimeException);
}
Types::Object TesterReturnMarshal(const Types::Object& Val)
{
    return Val;
}
TEST(ReturnMarshal)
{
    MP.RegisterFunction("","TRM",TesterReturnMarshal);
    MP.Evaluate("PassedFunc = TRM(Math.Max)");
    MP.Evaluate("PassedFunc(1,2)");
}
#include "ExceptionTests.hpp"
#include "TableTests.hpp"
#include "FunctionDefTests.hpp"
#include "FunctionCallTests.hpp"
#include "SpecialMethodTests.hpp"
#include "StringTests.hpp"
int main()
{
    return UnitTest::RunAllTests();
}
