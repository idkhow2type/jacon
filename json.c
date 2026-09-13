#include "json.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAP 4
#define MAKE_ARRAY(Name, T)                                                  \
    (Name) {                                                                 \
        .len = 0, .cap = DEFAULT_CAP, .data = calloc(DEFAULT_CAP, sizeof(T)) \
    }
#define DEFINE_ARRAY(Name, T)                                                \
    bool Name##_resize(Name* array) {                                        \
        size_t next_cap = array->cap == 0 ? DEFAULT_CAP : array->cap * 2;    \
        if (next_cap < array->cap ||                                         \
            next_cap > (size_t)-1 / sizeof(*array->data)) {                  \
            return false;                                                    \
        }                                                                    \
                                                                             \
        T* next = realloc(array->data, next_cap * sizeof(*next));            \
        if (next == NULL) return false;                                      \
                                                                             \
        array->data = next;                                                  \
        array->cap = next_cap;                                               \
        return true;                                                         \
    }                                                                        \
    bool Name##_append(Name* array, T value) {                               \
        if (array->len >= array->cap && !Name##_resize(array)) return false; \
        array->data[array->len++] = value;                                   \
        return true;                                                         \
    }

DEFINE_ARRAY(jacArray, struct jacValue);
DECLARE_ARRAY(CharArray, char);
DEFINE_ARRAY(CharArray, char);

bool jacString_cmp(jacString a, jacString b) {
    if (a.len != b.len) return false;
    for (size_t i = 0; i < a.len; ++i)
        if (a.data[i] != b.data[i]) return false;
    return true;
}

typedef struct ObjectField {
    jacValue value;
    jacString key;
} ObjectField;

static uint64_t hash(const char* s, size_t len) {
    uint64_t hash = 14695981039346656037ULL;

    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint8_t)*s++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

static bool jacObject_resize(jacObject* object) {
    size_t next_cap = object->cap == 0 ? DEFAULT_CAP : object->cap * 2;
    if (next_cap < object->cap ||
        next_cap > (size_t)-1 / sizeof(*object->data)) {
        return false;
    }
    struct ObjectField* old = object->data;
    object->data = calloc(next_cap, sizeof(*object->data));
    if (object->data == NULL) {
        object->data = old;
        return false;
    }
    size_t old_cap = object->cap;
    object->cap = next_cap;
    object->len = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        if (old[i].value.type != JAC_TYPE_UNDEFINED)
            jacObject_setjsval(object, old[i].key, old[i].value);
    }
    free(old);
    return true;
}

bool jacObject_setjsval(jacObject* object, const jacString key,
                        const jacValue value) {
    if (object->len >= object->cap - object->cap / 4)
        if (!jacObject_resize(object)) return false;

    size_t h = (hash(key.data, key.len) & (object->cap - 1));
    bool override = false;
    for (size_t i = 0; object->data[h].value.type != JAC_TYPE_UNDEFINED &&
                       !(override = jacString_cmp(object->data[h].key, key));
         ++i)
        h = (h + (i + i * i) / 2) % object->cap;

    object->data[h] = (struct ObjectField){.value = value, .key = key};
    object->len += !override;
    return true;
}

jacValue jacObject_getjs(const jacObject object, const jacString key) {
    size_t h = (hash(key.data, key.len) & (object.cap - 1));
    for (size_t i = 0; !jacString_cmp(object.data[h].key, key); ++i)
        h = (h + (i + i * i) / 2) % object.cap;
    return object.data[h].value;
}

bool jacObject_iter(const jacObject object, size_t* i, jacString* key,
                    jacValue* value) {
    while (*i < object.cap) {
        if (object.data[*i].value.type != JAC_TYPE_UNDEFINED) {
            *key = object.data[*i].key;
            *value = object.data[*i].value;
            ++*i;
            return true;
        };
        ++*i;
    }
    return false;
};

