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

## Usage

The software currently is set up to take in the filepath of a source file as an argument, and will output the generated
executable. Along with this, the following flags can be used:

* -o <output_file> : Specify the name of the output file
* -O0/-O1/-O2/-O3 : Specify the optimization level (default is O0)

## Example Code

Updated as of January 2nd, 2025.

The frontend now, with its preprocessor, supports a very basic library, which mostly act as libc wrappers. Including works
the same as with the C preprocessing directive, copying the contents of the file into the source file. With the stdio include, 
the following code can be used for a simple "Hello, World!" program:

```
#include stdio

fn main() {
    println("Hello, World!");
}
```

A slightly more advanced example utilized all three currently implemented stdio functions: println, print, and input. This
program will prompt the user for their name, and then print a greeting to the console:

```
#include stdio

fn main() {
    char[32] name;

    println("Enter your name: ");
    input(name);

    print("Hello, ");
    println(name);
}
```

Most standard forms of control flow are supported, for instance the following code below will print a numbered
list of all the arguments passed to the program:

```
#include libc

fn main(i32 argc, char** argv) {
    printf("Arguments Received: %d\n", argc);
    for (i32 i = 0; i < argc; i += 1) {
        printf("Argument %d: %s\n", i, argv[i]);
    }
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
    
    for (i32 i = 0; i < 5; i += 1) {
        printf("%d ", arr[i]);
    }
}
```