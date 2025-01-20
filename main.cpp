#include <iostream>
#include "interface/file_reader.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    in::file_pipeline pipeline { argc, argv };

//    pipeline.compile(true);

    pipeline
        .load_file()
        .pre_process()
        .gen_lex()
        .gen_ast()
        .val_ast()
        .print_ast();
//        .gen_llvm()
//        .print_llvm()
//        .gen_object_file()
//        .compile()
//        .cleanup();

    return 0;
}
