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
#endif // FUNCTIONCALLTESTS_HPP_INCLUDED
