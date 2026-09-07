
#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

enum JType {
    Undefined,
    Object,
    Array,
    String,
    Number,
    Bool,
    Null,
};

struct JArray {
    struct JValue* data;
    size_t len;
    size_t cap;
};

typedef struct JValue {
    enum JType type;
    union {
        bool boolean;
        double number;
        struct JArray array;
    } data;
} JValue;

const char* Jparse(const char* in, JValue* out);
void JfreeValue(JValue* value);

#endif