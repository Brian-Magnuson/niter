extern variadic fun printf(char*): i32

fun main(): i32 {
    var my_var = "Hello, World!"
    printf("my_var: %s\n", my_var)
        
    return 0
}
