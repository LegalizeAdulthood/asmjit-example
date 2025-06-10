#include "formula/formula.h"

#include <asmjit/core.h>
#include <asmjit/x86.h>
#include <boost/parser/parser.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace bp = boost::parser;

namespace formula
{

namespace
{

using SymbolTable = std::map<std::string, double>;

class Node
{
public:
    virtual ~Node() = default;

    virtual double evaluate(const SymbolTable &symbols) const = 0;

    virtual bool assemble(asmjit::x86::Assembler &assem, const SymbolTable &symbols) const = 0;
};

class NumberNode : public Node
{
public:
    NumberNode(double value) :
        m_value(value)
    {
    }
    ~NumberNode() override = default;

    double evaluate(const SymbolTable & /*symbols*/) const override
    {
        return m_value;
    }

    bool assemble(asmjit::x86::Assembler &assem, const SymbolTable &symbols) const override
    {
        assem.mov(asmjit::x86::eax, m_value);
        return true;
    }

private:
    double m_value{};
};

const auto make_number = [](auto &ctx) { return std::make_shared<NumberNode>(bp::_attr(ctx)); };

class IdentifierNode : public Node
{
public:
    IdentifierNode(std::string value) :
        m_value(std::move(value))
    {
    }
    ~IdentifierNode() override = default;

    double evaluate(const SymbolTable &symbols) const override
    {
        if (const auto &it = symbols.find(m_value); it != symbols.end())
        {
            return it->second;
        }
        return 0.0;
    }

    bool assemble(asmjit::x86::Assembler &assem, const SymbolTable &symbols) const override
    {
        const double value{evaluate(symbols)};
        assem.mov(asmjit::x86::eax, value);
        return true;
    }

private:
    std::string m_value;
};

const auto make_identifier = [](auto &ctx) { return std::make_shared<IdentifierNode>(bp::_attr(ctx)); };

class UnaryOpNode : public Node
{
public:
    UnaryOpNode(char op, std::shared_ptr<Node> operand) :
        m_op(op),
        m_operand(std::move(operand))
    {
    }
    ~UnaryOpNode() override = default;

    double evaluate(const SymbolTable &symbols) const override
    {
        if (m_op == '+')
        {
            return m_operand->evaluate(symbols);
        }
        if (m_op == '-')
        {
            return -m_operand->evaluate(symbols);
        }
        throw std::runtime_error(std::string{"Invalid unary prefix operator '"} + m_op + "'");
    }

    bool assemble(asmjit::x86::Assembler &assem, const SymbolTable &symbols) const override
    {
        return false;
    }

private:
    char m_op;
    std::shared_ptr<Node> m_operand;
};

const auto make_unary_op = [](auto &ctx)
{ return std::make_shared<UnaryOpNode>(std::get<0>(bp::_attr(ctx)), std::get<1>(bp::_attr(ctx))); };

class BinaryOpNode : public Node
{
public:
    BinaryOpNode() = default;
    BinaryOpNode(std::shared_ptr<Node> left, char op, std::shared_ptr<Node> right) :
        m_left(std::move(left)),
        m_op(op),
        m_right(std::move(right))
    {
    }
    ~BinaryOpNode() override = default;

    double evaluate(const SymbolTable &symbols) const override;

    bool assemble(asmjit::x86::Assembler &assem, const SymbolTable &symbols) const override
    {
        return false;
    }

private:
    std::shared_ptr<Node> m_left;
    char m_op;
    std::shared_ptr<Node> m_right;
};

double BinaryOpNode::evaluate(const SymbolTable &symbols) const
{
    const double left = m_left->evaluate(symbols);
    const double right = m_right->evaluate(symbols);
    if (m_op == '+')
    {
        return left + right;
    }
    if (m_op == '-')
    {
        return left - right;
    }
    if (m_op == '*')
    {
        return left * right;
    }
    if (m_op == '/')
    {
        return left / right;
    }
    throw std::runtime_error(std::string{"Invalid binary operator '"} + m_op + "'");
}

const auto make_binary_op = [](auto &ctx)
{
    return std::make_shared<BinaryOpNode>(
        std::get<0>(bp::_attr(ctx)), std::get<1>(bp::_attr(ctx)), std::get<2>(bp::_attr(ctx)));
};

const auto make_binary_op_seq = [](auto &ctx)
{
    auto left = std::get<0>(bp::_attr(ctx));
    for (const auto &op : std::get<1>(bp::_attr(ctx)))
    {
        left = std::make_shared<BinaryOpNode>(left, std::get<0>(op), std::get<1>(op));
    }
    return left;
};

using Expr = std::shared_ptr<Node>;

// Terminal parsers
//const auto number = bp::double_;
const auto alpha = bp::char_('a', 'z');
const auto digit = bp::char_('0', '9');
const auto alnum = alpha | digit;
const auto identifier = bp::lexeme[alpha >> *alnum];

// Grammar rules
bp::rule<struct NumberTag, Expr> number = "number";
bp::rule<struct IdentifierTag, Expr> variable = "variable";
bp::rule<struct ExprTag, Expr> expr = "expression";
bp::rule<struct TermTag, Expr> term = "multiplicative term";
bp::rule<struct FactorTag, Expr> factor = "additive factor";
bp::rule<struct UnaryOpTag, Expr> unary_op = "unary operator";

const auto number_def = bp::double_[make_number];
const auto variable_def = identifier[make_identifier];
const auto unary_op_def = (bp::char_("-+") >> factor)[make_unary_op];
const auto factor_def = number | variable | '(' >> expr >> ')' | unary_op;
const auto term_def = (factor >> *(bp::char_("*/") >> factor))[make_binary_op_seq];
const auto expr_def = (term >> *(bp::char_("+-") >> term))[make_binary_op_seq];

BOOST_PARSER_DEFINE_RULES(number, variable, expr, term, factor, unary_op);

class ParsedFormula : public Formula
{
public:
    ParsedFormula(std::shared_ptr<Node> ast) :
        m_ast(ast)
    {
        m_symbols["e"] = std::exp(1.0);
        m_symbols["pi"] = std::atan2(0.0, -1.0);
    }
    ~ParsedFormula() override = default;

    double evaluate() override;

    bool assemble() override;

private:
    SymbolTable m_symbols;
    std::shared_ptr<Node> m_ast;
    std::function<double(const SymbolTable &)> m_function;
};

double ParsedFormula::evaluate()
{
    return m_function ? m_function(m_symbols) : m_ast->evaluate(m_symbols);
}

bool ParsedFormula::assemble()
{
    asmjit::JitRuntime runtime;
    asmjit::CodeHolder code;
    code.init(runtime.environment(), runtime.cpuFeatures());
    asmjit::x86::Assembler assem(&code);
    if (!m_ast->assemble(assem, m_symbols))
    {
        std::cerr << "Failed to compile AST\n";
        return false;
    }
    assem.ret();

    if (const asmjit::Error err = runtime.add(&m_function, &code))
    {
        std::cerr << "Failed to compile formula: " << asmjit::DebugUtils::errorAsString(err) << '\n';
        return false;
    }

    return true;
}

} // namespace

std::shared_ptr<Formula> parse(std::string_view text)
{
    Expr ast;

    try
    {
        if (auto success = bp::parse(text, expr, bp::ws, ast/*, bp::trace::on*/); success && ast)
        {
            return std::make_shared<ParsedFormula>(ast);
        }
    }
    catch (const bp::parse_error<std::string_view::const_iterator> &e)
    {
        std::cerr << "Parse error: " << e.what() << '\n';
    }
    return {};
}

} // namespace formula
