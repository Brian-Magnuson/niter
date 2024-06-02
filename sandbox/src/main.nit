extern variadic fun printf(char*): i32

fun main(): i32 {
    var a = 21
    printf("a = %d\n", a)
    var b = a + 4
    printf("b = %d\n", b)
    var c = a - 4
    printf("c = %d\n", c)
    var d = a * 4
    printf("d = %d\n", d)
    var e = a / 4
    printf("e = %d\n", e)
    var f = a % 4
    printf("f = %d\n", f)
    var g = 2 ^ 3
    printf("g = %f\n", g)
        
    return 0
}
