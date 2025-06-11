find_package(asmjit CONFIG REQUIRED)

add_executable(asm-example example.cpp)
target_link_libraries(asm-example PRIVATE asmjit::asmjit)
target_folder(asm-example "Examples")
