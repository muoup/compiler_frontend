#include <iostream>
#include "interface/file_reader.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    in::file_pipeline pipeline{argc, argv};

    pipeline
            .load_file()
            .gen_lex()
            .gen_ast();

    pipeline.ast->print(0);
    pipeline.gen_llvm();

    return 0;
}
