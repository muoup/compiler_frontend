#include "ast/analyzer.h"
#include "lexer/lex.h"

int main() {
    const auto* code = R"(
        i8 main(i16 argc) {
            string s = "Hello, World!";
            int i = 0;
            i++;
            return 0;
        }
    )";

    const auto tokens = lex::lex(code);
    auto ast = ast::parse(tokens);

    return 0;
}
