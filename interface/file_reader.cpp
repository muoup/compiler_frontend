#include <sstream>
#include <fstream>
#include "file_reader.h"
#include "../lexer/lex.h"
#include "../ast/interface.h"

using namespace in;

file_pipeline& file_pipeline::load_file(std::string_view file_name) {
    std::ifstream file { file_name.data() };

    // At some point I really should move from using the barbaric C++ exception system
    if (file.bad())
        throw std::runtime_error("Failed to open file: " + std::string(file_name));

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

file_pipeline& file_pipeline::gen_lex() {
    this->tokens = lex::lex(this->code);
    return *this;
}

file_pipeline& file_pipeline::gen_ast() {
    this->ast = std::make_unique<ast::nodes::root>(ast::parse(this->tokens));
    return *this;
}