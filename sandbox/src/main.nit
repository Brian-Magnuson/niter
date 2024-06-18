extern variadic fun printf(char*): i32

struct Point {
    var x: i32
    var y: i32
    var z: i32

    fun move(var this: Point*, dx: i32, dy: i32) {
        this->x = this->x + dx
        this->y = this->y + dy
    }
}

fun main(): i32 {
    var p = :Point {x: 1, y: 2, z: 3}
    printf("p is: (%d, %d, %d)\n", p.x, p.y, p.z)
    p.x = 3
    p.y = 4
    p.z = 6
    printf("p is: (%d, %d, %d)\n", p.x, p.y, p.z)

    var pair: (i32, i32) = (5, 6)
    printf("pair is: (%d, %d)\n", pair[0], pair[1])
    pair[0] = 7
    pair[1] = 8
    printf("pair is: (%d, %d)\n", pair[0], pair[1])

    var triple: (i32, i32, i32) = (9, 10, 11)
    printf("triple is: (%d, %d, %d)\n", triple[0], triple[1], triple[2])
    triple[0] = 12
    triple[1] = 13
    triple[2] = 14
    printf("triple is: (%d, %d, %d)\n", triple[0], triple[1], triple[2])
//
//    p.move(2, 1)
//    printf("p is: (%d, %d)\n", p.x, p.y)
//
//    var i = 0;
//    var j = &i;
//    printf("i is: %d; j is: %d\n", i, *j)
//
//    i = 1
//    printf("i is: %d; j is: %d\n", i, *j)
//    *j = 2
//    printf("i is: %d; j is: %d\n", i, *j)


    return 0
}
