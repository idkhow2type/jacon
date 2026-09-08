
#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

#define DECLARE_ARRAY(Name, T) \
    typedef struct Name {              \
        T* data;               \
        size_t len;            \
        size_t cap;            \
    } Name;                         \
    bool Name##_append(Name* array, T value);

enum JType {
    Undefined,
    Object,
    Array,
    String,
    Number,
    Bool,
    Null,
};

DECLARE_ARRAY(JArray, struct JValue);

typedef struct JValue {
    enum JType type;
    union {
        bool boolean;
        double number;
        struct JArray array;
        char *string;
    } data;
} JValue;

const char* Jparse(const char* in, JValue* out);
void JfreeValue(JValue* value);

#endif