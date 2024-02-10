#include "lexer\lex.h"

int main() {
    const auto* code = R"(
        int main() {
            string s = "Hello, World!";
            int i = 0;
            i++;
            return 0;
        }
    )";

    auto tokens = lex::naive_lex(code);

    return 0;
}
