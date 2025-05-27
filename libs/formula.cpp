#include "formula/formula.h"

#include <boost/parser/parser.hpp>


#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace bp = boost::parser;

namespace formula
{

namespace
{

// AST node types
struct BinaryOp;
struct UnaryOp;

using ExprNode = std::variant<double, // Number literal
    std::string,                      // Variable
    std::shared_ptr<BinaryOp>,        // Binary operation
    std::shared_ptr<UnaryOp>          // Unary operation
    >;

struct BinaryOp
{
    ExprNode left;
    char op;
    ExprNode right;
};

struct UnaryOp
{
    char op;
    ExprNode operand;
};

// Helper functions to create AST nodes
ExprNode make_binary(ExprNode left, char op, ExprNode right)
{
    auto node = std::make_shared<BinaryOp>();
    node->left = std::move(left);
    node->op = op;
    node->right = std::move(right);
    return node;
}

ExprNode make_unary(char op,ExprNode operand)
{
    auto node = std::make_shared<UnaryOp>();
    node->op = op;
    node->operand = std::move(operand);
    return node;
}

/*
        factor = number | identifier | '(' >> expr >> ')' |
            ('-' >> factor)[([](auto &ctx) { return make_unary('-', bp::_attr(ctx, 0)); })];

        term = factor >>
            *(('*' >> factor)[([](auto &ctx) { return make_binary(bp::_val(ctx), '*', bp::_attr(ctx, 0)); })] |
                ('/' >> factor)[([](auto &ctx) { return make_binary(bp::_val(ctx), '/', bp::_attr(ctx, 0)); })]);

        expr =
            term >> *(('+' >> term)[([](auto &ctx) { return make_binary(bp::_val(ctx), '+', bp::_attr(ctx, 0)); })] |
                        ('-' >> term)[([](auto &ctx) { return make_binary(bp::_val(ctx), '-', bp::_attr(ctx, 0)); })]);
 */
// Terminal parsers
const auto number = bp::double_;
const auto alpha = bp::char_('a', 'z');
const auto digit = bp::char_('0', '9');
const auto alnum = alpha | digit;
const auto identifier = bp::lexeme[alpha >> *alnum];

// Grammar rules
bp::rule<struct expr> expr = "expression";
bp::rule<struct term> term = "term";
bp::rule<struct factor> factor = "factor";

const auto factor_def = number | identifier | '(' >> expr >> ')' | '-' >> factor | '+' >> factor;
const auto term_def = factor >> *('*' >> factor | '/' >> factor);
const auto expr_def = term >> *('+' >> term | '-' >> term);

BOOST_PARSER_DEFINE_RULES(expr, term, factor);

} // namespace

bool parse(std::string_view text)
{
    //ExprNode ast;
    auto it = text.begin();
    auto end = text.end();

    try
    {
        auto success = bp::prefix_parse(it, end, expr, bp::ws /*, bp::trace::on*/);
        return success && it == end;
    }
    catch (const bp::parse_error<std::string_view::const_iterator> &e)
    {
        std::cerr << "Parse error: " << e.what() << '\n';
        return false;
    }
}

} // namespace formula
