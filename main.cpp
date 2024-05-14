#include <iostream>
#include "interface/file_reader.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    in::file_pipeline pipeline;
    pipeline.load_file(argv[1])
            .gen_lex()
            .gen_ast();

    pipeline.ast->print(0);

    return 0;
}
