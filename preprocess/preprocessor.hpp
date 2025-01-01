#pragma once

namespace in {
    struct file_pipeline;
}

namespace pp {

    static bool handle_line(std::string_view line, std::stringstream &ss);
    void preprocess(std::string &code);
}