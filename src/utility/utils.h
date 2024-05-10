#ifndef UTILS_H
#define UTILS_H

// Determines if a shared pointer is pointing to an object of a certain type.
#define IS_TYPE(PTR, X) (std::dynamic_pointer_cast<X>(PTR) != nullptr)

// Determines if a map has a certain key.
#define HAS_KEY(MAP, KEY) (MAP.find(KEY) != MAP.end())

#endif // UTILS_H
