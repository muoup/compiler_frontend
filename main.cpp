#include <iostream>

#include "ast/interface.h"
#include "lexer/lex.h"

int main() {
    const auto* code = R"(
        void test() {
            for (i16 i = 0; i < 10; i += 1) {
                __libc_printf("%d", i);
            }
        }

        i8 main() {
            test();
            return 0;
        }
    )";

    const auto tokens = lex::lex(code);
    const auto ast = ast::parse(tokens);

    ast.print();
    std::cout << "-------------\n";

    return 0;
}
