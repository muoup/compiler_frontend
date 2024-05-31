#include <sstream>
#include <fstream>
#include <algorithm>
#include "preprocessor.hpp"

void handle_imports(std::string_view line, std::stringstream &ss) {
    if (line.length() < 10)
        throw std::runtime_error("Invalid import statement");

    auto path = line.substr(9);

    std::ifstream inc_file { std::string(path) + ".on" };

    if (!inc_file.is_open())
        inc_file = std::ifstream { "lib/" + std::string(path) + ".on" };

    if (!inc_file.is_open())
        throw std::runtime_error("Could not open file: " + std::string(path));

    std::string read_line;
    std::ostringstream code_stream;

    while (!inc_file.eof()){
        std::getline(inc_file, read_line);
        code_stream << read_line << '\n';
    }

    std::string code = code_stream.str();

    pp::preprocess(code);

    ss << code;

    inc_file.close();
}

static bool pp::handle_line(std::string_view line, std::stringstream &ss) {
    if (!line.starts_with("#"))
        return false;

    if (line.starts_with("#include"))
        handle_imports(line, ss);

    return true;
}

void pp::preprocess(std::string &code) {
    std::stringstream ss;

    auto line_begin = code.begin();
    auto line_end = code.begin();

    while (line_end != code.end()) {
        line_end = std::find(line_begin, code.end(), '\n');

        if (!handle_line({ line_begin, line_end }, ss))
            ss << std::string_view { line_begin, line_end } << '\n';

        if (line_end == code.end())
            break;

        line_begin = line_end + 1;
    }

    code = ss.str();
}
