extern variadic fun printf(char*): i32

fun main(): i32 {
    const num = 42
    printf("Hello, world!\n")
    printf("The answer is: %d\n", num)
    return 0
}
