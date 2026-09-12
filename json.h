
#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// array is readonly if cap = 0
#define DECLARE_ARRAY(Name, T) \
    typedef struct Name {      \
        T* data;               \
        size_t len;            \
        size_t cap;            \
    } Name;                    \
    bool Name##_append(Name* array, T value);

DECLARE_ARRAY(JArray, struct JValue);
// strings are utf8 encoded, might support byo encoder later
// string data is null terminated, but null can appear mid buffer
DECLARE_ARRAY(JString, char);

typedef struct ObjectField ObjectField;

typedef struct JObject {
    ObjectField* data;
    size_t len;
    size_t cap;
} JObject;

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
        JObject object;
    } data;
} JValue;

static inline JValue JValue_from_null() { return (JValue){.type = Null}; }
static inline JValue JValue_from_bool(bool value) {
    return (JValue){.type = Bool, .data.boolean = value};
}
static inline JValue JValue_from_int(int value) {
    return (JValue){.type = Number, .data.number = (double)value};
}
static inline JValue JValue_from_float(double value) {
    return (JValue){.type = Number, .data.number = (double)value};
}
static inline JValue JValue_from_double(double value) {
    return (JValue){.type = Number, .data.number = (double)value};
}
static inline JValue JValue_from_cstr(const char* value) {
    return (JValue){
        .type = String,
        .data.string = {.data = (char*)value, .len = strlen(value), .cap = 0}};
}
#define JValue_from(value)          \
    _Generic((value),               \
        void*: JValue_from_null,    \
        bool: JValue_from_bool,     \
        int: JValue_from_int,       \
        float: JValue_from_float,   \
        double: JValue_from_double, \
        char*: JValue_from_cstr,    \
        const char*: JValue_from_cstr)(value)

bool Jparse(const char* in, JValue* out);
void JfreeValue(JValue* value);

JString Jencode(const JValue value);

bool JObject_set(JObject* object, const JString key, const JValue value);
#define JObject_setcstr(object, key, value)                                \
    JObject_set(object,                                                    \
                _Generic((key), char*: JValue_from_cstr(key).data.string), \
                value)
#define JObject_set2(object, key, value) \
    JObject_set(object, key, JValue_from(value))
#define JObject_setcstr2(object, key, value) \
    JObject_setcstr(object, key, JValue_from(value))

JValue JObject_get(const JObject object, const JString key);
JValue JObject_getcstr(const JObject object, const char* key);
bool JObject_iter(const JObject object, size_t* i, JString* key, JValue* value);

#endif