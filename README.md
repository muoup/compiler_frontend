<a name="readme-top"></a>

# Compiler Frontend
Basic compiler frontend for a simple language using the LLVM IR C++ API and Clang for compilation. 
The ultimate goal for the language is falls somewhere between a simple C with basic OOP features
like interfaces and struct methods, and a more modern language like Rust with syntactic sugar and
useful features like pattern matching, destructuring, rich enums, and in-built tuple support.

## Table of Contents
1. [Example Code](#example-code)

## Example Code

Until a library system is implemented, all libc references must be declared by hand in the source code, however
the syntax is very similar to C, with a few minor differences. The following example demonstrates the syntax
for a basic "Hello, World!" program:

```
libc fn printf(char* str, ...) -> i32;

fn main() -> i32 {
    printf("Hello, World!\n");
}
```

Most standard forms of control flow are supported, including if-else statements, while/do while loops, and for loops,
as seen in the following example, which outputs the numbers 0 through 9:

Note that as the compiler works with non-owning strings, string literals cannot contain escape characters, this will
be fixed in the near future.

```
libc fn printf(char* str, ...) -> i32;

fn test() {
    for (i32 i = 0; i < 10; i++) {
        printf("%d ", i);
    }
}

fn main() -> i32 {
    test();
}
```

One final example of the language's capabilities is the ability for structs and arrays, as seen in the following example,
utilizing a struct to store two integers and an array to store 5 integers, which will output the numbers 1, 2, 3, 4, and 5:

```
libc fn printf(char* str, ...) -> i32;

struct example_struct {
    i32 a, i32 b
}

fn main() -> i32 {
    example_struct s = {1, 2};
    i32[] arr = {1, s.b, 3, 4, 5};
    
    for (i32 i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
}
```