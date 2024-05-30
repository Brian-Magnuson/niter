extern variadic fun printf(char*): i32

fun main(): i32 {
    var x = 42
    x = 21

    printf("The answer is: %d\n", x)
    return 0
}
