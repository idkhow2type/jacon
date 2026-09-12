
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

DECLARE_ARRAY(jacArray, struct jacValue);
// strings are utf8 encoded, might support byo encoder later
// string data is null terminated, but null can appear mid buffer
DECLARE_ARRAY(jacString, char);

typedef struct ObjectField ObjectField;

typedef struct jacObject {
    ObjectField* data;
    size_t len;
    size_t cap;
} jacObject;

typedef struct jacValue {
    enum {
        JAC_TYPE_UNDEFINED,
        JAC_TYPE_OBJECT,
        JAC_TYPE_ARRAY,
        JAC_TYPE_STRING,
        JAC_TYPE_NUMBER,
        JAC_TYPE_BOOL,
        JAC_TYPE_NULL,
    } type;
    union {
        bool boolean;
        double number;
        jacArray array;
        jacString string;
        jacObject object;
    } data;
} jacValue;

static inline jacValue jacValue_from_null() {
    return (jacValue){.type = JAC_TYPE_NULL};
}
static inline jacValue jacValue_from_bool(bool value) {
    return (jacValue){.type = JAC_TYPE_BOOL, .data.boolean = value};
}
static inline jacValue jacValue_from_int(int value) {
    return (jacValue){.type = JAC_TYPE_NUMBER, .data.number = (double)value};
}
static inline jacValue jacValue_from_float(double value) {
    return (jacValue){.type = JAC_TYPE_NUMBER, .data.number = (double)value};
}
static inline jacValue jacValue_from_double(double value) {
    return (jacValue){.type = JAC_TYPE_NUMBER, .data.number = (double)value};
}
static inline jacValue jacValue_from_cstr(const char* value) {
    return (jacValue){
        .type = JAC_TYPE_STRING,
        .data.string = {.data = (char*)value, .len = strlen(value), .cap = 0}};
}
#define jacValue_from(value)          \
    _Generic((value),                 \
        void*: jacValue_from_null,    \
        bool: jacValue_from_bool,     \
        int: jacValue_from_int,       \
        float: jacValue_from_float,   \
        double: jacValue_from_double, \
        char*: jacValue_from_cstr,    \
        const char*: jacValue_from_cstr)(value)

bool jac_parse(const char* in, jacValue* out);
void jac_freeValue(jacValue* value);

jacString jac_encode(const jacValue value);

bool jacObject_setjsval(jacObject* object, const jacString key,
                      const jacValue value);
#define jacObject_setval(object, key, value)                     \
    jacObject_setjsval(                                                \
        _Generic((object), jacObject*: object),                      \
        _Generic((key), char*: jacValue_from_cstr(key).data.string), \
        _Generic((value), jacValue: value))
#define jacObject_setjs(object, key, value)                    \
    jacObject_setjsval(_Generic((object), jacObject*: object), \
                     _Generic((key), jacString: key), jacValue_from(value))
#define jacObject_set(object, key, value)                    \
    jacObject_setval(_Generic((object), jacObject*: object), \
                         _Generic((key), char*: key), jacValue_from(value))

jacValue jacObject_get(const jacObject object, const jacString key);
jacValue jacObject_getcstr(const jacObject object, const char* key);
bool jacObject_iter(const jacObject object, size_t* i, jacString* key,
                    jacValue* value);

#endif