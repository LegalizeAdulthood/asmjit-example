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

TEST(TestFormulaParse, capitalLetterInIdentifier)
{
    ASSERT_TRUE(formula::parse("A"));
}

TEST(TestFormulaParse, numberInIdentifier)
{
    ASSERT_TRUE(formula::parse("a1"));
}

TEST(TestFormulaParse, underscoreInIdentifier)
{
    ASSERT_TRUE(formula::parse("A_1"));
}

TEST(TestFormulaParse, invalidIdentifier)
{
    EXPECT_FALSE(formula::parse("1a"));
    EXPECT_FALSE(formula::parse("_a"));
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

TEST(TestFormulaEvaluate, setSymbolValue)
{
    const auto formula{formula::parse("a*a + b*b")};
    ASSERT_TRUE(formula);
    formula->set_value("a", 2.0);
    formula->set_value("b", 3.0);

    ASSERT_NEAR(13.0, formula->evaluate(), 1e-5);
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

TEST(TestAssembledFormulaEvaluate, add)
{
    const auto formula{formula::parse("1.2+1.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(2.4, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, subtract)
{
    const auto formula{formula::parse("1.5-2.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(-0.7, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, multiply)
{
    const auto formula{formula::parse("2.2*3.1")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(6.82, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, divide)
{
    const auto formula{formula::parse("13.76/4.3")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(3.2, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, avogadrosNumberDivide)
{
    const auto formula{formula::parse("6.02e23/2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(3.01e23, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, unaryNegate)
{
    const auto formula{formula::parse("--1.6")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(1.6, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, addAddadd)
{
    const auto formula{formula::parse("1.1+2.2+3.3")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(6.6, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, mulMulMul)
{
    const auto formula{formula::parse("2.2*2.2*2.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(10.648, formula->evaluate(), 1e-6);
}

TEST(TestAssembledFormulaEvaluate, addMulAdd)
{
    const auto formula{formula::parse("1.1+2.2*3.3+4.4")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->assemble());

    ASSERT_NEAR(12.76, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, one)
{
    const auto formula{formula::parse("1")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_EQ(1.0, formula->evaluate());
}

TEST(TestCompiledFormulaEvaluate, two)
{
    const auto formula{formula::parse("2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_EQ(2.0, formula->evaluate());
}

TEST(TestCompiledFormulaEvaluate, identifier)
{
    const auto formula{formula::parse("e")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(std::exp(1.0), formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, add)
{
    const auto formula{formula::parse("1.2+1.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(2.4, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, subtract)
{
    const auto formula{formula::parse("1.5-2.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(-0.7, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, multiply)
{
    const auto formula{formula::parse("2.2*3.1")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(6.82, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, divide)
{
    const auto formula{formula::parse("13.76/4.3")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(3.2, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, avogadrosNumberDivide)
{
    const auto formula{formula::parse("6.02e23/2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(3.01e23, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, unaryNegate)
{
    const auto formula{formula::parse("--1.6")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(1.6, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, addAddadd)
{
    const auto formula{formula::parse("1.1+2.2+3.3")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(6.6, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, mulMulMul)
{
    const auto formula{formula::parse("2.2*2.2*2.2")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(10.648, formula->evaluate(), 1e-6);
}

TEST(TestCompiledFormulaEvaluate, addMulAdd)
{
    const auto formula{formula::parse("1.1+2.2*3.3+4.4")};
    ASSERT_TRUE(formula);
    ASSERT_TRUE(formula->compile());

    ASSERT_NEAR(12.76, formula->evaluate(), 1e-6);
}
