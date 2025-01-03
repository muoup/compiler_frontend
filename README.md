<!-- ABOUT THE PROJECT -->
# About The Project
Basic compiler frontend for a custom toy language, written in C++. Based loosely on the LLVM Kaleidoscope tutorial
as a source of reference for what is needed to build a working frontend. The language is very C-like by design
that supports basic control flow, function calls, structures, arrays, pointers, and libc integration. In the future,
the language is planned to be changed to a proper C super-set, acting as a means to research and experiment with
modern language features.

<!-- GETTING STARTED -->
## Getting Started

### Pre-requisites

This software is tested using Mingw, GCC, and CMake on Windows 10, and a fresh-build of LLVM 12.0.0 for its IR API.

For windows, the following software is required:

* GCC and Mingw
For downloading GCC on Windows, see [WinLibs](https://winlibs.com/)

* LLVM
For help installing and building LLVM, see [LLVM](https://llvm.org/docs/CMake.html)

If using a UNIX-based system, or WSL, the same software is required, however can be installed using a package manager.

### Installation

The software currently works more as a library, there is a test implemetation of main.cpp which takes in an argument
of a file path to a source file, prints the generated IR to the console, and then produces an executable in the same
directory as the source file. This pipeline can be altered fairly easily to stop at different stages, or to output
to different ostreams.

The steps therefore to build the software are as follows:

1. Clone the repo either through some GitHub interface, or via the command line:
    ```sh
    git clone https://github.com/muoup/compiler_frontend
    ```
   
2. Build the software using CMake:
    ```sh
    mkdir build
    cd build
    cmake ..
    ```
   
3. Compile the software using cmake (from within the build directory):
    ```sh
    cmake --build .
    ```

Alternatively if an IDE is being used, this can be done through the IDE's interface.

## Example Code

Updated as of January 2nd, 2025.

The frontend now, with its preprocessor, supports a very basic library, which mostly act as libc wrappers. Including works
the same as with the C preprocessing directive, copying the contents of the file into the source file. With the stdio include, 
the following code can be used for a simple "Hello, World!" program:

```
#include stdio

fn main() -> i32 {
    println("Hello, World!");
}
```

Most standard forms of control flow are supported, including if-else statements, while/do while loops, and for loops,
as seen in the following example, which outputs the numbers 0 through 9:

Note that the compiler currently lacks escape character parsing, so the newline character
will not be recognized in the string literal.

```
#include stdio

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
#include stdio

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