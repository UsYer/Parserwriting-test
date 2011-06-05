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
    MP.Evaluate("Math.Max(1,2)");
    long long Result;
    CHECK_THROW( Result = MP.Evaluate("Math.Max()"), std::logic_error);
}
#endif // FUNCTIONCALLTESTS_HPP_INCLUDED
