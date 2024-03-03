#include <iostream>

#include "ast/interface.h"
#include "ast/parser_methods/statement.h"
#include "lexer/lex.h"
// #include "llvm-gen/test.h"

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
        i8 main(i16 argc) {
            string s = "Hello, World!";
            char c = 'c';
            char c2 = '\n';
            i16 i = 0;
        }
    )";

    const auto tokens = lex::lex(code);
    auto ast = ast::parse(tokens);

    // llvm_gen::hello_world_example();

    // const auto* statement = "string s = \"Hello, World!\";";
    //
    // const auto stmt_toks = lex::lex(statement);
    // auto ptr = stmt_toks.cbegin();
    // auto ast = ast::pm::parse_statement(ptr, stmt_toks.cend());

    print_ast(ast);

    return 0;
}
