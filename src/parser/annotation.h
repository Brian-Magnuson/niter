#ifndef ANNOTATION_H
#define ANNOTATION_H

#include "../scanner/token.h"
#include "../utility/utils.h"

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
public:
    class Segmented;
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
 * @brief A segmented annotation.
 * Segmented annotations consist of multiple classes separated by "::".
 * E.g. `t::t`
 *
 */
class Annotation::Segmented : public Annotation {
public:
    class Class {
    public:
        // The name of the class. E.g. "std::Vector<int>" has class names "std" and "Vector".
        std::string name;
        // The type arguments of the class. E.g. "std::Vector<int>" has type arguments "int".
        std::vector<std::shared_ptr<Annotation>> type_args;

        Class(std::string name, std::vector<std::shared_ptr<Annotation>> type_args)
            : name(name), type_args(type_args) {}
    };

    // The classes that make up the segmented annotation. E.g. "std::Vector<int>::Iterator" has classes "std", "Vector<int>", and "Iterator".
    std::vector<std::shared_ptr<Class>> classes;

    Segmented(std::string name)
        : classes({std::make_shared<Class>(name, std::vector<std::shared_ptr<Annotation>>())}) {}

    Segmented(std::vector<std::shared_ptr<Class>> classes)
        : classes(classes) {}

    std::string to_string() const override {
        std::string result = "";
        for (size_t i = 0; i < classes.size(); i++) {
            result += classes[i]->name;
            if (classes[i]->type_args.size() > 0) {
                result += "<";
                for (size_t j = 0; j < classes[i]->type_args.size(); j++) {
                    result += classes[i]->type_args[j]->to_string();
                    if (j < classes[i]->type_args.size() - 1) {
                        result += ", ";
                    }
                }
                result += ">";
            }
            if (i < classes.size() - 1) {
                result += "::";
            }
        }
        return result;
    }
};

/**
 * @brief A function annotation.
 * E.g. `fun(t, t) => t`
 *
 */
class Annotation::Function : public Annotation {
public:
    // The arguments of the function, stored as pairs of whether the argument is mutable and the annotation of the argument.
    std::vector<std::pair<TokenType, std::shared_ptr<Annotation>>> params;
    // The return type of the function. E.g. "(int, int) => void" has the return type "void".
    std::shared_ptr<Annotation> return_annotation;
    // The declarer of the return type. E.g. "fun(int, int) => var void" has the return type declarer "var".
    TokenType return_declarer;
    // Whether the function is variadic. E.g. "fun(int, ...) => void" has the function as variadic.
    bool is_variadic = false;

    Function(
        std::vector<std::pair<TokenType, std::shared_ptr<Annotation>>> params,
        std::shared_ptr<Annotation> return_annotation,
        TokenType return_declarer,
        bool is_variadic = false
    ) : params(params), return_annotation(return_annotation), return_declarer(return_declarer), is_variadic(is_variadic) {}

    std::string to_string() const override {
        std::string result = "fun(";
        for (size_t i = 0; i < params.size(); i++) {
            result += params[i].first == KW_VAR ? "var " : "";
            result += params[i].second->to_string();
            if (i < params.size() - 1) {
                result += ", ";
            }
        }
        if (is_variadic) {
            result += ", ...";
        }
        result += ") => ";
        result += return_declarer == KW_VAR ? "var " : "";
        result += return_annotation->to_string();
        return result;
    }
};

/**
 * @brief An array annotation.
 * E.g. `[t; 10]` or `[t; *]`
 *
 */
class Annotation::Array : public Annotation {
public:
    // The inner type of the array. E.g. "[int; 1]" has the inner type "int".
    std::shared_ptr<Annotation> inner;
    // The size of the array. E.g. "[int; 1]" has the size 1. Unknown size is represented by -1.
    int size = -1;

    Array(std::shared_ptr<Annotation> inner, int size)
        : inner(inner), size(size) {}

    std::string to_string() const override {
        return "[" + inner->to_string() + (size == -1 ? "; *" : "; " + std::to_string(size)) + "]";
    }
};

/**
 * @brief A pointer annotation.
 *
 */
class Annotation::Pointer : public Annotation {
public:
    // The name of the pointer. E.g. "int*" has the name "int".
    std::shared_ptr<Annotation> inner;

    Pointer(std::shared_ptr<Annotation> inner)
        : inner(inner) {}

    std::string to_string() const override {
        return inner->to_string() + "*";
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
