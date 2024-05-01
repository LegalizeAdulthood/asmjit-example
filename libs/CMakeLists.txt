add_library(formula
    include/formula/formula.h
    formula.cpp
)
target_include_directories(formula PUBLIC include)
target_folder(formula "Libraries")
