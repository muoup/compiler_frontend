#include <iostream>
#include <llvm/Support/raw_ostream.h>

#include "ast/interface.h"
#include "ast/parser_methods/statement.h"
#include "lexer/lex.h"
#include "llvm-gen/codegen.h"

void print_ast(const ast::ast_node& node, const int depth = 0) {
    for (const auto& child : node.children) {
        for (int i = 0; i < depth; ++i)
            std::cout << "  ";
        std::cout << static_cast<int>(child.type) << " " << child.data << " " << child.metadata << "\n";
        print_ast(child, depth + 1);
    }
}

int main() {
    const auto* code = R"(
        void test() {
            i16 i = 10;
            __libc_printf("%d", i);
        }

        i8 main() {
            test();
            return 0;
        }
    )";

    const auto tokens = lex::lex(code);
    const auto ast = ast::parse(tokens);

    std::cout << "-------------\n";

    //cg_llvm::generate_code(llvm::outs(), ast);

    return 0;
}
