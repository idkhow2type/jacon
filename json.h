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

typedef struct {
    enum JType type;
    void* data;
} JValue;

int parse(char* in, JValue* out);

#endif