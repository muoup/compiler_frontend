#pragma once

#include "includes.h"

static llvm::LLVMContext TheContext;

namespace llvm_gen {
    static void hello_world_example() {
        // Using the LLVM Library, generate a hard-coded hello world program
        // Create a new module
        std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("top", TheContext);

        // Create the main function: first create the type 'int ()'
        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), false);
        llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get());

        // Create a basic block in the function
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(TheContext, "entry", mainFunc);
        llvm::IRBuilder<> builder(entry);

        // Create the string "Hello, world!\n"
        llvm::Value *helloWorld = builder.CreateGlobalStringPtr("Hello, world!\n");

        // Call the puts function with the string
        llvm::FunctionCallee putsFunc = module->getOrInsertFunction("puts", llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), llvm::Type::getInt8PtrTy(TheContext), true));
        builder.CreateCall(putsFunc, helloWorld);

        // Return 0
        builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheContext), 0));

        // Print the module to the console
        module->print(llvm::outs(), nullptr);
    }
}
