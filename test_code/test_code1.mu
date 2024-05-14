libc fn printf(char* str, ...) -> i32;

fn main() {
    i32 x = 42;
    printf("Hello, world! %d", x);
}