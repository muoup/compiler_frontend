//
// Created by zbv11 on 23-Apr-24.
//

#ifndef LLVM_FRONTEND_FILE_READER_H
#define LLVM_FRONTEND_FILE_READER_H

#include <string>
#include <vector>
#include <memory>

#include "../lexer/lex.h"
#include "../ast/data/ast_nodes.h"

namespace in {
    struct file_pipeline {
        std::string code;
        std::vector<lex::lex_token> tokens;
        std::unique_ptr<ast::nodes::root> ast;

        file_pipeline() = default;

        file_pipeline& load_file(std::string_view file_name);
        file_pipeline& gen_lex();
        file_pipeline& gen_ast();

#ifdef LLVM_ENABLE
        file_pipeline& gen_llvm();
#endif
    };

}

#endif