void jac_freeValue(jacValue* value) {
    switch (value->type) {
        case JAC_TYPE_ARRAY:
            for (size_t i = 0; i < value->data.array.len; ++i)
                jac_freeValue(&value->data.array.data[i]);
            if (value->data.array.cap) free(value->data.array.data);
            break;
        case JAC_TYPE_STRING:
            if (!value->data.string.isView) free(value->data.string.data);
            break;
        case JAC_TYPE_OBJECT:
            size_t i = 0;
            jacString key = {0};
            jacValue field = {0};
            while (jacObject_iter(value->data.object, &i, &key, &field)) {
                if (!key.isView) free(key.data);
                jac_freeValue(&field);
            }
            free(value->data.object.data);
            break;
        default:
            break;
    }
    *value = (jacValue){0};
}

// https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5
static size_t utf8_encode(char* out, uint32_t utf) {
    if (utf <= 0x7F) {
        // Plain ASCII
        out[0] = (char)utf;
        out[1] = 0;
        return 1;
    } else if (utf <= 0x07FF) {
        // 2-byte unicode
        out[0] = (char)(((utf >> 6) & 0x1F) | 0xC0);
        out[1] = (char)(((utf >> 0) & 0x3F) | 0x80);
        out[2] = 0;
        return 2;
    } else if (utf <= 0xFFFF) {
        // 3-byte unicode
        out[0] = (char)(((utf >> 12) & 0x0F) | 0xE0);
        out[1] = (char)(((utf >> 6) & 0x3F) | 0x80);
        out[2] = (char)(((utf >> 0) & 0x3F) | 0x80);
        out[3] = 0;
        return 3;
    } else if (utf <= 0x10FFFF) {
        // 4-byte unicode
        out[0] = (char)(((utf >> 18) & 0x07) | 0xF0);
        out[1] = (char)(((utf >> 12) & 0x3F) | 0x80);
        out[2] = (char)(((utf >> 6) & 0x3F) | 0x80);
        out[3] = (char)(((utf >> 0) & 0x3F) | 0x80);
        out[4] = 0;
        return 4;
    } else {
        // error - use replacement character
        out[0] = (char)0xEF;
        out[1] = (char)0xBF;
        out[2] = (char)0xBD;
        out[3] = 0;
        return 0;
    }
}

static bool isWs(char c) { return c != '\0' && strchr(" \n\r\t", c) != NULL; }

// typedef bool (*parser)(const char** in);

static bool consumeWs(const char** in) {
    for (; isWs(*in[0]); ++*in);
    return true;
}

static bool consumeLiteral(const char** in, const char* lit) {
    const char* start = *in;
    for (; lit[0] != '\0' && *in[0] == lit[0]; ++*in, ++lit);
    *in = lit[0] == '\0' ? *in : start;
    return lit[0] == '\0';
}

static bool consumeSet(const char** in, char* out, const char* set) {
    return *in[0] != '\0' && strchr(set, *in[0]) != NULL &&
           (*out = *in[0], ++*in, true);
}

typedef bool (*valueParser)(const char** in, jacValue* out);
static bool parseValue(const char** in, jacValue* out);

static bool nullParser(const char** in, jacValue* out) {
    if (consumeLiteral(in, "null")) {
        out->type = JAC_TYPE_NULL;
        return true;
    }
    return false;
}

static bool boolParser(const char** in, jacValue* out) {
    out->type = JAC_TYPE_BOOL;
    if (consumeLiteral(in, "true")) {
        out->data.boolean = true;
        return true;
    }
    if (consumeLiteral(in, "false")) {
        out->data.boolean = false;
        return true;
    }
    out->type = JAC_TYPE_UNDEFINED;
    return false;
}

static bool arrayParser(const char** in, jacValue* out) {
    *out = (jacValue){.type = JAC_TYPE_ARRAY,
                      .data.array = MAKE_ARRAY(jacArray, struct jacValue)};
    const char* start = *in;
    jacValue curr = {0};
    if (!consumeLiteral(in, "[")) goto fail;
    consumeWs(in);
    if (consumeLiteral(in, "]")) goto pass;
    if (!parseValue(in, &curr)) goto fail;
    if (!jacArray_append(&out->data.array, curr)) goto fail;
    curr = (jacValue){0};
    while (!consumeLiteral(in, "]")) {
        if (!consumeLiteral(in, ",")) goto fail;
        if (!parseValue(in, &curr)) goto fail;
        if (!jacArray_append(&out->data.array, curr)) goto fail;
        curr = (jacValue){0};
    }
pass:
    return true;
fail:
    jac_freeValue(&curr);
    jac_freeValue(out);
    *out = (jacValue){0};
    *in = start;
    return false;
}

