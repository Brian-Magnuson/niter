extern variadic fun printf(char*): i32

struct Point {
    var x: i32
    var y: i32

    fun move(var this: Point*, dx: i32, dy: i32) {
        this->x = this->x + dx
        this->y = this->y + dy
    }
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

    p.move(2, 1)
    printf("p is: (%d, %d)\n", p.x, p.y)


    return 0
}
