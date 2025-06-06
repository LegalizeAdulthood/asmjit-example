// SPDX-License-Identifier: BSL-1.0
//
// Based on a Boost.Parser example by Larry Evans; see
// <https://github.com/boostorg/parser/issues/141>.
//
#include "formula/formula.h"

#include <boost/parser/parser.hpp>

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <limits>
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

ExprNode make_unary(char op, ExprNode operand)
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

/////////////////////////////////////////////////////////////////////////////////

template <typename Visitor, typename Variant>
decltype(auto) visit_node_recursively(Visitor &&visitor, Variant &&variant)
{
    return std::visit(std::forward<Visitor>(visitor), std::forward<Variant>(variant));
}

namespace impl
{

enum class ub_operator
{
    uminus,
    add,
    subtract,
    multiply,
    divide
};

struct node;
using onode = std::optional<node>;
using vnode = std::variant<unsigned int, int, float, double, onode>;
using node_array = std::vector<vnode>;
struct node
{
    ub_operator op;
    node_array nodes;

    node(const ub_operator op, vnode const &arg1) :
        op(op),
        nodes{arg1}
    {
    }
    node(const ub_operator op, vnode const &arg1, vnode const &arg2) :
        op(op),
        nodes{arg1, arg2}
    {
    }
};

} // namespace impl

using impl::ub_operator;
using impl::vnode;
using impl::onode;

struct node_visitor
{
    double operator()(unsigned int x) const
    {
        return (double) x;
    }
    double operator()(int x) const
    {
        return (double) x;
    }
    double operator()(float x) const
    {
        return (double) x;
    }
    double operator()(double x) const
    {
        return x;
    }

    double operator()(onode const &pn)
    {
        auto &node = *pn;
        switch (node.op)
        {
        case ub_operator::uminus:
            assert(node.nodes.size() == 1);
            return -visit_node_recursively(*this, node.nodes[0]);

        case ub_operator::add:
            assert(node.nodes.size() == 2);
            return visit_node_recursively(*this, node.nodes[0]) + visit_node_recursively(*this, node.nodes[1]);

        case ub_operator::subtract:
            assert(node.nodes.size() == 2);
            return visit_node_recursively(*this, node.nodes[0]) - visit_node_recursively(*this, node.nodes[1]);

        case ub_operator::multiply:
            assert(node.nodes.size() == 2);
            return visit_node_recursively(*this, node.nodes[0]) * visit_node_recursively(*this, node.nodes[1]);

        case ub_operator::divide:
            assert(node.nodes.size() == 2);
            return visit_node_recursively(*this, node.nodes[0]) / visit_node_recursively(*this, node.nodes[1]);
        }
    }
};

namespace client
{
///////////////////////////////////////////////////////////////////////////////
//  Semantic actions
////////////////////////////////////////////////////////1///////////////////////
namespace
{

std::deque<vnode> vn_stack;

auto do_int = [](auto &ctx)
{
    std::cout << "push " << bp::_attr(ctx) << std::endl;
    vnode i = _attr(ctx);
    vn_stack.push_front(i); // ast
};
auto const do_add = [](auto &ctx)
{
    std::cout << "add" << '\n';
    // stack effect notation ( a, b -- c ) where c = a + b
    { // ast
        assert(vn_stack.size() > 1);
        auto &&second = vn_stack[0];
        auto &&first = vn_stack[1];
        auto result = std::optional(impl::node(ub_operator::add, first, second));
        vn_stack[1] = result;
        vn_stack.pop_front();
    }
};
auto const do_subt = [](auto &ctx)
{
    std::cout << "subtract" << '\n';
    // stack effect notation ( a, b -- c ) where c = a - b
    { // ast
        assert(vn_stack.size() > 1);
        auto &&second = vn_stack[0];
        auto &&first = vn_stack[1];
        auto result = std::optional(impl::node(ub_operator::subtract, first, second));
        vn_stack[1] = result;
        vn_stack.pop_front();
    }
};
auto const do_mult = [](auto &ctx)
{
    std::cout << "mult" << '\n';
    // stack effect notation ( a, b -- c ) where c = a * b
    { // ast
        assert(vn_stack.size() > 1);
        auto &&second = vn_stack[0];
        auto &&first = vn_stack[1];
        auto result = std::optional(impl::node(ub_operator::multiply, first, second));
        vn_stack[1] = result;
        vn_stack.pop_front();
    }
};
auto const do_div = [](auto &ctx)
{
    std::cout << "divide" << '\n';
    // stack effect notation ( a, b -- c ) where c = a / b
    { // ast
        assert(vn_stack.size() > 1);
        auto &&second = vn_stack[0];
        auto &&first = vn_stack[1];
        auto result = std::optional(impl::node(ub_operator::divide, first, second));
        vn_stack[1] = result;
        vn_stack.pop_front();
    }
};
auto const do_neg = [](auto &ctx)
{
    std::cout << "negate" << '\n';
    // stack effect notation ( a -- -a )
    { // ast
        assert(vn_stack.size() > 0);
        auto &&arg = vn_stack[0];
        auto result = std::optional(impl::node(ub_operator::uminus, arg));
        vn_stack[0] = result;
    }
};

} // namespace

///////////////////////////////////////////////////////////////////////////////
//  The calculator grammar
///////////////////////////////////////////////////////////////////////////////
namespace calculator_grammar
{

bp::rule<class expression> const expression("expression");

auto const uint = bp::uint_[do_int] | '(' >> expression >> ')';

auto const factor = uint | ('-' >> uint[do_neg]) | ('+' >> uint);

auto const term = factor >> *(('*' >> factor[do_mult]) | ('/' >> factor[do_div]));

auto const expression_def = term >> *(('+' >> term[do_add]) | ('-' >> term[do_subt]));

BOOST_PARSER_DEFINE_RULES(expression);

auto calculator = expression;

} // namespace calculator_grammar

using calculator_grammar::calculator;

}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main()
{
    // test_recursive_node_visitor();
    std::cout << "/////////////////////////////////////////////////////////\n"
                 "Expression parser...\n"
                 "/////////////////////////////////////////////////////////\n\n";

    std::string inputs[] = {"999", "-888", "1+2", "2*3", "2*(3+4)", "2*-(3+4)"};
    for (std::string str : inputs)
    {
        auto &calc = client::calculator; // Our grammar

        if (auto r = bp::parse(str, calc, bp::ws))
        {
            std::cout << "-------------------------\n"
                         "Parsing succeeded\n";
            assert(client::vn_stack.size() == 1);
            std::cout << str << " ==> " << std::visit(node_visitor(), client::vn_stack[0]) << " (AST eval)\n";
            client::vn_stack.pop_front();
            std::cout << "-------------------------\n";
        }
        else
        {
            typedef std::string::const_iterator iterator_type;
            iterator_type iter = str.begin();
            iterator_type end = str.end();
            std::cout << "-------------------------\n"
                         "Parsing failed\n";
            bp::prefix_parse(iter, end, calc, bp::ws);
            std::string rest(iter, end);
            std::cout << "stopped at: \"" << rest << "\"\n";
            client::vn_stack.clear();
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-)\n\n";
    return 0;
}

} // namespace

} // namespace formula
