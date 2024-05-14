#include <iostream>
#include <cstdlib>
#include "argument_parser.h"

using namespace in;

bool get_arg(c_args& args, int& i, std::string_view& arg) {
    if (i >= args.argc - 1)
        return false;

    arg = args.argv[i++];
    return true;
}

bool get_filename(c_args& args, int& i, std::string_view& arg) {
    if (args.argc == 1)
        return false;

    if (args.argv[args.argc - 1][0] == '-')
        return false;

    arg = args.argv[args.argc - 1];
    return true;
}

bool is_switch(std::string_view arg) {
    return arg[0] == '-';
}

arg_env in::parse_args(int argc, char **argv) {
    arg_env env;
    c_args args { (size_t) argc, argv };

    int i = 1;
    std::string_view arg;

    while (get_arg(args, i, arg)) {
        if (arg == "-O0")
            env.opt_level = O0;
        else if (arg == "-O1")
            env.opt_level = O1;
        else if (arg == "-O2")
            env.opt_level = O2;
        else if (arg == "-O3")
            env.opt_level = O3;
        else if (arg == "-emit-llvm")
            env.emit = LLVM;
        else if (arg == "-emit-asm")
            env.emit = ASM;
        else if (arg == "-emit-obj")
            env.emit = OBJ;
        else if (arg == "-emit-exec")
            env.emit = EXEC;
        else {
            std::cerr << "Unknown argument: " << arg << '\n';
            std::exit(1);
        }
    }

    if (!get_filename(args, i, arg)) {
        std::cerr << "No input file provided\n";
        std::exit(1);
    }
}