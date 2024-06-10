extern variadic fun printf(char*): i32

struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var p = :Point {x: 1, y: 2}
    printf("Hello, world! (%d, %d)\n", p.x, p.y)
    return 0
}
