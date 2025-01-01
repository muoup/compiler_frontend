#include <sstream>
#include <fstream>
#include "file_reader.h"
#include "../ast/interface.h"
#include "../preprocess/preprocessor.hpp"
#include "../ast/validator/validator.hpp"

#ifdef ENABLE_LLVM
#include "../llvm-gen/basic_codegen.h"
#include "../llvm-gen/basic_codegen.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include "llvm/Target/TargetMachine.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/MCContext.h>
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#endif

using namespace in;

file_pipeline & file_pipeline::load_file() {
    std::ifstream file { env.output_file };

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

file_pipeline& file_pipeline::val_ast() {
    ast::val::validate(*ast);
    return *this;
}

#ifdef ENABLE_LLVM

file_pipeline& file_pipeline::gen_llvm() {
    auto [new_module, new_context] = cg::generate_ir(*ast);

    this->module.swap(new_module);
    this->context.swap(new_context);

    this->module->print(llvm::outs(), nullptr);

    return *this;
}

file_pipeline& file_pipeline::compile() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(target_triple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);

    if (!target) {
        llvm::errs() << error;
        return *this;
    }

    auto cpu = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto target_machine = target->createTargetMachine(
        target_triple, cpu, features, opt, llvm::Reloc::Model::PIC_
    );

    module->setDataLayout(target_machine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(env.output_file + ".o", ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "Could not open file: " << ec.message();
        return *this;
    }

    llvm::legacy::PassManager pass;
    auto file_type = llvm::CodeGenFileType::ObjectFile;

    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return *this;
    }

    pass.run(*module);
    dest.flush();

    return *this;
}

#endif