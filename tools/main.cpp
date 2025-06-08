#include <formula/formula.h>

#include <iostream>
#include <string>

int main()
{
    std::cout << "Enter an expression:\n";
    std::string line;
    std::getline(std::cin, line);
    std::shared_ptr<formula::Formula> formula = formula::parse(line);
    if (!formula)
    {
        std::cerr << "Error: Invalid formula\n";
        return 1;
    }

    std::cout << "Evaluated: " << formula->evaluate() << '\n';
    return 0;
}
