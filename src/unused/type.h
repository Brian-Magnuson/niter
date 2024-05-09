#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#define IS_TYPE(ptr, x) (std::dynamic_pointer_cast<x>(ptr) != nullptr)

/**
 * @brief A class for all types in our type system.
 * Should be stored as a shared pointer.
 * If the pointer is null, it means the type was not resolved yet.
 *
 */
class Type {
public:
    class Void;
    class Int;
    class Float;
    class Bool;
    class Char;
    class Pointer;
    class Array;
    class Tuple;
    class Function;
    class Struct;

    virtual ~Type() {}
};

/**
 * @brief The void type. Objects of this type have no value.
 *
 */
class Void : public Type {};

/**
 * @brief The integer type. Integers may be signed or unsigned, and may have different sizes.
 * The only sizes that should be used are 8, 16, 32, and 64.
 * Integer literals are assumed to be signed 32-bit integers.
 *
 */
class Int : public Type {
public:
    // Whether the integer is signed or unsigned
    bool is_signed;
    // The number of bits in the integer; should be 8, 16, 32, or 64
    int bits;

    Int(bool is_signed, int bits) : is_signed(is_signed), bits(bits) {}
};

/**
 * @brief The floating-point type. Floating-point numbers may have different sizes.
 * The only sizes that should be used are 32 and 64.
 * Floating-point literals are assumed to be 64-bit floating-point numbers (called doubles in other languages).
 *
 */
class Float : public Type {
public:
    // The number of bits in the floating-point number; should be 32 or 64
    int bits;

    Float(int bits) : bits(bits) {}
};

/**
 * @brief The boolean type. Booleans have only two values: true and false.
 * This class has no other members, as there is only one boolean type.
 *
 */
class Bool : public Type {};

/**
 * @brief The character type. Characters are 8-bit integers.
 *
 */
class Char : public Type {};

/**
 * @brief The pointer type. Pointers point to objects of other types.
 * Pointers are not arrays, and do not have a size.
 * However, we keep track of the type of the object they point to.
 *
 */
class Pointer : public Type {
public:
    // The type of the object that the pointer points to
    std::shared_ptr<Type> pointee;

    Pointer(std::shared_ptr<Type> pointee) : pointee(pointee) {}
};

/**
 * @brief The array type. Arrays are fixed-size sequences of objects of other types.
 * Arrays have a size and a type. There is no primitive type for a dynamically-sized array, though a user-defined type may be created using these arrays.
 *
 */
class Array : public Type {
public:
    // The type of the elements in the array
    std::shared_ptr<Type> element;
    // The number of elements in the array, not to be confused with the size in bytes.
    int length;

    Array(std::shared_ptr<Type> element, int length) : element(element), length(length) {}
};

/**
 * @brief The tuple type. Tuples are fixed-size sequences of objects of other types.
 * Tuples have a size and a type. Tuples are distinct from arrays in that they may contain objects of different types. They are also distinct from structs in that its members are not named.
 *
 */
class Tuple : public Type {
public:
    // The types of the elements in the tuple stored in an ordered list.
    std::vector<std::shared_ptr<Type>> elements;

    Tuple(std::vector<std::shared_ptr<Type>> elements) : elements(elements) {}
};

/**
 * @brief The function type. Functions are objects that can be called.
 * Functions have a return type and a list of parameter types.
 * Parameters are stored as a list of pairs, where the first element is the name of the parameter and the second element is the type of the parameter.
 *
 */
class Function : public Type {
public:
    // The return type of the function
    std::shared_ptr<Type> return_type;
    // The types of the parameters of the function stored in an ordered list.
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> parameters;
};

/**
 * @brief The struct type. Structs are user-defined types that contain a list of named members.
 * Structs have a size and a list of members.
 * The list of members is ordered (even though users typically do not utilize this ordering) and are stored as a list of pairs, where the first element is the name of the member and the second element is the type of the member.
 *
 */
class Struct : public Type {
public:
    // The types of the members of the struct stored in an ordered list.
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> members;

    Struct(std::vector<std::pair<std::string, std::shared_ptr<Type>>> members) : members(members) {}
};

#endif // TYPE_H