static const char specialMap[] = {
    ['"'] = '"',  ['\\'] = '\\', ['/'] = '/',  ['b'] = '\b',
    ['f'] = '\f', ['n'] = '\n',  ['r'] = '\r', ['t'] = '\t',
};
static const char hexMap[] = {
    ['0'] = 0,  ['1'] = 1,  ['2'] = 2,  ['3'] = 3,  ['4'] = 4,  ['5'] = 5,
    ['6'] = 6,  ['7'] = 7,  ['8'] = 8,  ['9'] = 9,  ['a'] = 10, ['b'] = 11,
    ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15, ['A'] = 10, ['B'] = 11,
    ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,
};

static bool stringParser(const char** in, jacValue* out) {
    CharArray charArray = MAKE_ARRAY(CharArray, char);
    const char* start = *in;
    if (!consumeLiteral(in, "\"")) goto fail;
    char flag = 0;

    while (flag || (!flag && !consumeLiteral(in, "\""))) {
        char c = (*in)++[0];  // this is abuse
        if (c == '\\') {
            c = (*in)++[0];
            if (c == 'u') {
                char hex;
                uint32_t code = 0;

                for (size_t i = 0; i < 4; ++i) {
                    if (!consumeSet(in, &hex, "0123456789abcdefABCDEF"))
                        goto fail;
                    code = code * 16 + hexMap[(size_t)hex];
                }

                char chars[4];
                for (size_t i = 0; i < utf8_encode(chars, code); ++i)
                    if (!CharArray_append(&charArray, chars[i])) goto fail;
            } else {
                if (!(c = specialMap[(size_t)c])) goto fail;
                if (!CharArray_append(&charArray, c)) goto fail;
            }
        } else {
            if (c == '\0') goto fail;
            if (!CharArray_append(&charArray, c)) goto fail;
        }
    }
    CharArray_append(&charArray, '\0');
    *out = (jacValue){.type = JAC_TYPE_STRING,
                      .data = {.string = {.data = charArray.data,
                                          .len = charArray.len - 1,
                                          .isView = false}}};
    return true;
fail:
    *out = (jacValue){0};
    free(charArray.data);
    *in = start;
    return false;
}

static bool objectParser(const char** in, jacValue* out) {
    *out =
        (jacValue){.type = JAC_TYPE_OBJECT, .data = {.object = (jacObject){0}}};
    const char* start = *in;
    if (!consumeLiteral(in, "{")) goto fail;
    consumeWs(in);
    if (consumeLiteral(in, "}")) goto pass;
    jacValue key = {0};
    jacValue value = {0};
    if (!parseValue(in, &key) || key.type != JAC_TYPE_STRING) goto fail;
    if (!consumeLiteral(in, ":")) goto fail;
    if (!parseValue(in, &value)) goto fail;
    if (!jacObject_setjsval(&out->data.object, key.data.string, value))
        goto fail;
    while (!consumeLiteral(in, "}")) {
        if (!consumeLiteral(in, ",")) goto fail;
        if (!parseValue(in, &key) || key.type != JAC_TYPE_STRING) goto fail;
        if (!consumeLiteral(in, ":")) goto fail;
        if (!parseValue(in, &value)) goto fail;
        if (!jacObject_setjsval(&out->data.object, key.data.string, value))
            goto fail;
        key = (jacValue){0};
        value = (jacValue){0};
    }
pass:
    return true;
fail:
    jac_freeValue(&key);
    jac_freeValue(&value);
    jac_freeValue(out);
    *out = (jacValue){0};
    *in = start;
    return false;
}

