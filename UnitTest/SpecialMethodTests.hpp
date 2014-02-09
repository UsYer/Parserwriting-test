#ifndef SPECIALMETHODTESTS_HPP_INCLUDED
#define SPECIALMETHODTESTS_HPP_INCLUDED

TEST(AtCallTestNoArg)
{
    std::cout << "AtCallTestNoArg\n";
    MP.Evaluate("t = Table()");
    MP.Evaluate("t.@call = func() return 1337 end");
    long long result = MP.Evaluate("t()");
    CHECK_EQUAL(result, 1337);
}
TEST(AtCallTestArgs)
{
    std::cout << "AtCallTestArgs\n";
    MP.Evaluate("t = Table()");
    MP.Evaluate("t.@call = func(one,two) return Math.Max(two,one) end");
    double result = MP.Evaluate("t(Math.E, Math.Pi)");
    CHECK_CLOSE(3.142, result, 0.001);
}
#endif // SPECIALMETHODTESTS_HPP_INCLUDED
