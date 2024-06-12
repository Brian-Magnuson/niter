extern variadic fun printf(char*): i32

struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var p = :Point {x: 1, y: 2}
    printf("p is: (%d, %d)\n", p.x, p.y)
    p.x = 3
    p.y = 4
    printf("p is: (%d, %d)\n", p.x, p.y)

    var pair: (i32, i32) = (5, 6)
    printf("pair is: (%d, %d)\n", pair[0], pair[1])
    pair[0] = 7
    pair[1] = 8
    printf("pair is: (%d, %d)\n", pair[0], pair[1])
    
    return 0
}
