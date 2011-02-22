#ifndef TABLETESTS_HPP_INCLUDED
#define TABLETESTS_HPP_INCLUDED
//Creation of tables:
TEST(ShortTableDef)
{
    std::cout << "---- TableTests ----\nShortTableDef\n";
    MP.Evaluate("test = (1,2,3)");
    long long result = Types::Object(MP.GlobalScope()["test"][0]);
    CHECK_EQUAL(1,result);
    result = Types::Object(MP.GlobalScope()["test"][1]);
    CHECK_EQUAL(2,result);
    result = Types::Object(MP.GlobalScope()["test"][2]);
    CHECK_EQUAL(3,result);
}
TEST(FunctionalTableDef)
{
    std::cout << "FunctionalTableDef\n";
    MP.Evaluate("test=Table(1,2,3)");
    long long result = Types::Object(MP.GlobalScope()["test"][0]);
    CHECK_EQUAL(1,result);
    result = Types::Object(MP.GlobalScope()["test"][1]);
    CHECK_EQUAL(2,result);
    result = Types::Object(MP.GlobalScope()["test"][2]);
    CHECK_EQUAL(3,result);
}
TEST(ShortTableDefInFuncCall)
{
    std::cout << "ShortTableDefInFuncCall\n";
    MP.Evaluate("Type((1,2))");
}
TEST(FunctionalTableDefInFuncCall)
{
    std::cout << "ShortTableDef\n";
    MP.Evaluate("Type(Table(1,2))");
}
//Internal table interaction:
TEST(ReadTableByIndexSimple)
{
    std::cout << "ReadTableByIndexSimple\n";
    MP.Evaluate("result = test[0]");
    long long result = Types::Object(MP.GlobalScope()["result"]);
    CHECK_EQUAL(1,result);
}
//Covers a bug with a misplaced .-operator. Code: t = Table(); t.S = Table(1,2,3); t.S[0] fails with t S [ 0 ] .
//pepped up a little to cover more complex acces of tables by an index
TEST(ReadTableByIndexComplex)
{
    std::cout << "ReadTableByIndexComplex\n";
    MP.Evaluate("t = Table()");
    MP.Evaluate("t.Scope = Table(1,2,3)");
    MP.Evaluate("result = t.Scope[1+1*2-1]");
    long long result = Types::Object(MP.GlobalScope()["result"]);
    CHECK_EQUAL(3,result);
}
TEST(CalculationWithTableAccesByIndex)
{
    std::cout << "CalculationWithTableAccesByIndex\n";
    MP.Evaluate("Math.Scope = Table(1,2,3)");
    MP.Evaluate("result = 5++ + Math.Scope[Math.Max(0,1)]");
    long long result = Types::Object(MP.GlobalScope()["result"]);
    CHECK_EQUAL(8,result);
}
TEST(WriteTableByIndexSimple)
{
    std::cout << "WriteTableByIndexSimple\n";
    MP.Evaluate("test[0] = 0");
    MP.Evaluate("result = test[0]");
    long long result = Types::Object(MP.GlobalScope()["result"]);
    CHECK_EQUAL(0,result);
}
TEST(ReadWriteTableByIndex)
{
    std::cout << "ReadWriteTableByIndex\n";
    MP.Evaluate("result = test[1] = test[0]");
    long long result = Types::Object(MP.GlobalScope()["result"]);
    CHECK_EQUAL(0,result);
}
//Userside table interaction
TEST(UserSideReadTableByIndex)
{
    std::cout << "UserSideReadTableByIndex\n";
    long long result = Types::Object(MP.GlobalScope()["test"][0]);
    CHECK_EQUAL(0,result);
}
TEST(UserSideWriteTableByIndex)
{
    std::cout << "UserSideWriteTableByIndex\n";
    MP.GlobalScope()["test"][0] = 1ll;
    long long result = Types::Object(MP.GlobalScope()["test"][0]);
    CHECK_EQUAL(1,result);
}
TEST(UserSideReadWriteTableByIndex)
{
    std::cout << "UserSideReadWriteTableByIndex\n";
    Types::Object test(MP.GlobalScope()["test"]);
    Types::Object result = test[1] = test[0];
    CHECK_EQUAL(1,static_cast<long long>(result));
}
//----
TEST(ComplexAssignment)
{
    std::cout << "ComplexAssignment\n";
    MP.Evaluate("test.eins = Math.Max");
}
//Covering wrong Tokenizing of the following expression. Does only happen when there are no arguments for Table()
//Gets tokenized as Table( instead of Table (
TEST(MemberAccesOnTempTable)
{
    std::cout << "MemberAccesOnTempTable\n";
    MP.Evaluate("Table().Id=1");
}

#endif // TABLETESTS_HPP_INCLUDED
