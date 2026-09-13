
#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// array is readonly if cap = 0
#define DECLARE_ARRAY(Name, T)       \
    typedef struct Name {            \
        T* data;                     \
        size_t len;                  \
        size_t cap;                  \
    } Name;                          \
    bool Name##_resize(Name* array); \
    bool Name##_append(Name* array, T value);
DECLARE_ARRAY(jacArray, struct jacValue);

typedef struct jacString {
    char* data;
    size_t len;
    bool isView;
} jacString;

typedef struct ObjectField ObjectField;
typedef struct jacObject {
    ObjectField* data;
    size_t len;
    size_t cap;
} jacObject;

typedef struct jacValue {
    union {
        bool boolean;
        double dnumber;
        int inumber;
        jacArray array;
        jacString string;
        jacObject object;
    } data;
    enum {
        JAC_TYPE_UNDEFINED,
        JAC_TYPE_OBJECT,
        JAC_TYPE_ARRAY,
        JAC_TYPE_STRING,
        JAC_TYPE_DOUBLE,
        JAC_TYPE_INT,
        JAC_TYPE_BOOL,
        JAC_TYPE_NULL,
    } type;
} jacValue;
static inline jacValue jacValue_from_null() {
    return (jacValue){.type = JAC_TYPE_NULL};
}
static inline jacValue jacValue_from_bool(bool value) {
    return (jacValue){.type = JAC_TYPE_BOOL, .data.boolean = value};
}
static inline jacValue jacValue_from_int(int value) {
    return (jacValue){.type = JAC_TYPE_INT, .data.inumber = value};
}
static inline jacValue jacValue_from_float(float value) {
    return (jacValue){.type = JAC_TYPE_DOUBLE, .data.dnumber = (double)value};
}
static inline jacValue jacValue_from_double(double value) {
    return (jacValue){.type = JAC_TYPE_DOUBLE, .data.dnumber = value};
}
static inline jacValue jacValue_from_cstr(const char* value) {
    return (jacValue){
        .type = JAC_TYPE_STRING,
        .data.string = {
            .data = (char*)value, .len = strlen(value), .isView = true}};
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

bool jacObject_setjsval(jacObject* object, const jacString key,
                        const jacValue value);
#define jacObject_setval(object, key, value)                         \
    jacObject_setjsval(                                              \
        _Generic((object), jacObject*: object),                      \
        _Generic((key), char*: jacValue_from_cstr(key).data.string), \
        _Generic((value), jacValue: value))
#define jacObject_setjs(object, key, value)                    \
    jacObject_setjsval(_Generic((object), jacObject*: object), \
                       _Generic((key), jacString: key), jacValue_from(value))
#define jacObject_set(object, key, value)                    \
    jacObject_setval(_Generic((object), jacObject*: object), \
                     _Generic((key), char*: key), jacValue_from(value))
jacValue jacObject_getjs(const jacObject object, const jacString key);
#define jacObject_get                             \
    jacObject_getjs(object, key) jacObject_getjs( \
        _Generic((object), jacObject*: object),   \
        _Generic((key), char*: jacValue_from_cstr(key).data.string))
bool jacObject_iter(const jacObject object, size_t* i, jacString* key,
                    jacValue* value);

bool jac_parsejs(const jacString in, jacValue* out);
// this is kinda wasteful but it looks cool ig
#define jac_parse(in, out)                                                 \
    jac_parsejs(_Generic((in), char*: jacValue_from_cstr(in).data.string), \
                _Generic((out), jacValue*: out))
void jac_freeValue(jacValue* value);
jacString jac_encode(const jacValue value);

#endif