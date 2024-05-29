#include <sstream>
#include <fstream>
#include "file_reader.h"
#include "../lexer/lex.h"
#include "../ast/interface.h"

#ifdef LLVM_ENABLE
#include "../llvm-gen/basic_codegen.h"
#include "../preprocess/preprocessor.hpp"

#endif

using namespace in;

file_pipeline & file_pipeline::load_file() {
    std::ifstream file { env.output_file };

    // At some point I really should move from using the barbaric C++ exception system
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + env.output_file);

    std::string line;
    std::ostringstream code_stream;

    while (!file.eof()){
        std::getline(file, line);
        code_stream << line << '\n';
    }

    this->code = code_stream.str();
    file.close();

    return *this;
}

file_pipeline &file_pipeline::pre_process() {
    pp::preprocess(this->code);

    return *this;
}

file_pipeline& file_pipeline::gen_lex() {
    this->tokens = lex::lex(this->code);
    return *this;
}

file_pipeline& file_pipeline::gen_ast() {
    this->ast = std::make_unique<ast::nodes::root>(ast::parse(this->tokens));
    return *this;
}

#ifdef LLVM_ENABLE

file_pipeline& file_pipeline::gen_llvm() {
    this->module = cg::generate_code(*ast);
    return *this;
}

file_pipeline& file_pipeline::print_llvm() {
    this->module->print(llvm::outs(), nullptr);
    return *this;
}

file_pipeline& file_pipeline::gen_exec() {

}

#endif