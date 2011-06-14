#ifndef EXCEPTIONTESTS_HPP_INCLUDED
#define EXCEPTIONTESTS_HPP_INCLUDED

TEST(InternalToNativeException)
{
    std::cout << "InternalToNativeException\n";
    CHECK_THROW(MP.Evaluate("throw RuntimeException()"),Exceptions::RuntimeException);
}
TEST(ThrowNoException)
{
    std::cout << "ThrowNoException\n";
    try
    {
        MP.Evaluate("throw (1,2)");
    }
    catch( Exceptions::RuntimeException& e)
    {
        CHECK(e.TypeId() == 2); //TypeId 2 == TypeException because throw expects an exception type
    }
}
TEST(ThrowCatchNoArg)
{
    std::cout << "ThrowCatchNoArg\n";
    MP.Evaluate("t= 0");
    MP.Evaluate("try throw RuntimeException() end catch() __PARENT__.t = 1 end");
    long long t = MP.GlobalScope()["t"];
    CHECK_EQUAL(1,t);
}
TEST(ThrowCatchArg)
{
    std::cout << "ThrowCatchArg\n";
    MP.Evaluate("t= 0");
    MP.Evaluate("try throw RuntimeException() end catch(ex) __PARENT__.t = ex.TypeId end");
    long long t = MP.GlobalScope()["t"];
    CHECK_EQUAL(1,t);
}

void NativeThrow()
{
    throw Exceptions::TypeException("testing conversion from native to internal exceptions");
}
TEST(NativeToInternalException)
{
    std::cout << "NativeToInternalException" << std::endl;
    MP.RegisterFunction("NativeThrow","NativeThrow",&NativeThrow);
    MP.Evaluate("try NativeThrow() end catch(ex) __PARENT__.t = ex.TypeId end");
    long long t = MP.GlobalScope()["t"];
    CHECK_EQUAL(2,t);
}
TEST(ThrowInFunc)
{
    std::cout << "ThrowInFunc" << std::endl;
    MP.Evaluate("func f() throw NameException() end");
    MP.Evaluate("try f() end catch(ex) __PARENT__.t = ex.TypeId end");
    long long t = MP.GlobalScope()["t"];
    CHECK_EQUAL(3,t);
}
TEST(ThrowInFuncNoHandle)
{
    std::cout << "ThrowInFuncNoHandle" << std::endl;
    MP.Evaluate("func f() throw NameException() end");
    try
    {
        MP.Evaluate("f()");
    }
    catch(Exceptions::RuntimeException& e)
    {
        CHECK_EQUAL(3,e.TypeId());
    }
}
#endif // EXCEPTIONTESTS_HPP_INCLUDED
