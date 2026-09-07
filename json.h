#include <stdbool.h>

#ifndef JSON_H
#define JSON_H

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
    int len;
    int cap;
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