#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <memory>
#include <string>
#include <vector>

/**
 * @brief An enumeration of the different types of annotations.
 *
 */
enum class AnnotationType {
    CLASS,
    FUNCTION,
    ARRAY,
    POINTER,
    TUPLE
};

/**
 * @brief A base class for a type annotation.
 *
 */
class Annotation {
    class Class;
    class Function;
    class Array;
    class Pointer;
    class Tuple;

    virtual ~Annotation() = default;

    /**
     * @brief Get the string representation of the annotation.
     *
     * @return std::string
     */
    virtual std::string to_string() const = 0;
};

/**
 * @brief A class annotation.
 *
 */
class Annotation::Class : public Annotation {
public:
    // The name of the class. E.g. "std::Vector" has the class names "std" and "Vector".
    std::string name;
    // The type arguments of the class. E.g. "std::Vector<int>" has the type argument "int".
    std::vector<std::shared_ptr<Annotation>> type_args;
    // The inner annotation of the class. E.g. "std::Vector<int>::Iterator" has the inner annotation "Iterator".
    std::shared_ptr<Annotation::Class> inner;

    Class(std::string name, std::vector<std::shared_ptr<Annotation>>& type_args, std::shared_ptr<Annotation::Class> inner)
        : name(name), type_args(type_args), inner(inner) {}

    std::string to_string() const override {
        std::string result = name;
        if (type_args.size() > 0) {
            result += "<";
            for (size_t i = 0; i < type_args.size(); i++) {
                result += type_args[i]->to_string();
                if (i < type_args.size() - 1) {
                    result += ", ";
                }
            }
            result += ">";
        }
        if (inner != nullptr) {
            result += "::" + inner->to_string();
        }
        return result;
    }
};

/**
 * @brief A function annotation.
 *
 */
class Annotation::Function : public Annotation {
public:
    // The arguments of the function, stored as pairs of whether the argument is mutable and the annotation of the argument.
    std::vector<std::pair<bool, std::shared_ptr<Annotation>>> args;
    // The return type of the function. E.g. "(int, int) => void" has the return type "void".
    std::shared_ptr<Annotation> ret;
    // Whether the return type is mutable or not. If the return type is mutable, the keyword "var" is prepended to the return type.
    bool is_ret_mutable;

    Function(std::vector<std::pair<bool, std::shared_ptr<Annotation>>>& args, std::shared_ptr<Annotation> ret, bool is_ret_mutable)
        : args(args), ret(ret), is_ret_mutable(is_ret_mutable) {}

    std::string to_string() const override {
        std::string result = "fun(";
        for (size_t i = 0; i < args.size(); i++) {
            result += args[i].first ? "var " : "";
            result += args[i].second->to_string();
            if (i < args.size() - 1) {
                result += ", ";
            }
        }
        result += ") => ";
        result += is_ret_mutable ? "var " : "";
        result += ret->to_string();
        return result;
    }
};

/**
 * @brief An array annotation.
 *
 */
class Annotation::Array : public Annotation {
public:
    // The name of the array. E.g. "int[]" has the name "int".
    std::shared_ptr<Annotation> name;

    Array(std::shared_ptr<Annotation> name, bool is_mutable)
        : name(name) {}

    std::string to_string() const override {
        return name->to_string() + "[]";
    }
};

/**
 * @brief A pointer annotation.
 *
 */
class Annotation::Pointer : public Annotation {
public:
    // The name of the pointer. E.g. "int*" has the name "int".
    std::shared_ptr<Annotation> name;

    Pointer(std::shared_ptr<Annotation> name, bool is_obj_mutable)
        : name(name) {}

    std::string to_string() const override {
        return name->to_string() + "*";
    }
};

/**
 * @brief A tuple annotation.
 *
 */
class Annotation::Tuple : public Annotation {
public:
    // The elements of the tuple. E.g. "(int, int)" has the elements "int" and "int".
    std::vector<std::shared_ptr<Annotation>> elements;

    Tuple(std::vector<std::shared_ptr<Annotation>> elements)
        : elements(elements) {}

    std::string to_string() const override {
        std::string result = "(";
        for (size_t i = 0; i < elements.size(); i++) {
            result += elements[i]->to_string();
            if (i < elements.size() - 1) {
                result += ", ";
            }
        }
        result += ")";
        return result;
    }
};

#endif // ANNOTATION_H