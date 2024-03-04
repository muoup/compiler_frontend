#include <iostream>
#include <llvm/Support/raw_ostream.h>

#include "ast/interface.h"
#include "ast/parser_methods/statement.h"
#include "lexer/lex.h"
#include "llvm-gen/codegen.h"
#include "llvm-gen/test.h"

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
        i8 main() {
            __clib_printf("hi");
            return 0;
        }
    )";

    const auto tokens = lex::lex(code);
    const auto ast = ast::parse(tokens);

    print_ast(ast);
    std::cout << "-------------\n";

    cg_llvm::generate_code(llvm::outs(), ast);


    // llvm_gen::hello_world_example();

    // const auto* statement = "string s = \"Hello, World!\";";
    //
    // const auto stmt_toks = lex::lex(statement);
    // auto ptr = stmt_toks.cbegin();
    // auto ast = ast::pm::parse_statement(ptr, stmt_toks.cend());

    // print_ast(ast);

    return 0;
}
