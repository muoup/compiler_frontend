<a name="readme-top"></a>

# Compiler Frontend
Basic compiler frontend for a simple language using the LLVM IR C++ API and Clang for compilation. 
The ultimate goal for the language is falls somewhere between a simple C with basic OOP features
like interfaces and struct methods, and a more modern language like Rust with syntactic sugar and
useful features like pattern matching, destructuring, rich enums, and in-built tuple support.

## Table of Contents
1. [Example Code](#example-code)

## Example Code
```
i8 main() {
    i32 x = 5;
    
    __libc_printf("Hello, World!\n");
    __libc_printf("x = %d\n", x); 
}
```

For now until function prototypes / external definitions are implemented, the prefix
'__libc_' is used to denote external functions from the C standard library.

```
void test() {
    for (i32 i = 0; i < 10; i++) {
        __libc_printf("i = %d\n", i);
    }
}

i8 main() {
    test();
}
```

Most standard forms of control flow are supported, including if-else statements, while/do while loops, and for loops.