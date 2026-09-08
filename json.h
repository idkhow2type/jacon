
#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

#define DECLARE_ARRAY(Name, T) \
    typedef struct Name {      \
        T* data;               \
        size_t len;            \
        size_t cap;            \
    } Name;                    \
    bool Name##_append(Name* array, T value);

DECLARE_ARRAY(JArray, struct JValue);
// is null terminated, but null can appear mid buffer
DECLARE_ARRAY(JString, char);

typedef struct JValue {
    enum {
        Undefined,
        Object,
        Array,
        String,
        Number,
        Bool,
        Null,
    } type;
    union {
        bool boolean;
        double number;
        JArray array;
        JString string;
    } data;
} JValue;

const char* Jparse(const char* in, JValue* out);
void JfreeValue(JValue* value);

#endif