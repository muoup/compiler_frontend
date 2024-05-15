#pragma once

#include <string_view>

namespace in {
    enum optimizer_level {
        O0, O1, O2, O3
    };

    enum emit_type {
        LLVM, ASM, OBJ, EXEC
    };

    struct c_args {
        size_t argc;
        char** argv;
    };

    struct arg_env {
        optimizer_level opt_level = O0;
        emit_type emit = EXEC;
        std::string output_file;
    };

    extern arg_env parse_args(int argc, char** argv);
}