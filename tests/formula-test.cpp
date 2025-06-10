#include <formula/formula.h>

#include <gtest/gtest.h>

#include <cmath>

TEST(TestFormulaParse, constant)
{
    ASSERT_TRUE(formula::parse("1"));
}

TEST(TestFormulaParse, identifier)
{
    ASSERT_TRUE(formula::parse("z2"));
}

TEST(TestFormulaParse, parenExpr)
{
    ASSERT_TRUE(formula::parse("(z)"));
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
    ASSERT_TRUE(formula::parse("1/2"));
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

TEST(TestFormulaParse, unaryMinusNegativeOne)
{
    ASSERT_TRUE(formula::parse("--1"));
}

TEST(TestFormulaParse, addAddAdd)
{
    ASSERT_TRUE(formula::parse("1+1+1"));
}

TEST(TestFormulaEvaluate, one)
{
    ASSERT_EQ(1.0, formula::parse("1")->evaluate());
}

TEST(TestFormulaEvaluate, two)
{
    ASSERT_EQ(2.0, formula::parse("2")->evaluate());
}

TEST(TestFormulaEvaluate, add)
{
    ASSERT_EQ(2.0, formula::parse("1+1")->evaluate());
}

TEST(TestFormulaEvaluate, unaryMinusNegativeOne)
{
    ASSERT_EQ(1.0, formula::parse("--1")->evaluate());
}

TEST(TestFormulaEvaluate, multiply)
{
    ASSERT_EQ(6.0, formula::parse("2*3")->evaluate());
}

TEST(TestFormulaEvaluate, divide)
{
    ASSERT_EQ(3.0, formula::parse("6/2")->evaluate());
}

TEST(TestFormulaEvaluate, addMultiply)
{
    const auto formula{formula::parse("1+3*2")};
    ASSERT_TRUE(formula);

    ASSERT_EQ(7.0, formula->evaluate());
}

TEST(TestFormulaEvaluate, multiplyAdd)
{
    const auto formula{formula::parse("3*2+1")};
    ASSERT_TRUE(formula);

    ASSERT_EQ(7.0, formula->evaluate());
}

TEST(TestFormulaEvaluate, addAddAdd)
{
    const auto formula{formula::parse("1+1+1")};
    ASSERT_TRUE(formula);

    ASSERT_EQ(3.0, formula->evaluate());
}

TEST(TestFormulaEvaluate, mulMulMul)
{
    const auto formula{formula::parse("2*2*2")};
    ASSERT_TRUE(formula);

    ASSERT_EQ(8.0, formula->evaluate());
}

TEST(TestFormulaEvaluate, twoPi)
{
    const auto formula{formula::parse("2*pi")};
    ASSERT_TRUE(formula);

    ASSERT_NEAR(6.28318, formula->evaluate(), 1e-5);
}

TEST(TestAssembledFormulaEvaluate, one)
{
    const auto formula{formula::parse("1")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_EQ(1.0, formula->evaluate());
}

TEST(TestAssembledFormulaEvaluate, two)
{
    const auto formula{formula::parse("2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_EQ(2.0, formula->evaluate());
}

TEST(TestAssembledFormulaEvaluate, identifier)
{
    const auto formula{formula::parse("e")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(std::exp(1.0), formula->evaluate(), 1e-6);
}
