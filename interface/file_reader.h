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
#include "argument_parser.h"

namespace in {
    struct file_pipeline {
        arg_env env;

        std::string code;
        std::vector<lex::lex_token> tokens;
        std::unique_ptr<ast::nodes::root> ast;

#ifdef LLVM_ENABLE
        std::shared_ptr<llvm::Module> module;
#endif

        file_pipeline(int argc, char** argv)
            : env(parse_args(argc, argv)) {}

        file_pipeline& load_file();
        file_pipeline& pre_process();
        file_pipeline& gen_lex();
        file_pipeline& gen_ast();

#ifdef LLVM_ENABLE
        file_pipeline& gen_llvm();
#endif

        file_pipeline& gen_exec();
    };

}

#endif
