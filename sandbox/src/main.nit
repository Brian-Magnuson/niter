extern variadic fun printf(char*): i32

fun add(a: i32, b: i32): i32 {
    return a + b
}

fun main(): i32 {
    printf("add(1, 2): %d\n", add(1, 2))
    return 0
}