static bool numberParser(const char** in, jacValue* out) {
    const char* start = *in;
    char d;
    if (!consumeSet(in, &d, "-0123456789")) goto fail;
    if (d == '-' && !consumeSet(in, &d, "0123456789")) goto fail;
    if (d == '0' && !consumeSet(in, &d, "123456789")) goto fail;

    *out = (jacValue){.type = JAC_TYPE_INT,
                      .data.inumber = strtol(start, (char**)in, 10)};
    if (consumeSet(in, &d, ".eE")) {
        if (d == '.' && !consumeSet(in, &d, "0123456789")) goto fail;
        *out = (jacValue){.type = JAC_TYPE_DOUBLE,
                          .data.dnumber = strtod(start, (char**)in)};
        if (*in == start) goto fail;
    }

    return true;
fail:
    *in = start;
    *out = (jacValue){0};
    return false;
}

static valueParser valueParsers[] = {nullParser,   boolParser,   arrayParser,
                                     stringParser, objectParser, numberParser};

static bool parseValue(const char** in, jacValue* out) {
    *out = (jacValue){0};
    const char* next = *in;
    consumeWs(&next);
    for (size_t i = 0; out->type == JAC_TYPE_UNDEFINED &&
                       i < sizeof(valueParsers) / sizeof(valueParser);
         ++i) {
        if (valueParsers[i](&next, out)) {
            consumeWs(&next);
            *in = next;
            return true;
        };
    }
    return false;
}

bool jac_parse(const char* in, jacValue* out) {
    if (!parseValue(&in, out) || in[0] != '\0') {
        *out = (jacValue){0};
        return false;
    }
    return true;
}

jacString jac_encode(jacValue value) {
    CharArray ca = MAKE_ARRAY(CharArray, char);
    switch (value.type) {
        case JAC_TYPE_NULL:
            while ((ca.len = snprintf(ca.data, ca.cap, "null") + 1) > ca.cap)
                CharArray_resize(&ca);
            break;
        case JAC_TYPE_BOOL:
            char* format = value.data.boolean ? "true" : "false";
            while ((ca.len =
                        snprintf(ca.data, ca.cap, format, value.data.inumber) +
                        1) > ca.cap)
                CharArray_resize(&ca);
            break;
        case JAC_TYPE_INT:
            while (
                (ca.len = snprintf(ca.data, ca.cap, "%d", value.data.inumber) +
                          1) > ca.cap)
                CharArray_resize(&ca);
            break;
        case JAC_TYPE_DOUBLE:
            while (
                (ca.len = snprintf(ca.data, ca.cap, "%g", value.data.dnumber) +
                          1) > ca.cap)
                CharArray_resize(&ca);
            break;
        case JAC_TYPE_ARRAY:
            CharArray_append(&ca, '[');
            for (size_t i = 0; i < value.data.array.len; ++i) {
                jacString child = jac_encode(value.data.array.data[i]);
                for (size_t j = 0; j < child.len; ++j)
                    CharArray_append(&ca, child.data[j]);
                if (i + 1 < value.data.array.len) CharArray_append(&ca, ',');
                free(child.data);
            }
            CharArray_append(&ca, ']');
            break;
        case JAC_TYPE_STRING:
            CharArray_append(&ca, '"');
            for (size_t i = 0; i < value.data.string.len; ++i)
                CharArray_append(&ca, value.data.string.data[i]);
            CharArray_append(&ca, '"');
            break;
        case JAC_TYPE_OBJECT:
            CharArray_append(&ca, '{');
            size_t i = 0;
            jacString key = {0};
            jacValue field = {0};
            for (size_t j = 0;
                 j < value.data.object.len &&
                 jacObject_iter(value.data.object, &i, &key, &field);
                 ++j) {
                CharArray_append(&ca, '"');
                for (size_t k = 0; k < key.len; ++k)
                    CharArray_append(&ca, key.data[k]);
                CharArray_append(&ca, '"');
                CharArray_append(&ca, ':');
                jacString child = jac_encode(field);
                for (size_t k = 0; k < child.len; ++k)
                    CharArray_append(&ca, child.data[k]);
                free(child.data);
                if (j + 1 < value.data.object.len) CharArray_append(&ca, ',');
            }
            CharArray_append(&ca, '}');
        default:
            break;
    }
    CharArray_append(&ca, '\0');
    return (jacString){.data = ca.data, .len = ca.len - 1, .isView = false};
};