#include <formula/formula.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

int main(const std::vector<std::string_view> &args)
{
    bool assemble{};
    bool compile{};
    if (args.size() == 2)
    {
        if (args[1] == "--assemble")
        {
            assemble = true;
        }
        else if (args[1] == "--compile")
        {
            compile = true;
        }
        else
        {
            std::cerr << "Usage: " << args[0] << " [--assemble | --compile]\n";
            return 1;
        }
    }

    std::cout << "Enter an expression:\n";
    std::string line;
    std::getline(std::cin, line);
    std::shared_ptr<formula::Formula> formula = formula::parse(line);
    if (!formula)
    {
        std::cerr << "Error: Invalid formula\n";
        return 1;
    }

    if (assemble && !formula->assemble())
    {
        std::cerr << "Error: Failed to assemble formula\n";
        return 1;
    }
    if (compile && !formula->compile())
    {
        std::cerr << "Error: Failed to compile formula\n";
        return 1;
    }

    std::cout << "Evaluated: " << formula->evaluate() << '\n';
    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    std::vector<std::string_view> args;
    for (int i = 0; i < argc; ++i)
    {
        args.emplace_back(argv[i]);
    }
    return main(args);
}
