#include <iostream>
#include <fstream>
#include <llvm/Support/raw_ostream.h>

#include "llvm-gen/basic_codegen.h"
#include "ast/interface.h"
#include "lexer/lex.h"

int main() {
//    const auto* code = R"(
//        struct test_struct {
//            i8 a;
//            i16 b;
//            i32 c;
//        }
//
//        fn test() {
//            for (i16 i = 0; i < 10; i += 1) {
//                __libc_printf("%d", i);
//            }
//        }
//
//        fn main() -> i8 {
//            test();
//            return 0;
//        }
//    )";

    const auto *code = R"(
        fn main() -> i8 {
            i32 i = 0;
            match i {
                case 0 { return 1; },
                case 1 { return 2; },
                case 2 { return 3; },
            };
        }
    )";

    const auto tokens = lex::lex(code);
    const auto ast = ast::parse(tokens);

    ast.print();
    std::cout << "-------------\n";

    // The AST sometimes prints after the codegen, so we need to flush the output stream.
    cg::generate_code(ast, llvm::outs());

    return 0;
}
