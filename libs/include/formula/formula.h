#pragma once

#include <memory>
#include <string_view>

namespace formula
{

class Node;

class Formula
{
public:
    virtual ~Formula() = default;

    virtual double evaluate() = 0;

    virtual bool assemble() = 0;
};

std::shared_ptr<Formula> parse(std::string_view text);

}
