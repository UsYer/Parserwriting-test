#ifndef FUNCTIONCALLTESTS_HPP_INCLUDED
#define FUNCTIONCALLTESTS_HPP_INCLUDED

TEST(MissingArgsInFunccall)
{
    std::cout << "MissingArgsInFunccall\n";
    CHECK_THROW(MP.Evaluate("Type(,)"),Exceptions::ParseError);
}
TEST(ArgSepAfterOpeningBracket)
{
    std::cout << "ArgSepAfterOpeningBracket\n";
    CHECK_THROW(MP.Evaluate("Type(,1)"),Exceptions::ParseError);
}
TEST(VariadicFuncCallWithChangingArgcount)
{
    std::cout << "VariadicFuncCallWithChangingArgcount\n";
    long long Result = MP.Evaluate("Math.Max(1,2)");
    CHECK_EQUAL(Result,2);
    CHECK_THROW( Result = MP.Evaluate("Math.Max()"), Exceptions::RuntimeException);
}
TEST(ReturnValues)
{
    std::cout << "ReturnValues\n";
    long long Result = MP.Evaluate("func() return 5 end()");
    CHECK_EQUAL( Result, 5);
}
TEST(ReturnValuesAnonFunc)
{
    std::cout << "ReturnValuesAnonFunc\n";
    MP.Evaluate("func f() return 5 end");
    long long Result = MP.Evaluate("f()");
    CHECK_EQUAL( Result, 5);
}
TEST(ReturnArg)
{
    std::cout << "ReturnArg\n";
    MP.Evaluate("func f(arg) return 5+5^arg++ end");
    long long Result = MP.Evaluate("f(2)");
    CHECK_EQUAL( Result, 130);
}
TEST(ReturnFunc)
{
    std::cout << "ReturnFunc\n";
    MP.Evaluate("func f() return func(eins,zwei) return (eins,zwei) end end");
    long long Result = MP.Evaluate("f()(1,2)[1]");
    CHECK_EQUAL( Result, 2);
}

#endif // FUNCTIONCALLTESTS_HPP_INCLUDED
