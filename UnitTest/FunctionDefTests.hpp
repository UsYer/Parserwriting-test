#ifndef funcDEFTESTS_HPP_INCLUDED
#define funcDEFTESTS_HPP_INCLUDED

TEST(AnonFuncDef)
{
    std::cout << "---- FuncDefTests ----\nAnonFuncDef\n";
    MP.Evaluate("fun = func() end");
}
TEST(AnonFuncDefWithArgs)
{
    std::cout << "AnonFuncDefWithArgs\n";
    MP.Evaluate("fun = func(one,two)  end");
}
TEST(NamedFuncDef)
{
    std::cout << "NamedFuncDef\n";
    MP.Evaluate("func f()  end");
}
TEST(NamedFuncDefWithArgs)
{
    std::cout << "NamedFuncDefWithArgs\n";
    MP.Evaluate("func f(one,two)  end");
}
TEST(NoNamedFuncDefInFunccall)
{
    std::cout << "NoNamedFuncDefInFunccall\n";
    CHECK_THROW(MP.Evaluate("Type(func f()  end)"),std::logic_error);
}
TEST(AnonFuncDefInFunccall)
{
    std::cout << "AnonFuncDefInFunccall\n";
    MP.Evaluate("Type(func ()  end)");
}
TEST(FuncWithCallsToOtherScopes)
{
    std::cout << "FuncWithCallsToOtherScopes\n";
    MP.Evaluate("func f() Type(Math.Max(1,2.1)) end");
    MP.Evaluate("f()");
}
//Syntaxchecks
TEST(MissingArgList)
{
    std::cout << "--- FuncDefTests Syntax ----\nMissingArgList\n";
    CHECK_THROW(MP.Evaluate("func"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f"),Exceptions::ParseError);
}
TEST(NumberLiteralsInArgList)
{
    std::cout << "NumberLiteralsInArgList\n";
    CHECK_THROW(MP.Evaluate("func(1)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f(1)"),Exceptions::ParseError);
}
TEST(NumberLiteralsInArgListWithNormalArgs)
{
    std::cout << "NumberLiteralsInArgListWithNormalArgs\n";
    CHECK_THROW(MP.Evaluate("func(a,1)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func(1,a)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f(a,1)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f(1,a)"),Exceptions::ParseError);
}
TEST(NumberLiteralsInArgListWithinArgs)
{
    std::cout << "NumberLiteralsInArgListWithinArgs\n";
    MP.Evaluate("func(a1) end");
    CHECK_THROW(MP.Evaluate("func(1a)"),Exceptions::ParseError);
    MP.Evaluate("func f(a1) end");
    CHECK_THROW(MP.Evaluate("func f(1a)"),Exceptions::ParseError);
}
TEST(DoubleArgNames)
{
    std::cout << "DoubleArgNames\n";
    CHECK_THROW(MP.Evaluate("func(a,a)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func(b,a,a)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f(a,a)"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f(b,a,a)"),Exceptions::ParseError);
}
TEST(MissingEnd)
{
    std::cout << "MissingEnd\n";
    CHECK_THROW(MP.Evaluate("func()"),Exceptions::ParseError);
    CHECK_THROW(MP.Evaluate("func f()"),Exceptions::ParseError);
}
TEST(SpareEnd)
{
    std::cout << "SpareEnd\n";
    CHECK_THROW(MP.Evaluate("end"),Exceptions::ParseError);
}
#endif // funcDEFTESTS_HPP_INCLUDED
