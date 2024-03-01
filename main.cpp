#include <iostream>

#include "ast/interface.h"
#include "lexer/lex.h"

int main() {
    const auto* code = R"(
        i8 main(i16 argc) {
            string s = "Hello, World!";
            char c = 'c';
            char c2 = '\n';
            i16 i = 0;
        }
    )";

    const auto tokens = lex::lex(code);
    // auto ast = ast::parse(tokens);

    // llvm_gen::hello_world_example();

    return 0;
}
