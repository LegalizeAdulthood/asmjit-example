#include <formula/formula.h>

#include <gtest/gtest.h>

TEST(TestFormulaParse, constant)
{
    ASSERT_TRUE(formula::parse("1"));
}

TEST(TestFormulaParse, add)
{
    ASSERT_TRUE(formula::parse("1+2"));
}

TEST(TestFormulaParse, subtract)
{
    ASSERT_TRUE(formula::parse("1-2"));
}

TEST(TestFormulaParse, multiply)
{
    ASSERT_TRUE(formula::parse("1*2"));
}

TEST(TestFormulaParse, divide)
{
    ASSERT_TRUE(formula::parse("1*2"));
}

TEST(TestFormulaParse, multiplyAdd)
{
    ASSERT_TRUE(formula::parse("1*2+4"));
}

TEST(TestFormulaParse, parenthesisExpr)
{
    ASSERT_TRUE(formula::parse("1*(2+4)"));
}

TEST(TestFormulaParse, unaryMinus)
{
    ASSERT_TRUE(formula::parse("-(1)"));
}

TEST(TestFormulaParse, unaryPlus)
{
    ASSERT_TRUE(formula::parse("+(1)"));
}
