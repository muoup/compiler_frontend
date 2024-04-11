#include <iostream>
#include <llvm/Support/raw_ostream.h>

#include "llvm-gen/basic_codegen.h"
#include "ast/interface.h"
#include "lexer/lex.h"

void generate_machine_code(llvm::Module &module, const ast::nodes::root &root) {

}

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

    cg::generate_code(ast, llvm::outs());

    return 0;
}
