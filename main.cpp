#include <iostream>
#include <llvm/Support/raw_ostream.h>

#include "llvm-gen/basic_codegen.h"
#include "ast/interface.h"
#include "lexer/lex.h"

int main() {
    const auto* code = R"(
        void test() {
            i16 i = 10;
            //if (i == 10) {
                __libc_printf("%d", i);
            //}
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

    cg::generate_code(llvm::outs(), ast);

    return 0;
}
