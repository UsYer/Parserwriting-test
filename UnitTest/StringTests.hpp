#ifndef STRINGTESTS_HPP_INCLUDED
#define STRINGTESTS_HPP_INCLUDED

TEST(CreateEmptyStringTest)
{

    utf8::ustring str = MP.Evaluate("\"\"");
    CHECK_EQUAL(str, "");
    CHECK_EQUAL(str.length(), 0);
    CHECK_EQUAL(str.bytes(), 0);
}

TEST(Utf8StringLengthTest)
{

    utf8::ustring str = MP.Evaluate("\"ä\"");
    CHECK_EQUAL(str, "ä");
    CHECK_EQUAL(str.length(), 1);
    CHECK_EQUAL(str.bytes(), 2);
}

TEST(AppendStringTest)
{

    utf8::ustring str = MP.Evaluate("\"one\" + \"two\" + \"three\"");
    CHECK_EQUAL(str, "onetwothree");
    CHECK_EQUAL(str.length(), 11);
    CHECK_EQUAL(str.bytes(), 11);
}

#endif // STRINGTESTS_HPP_INCLUDED
