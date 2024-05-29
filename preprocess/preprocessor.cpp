#include <sstream>
#include <fstream>
#include "preprocessor.hpp"
#include "../interface/file_reader.h"

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

    ss << code_stream.str();
    inc_file.close();
}

static bool pp::handle_line(std::string_view line, std::stringstream &ss) {
    if (!line.starts_with("#"))
        return false;

    if (line.starts_with("#include"))
        handle_imports(line, ss);

    return true;
}

void pp::preprocess(in::file_pipeline &pipeline) {
    std::stringstream ss;

    auto line_begin = pipeline.code.begin();
    auto line_end = pipeline.code.begin();

    while (line_end != pipeline.code.end()) {
        line_end = std::find(line_begin, pipeline.code.end(), '\n');
        if (!handle_line({ line_begin, line_end }, ss))
            ss << std::string_view { line_begin, line_end } << '\n';

        if (line_end == pipeline.code.end())
            break;

        line_begin = line_end + 1;
    }

    pipeline.code = ss.str();
}
