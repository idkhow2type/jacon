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
#define DEFINE_ARRAY(Name, T)                                                 \
    bool Name##_append(Name* array, T value) {                                \
        if (array->len >= array->cap) {                                       \
            size_t next_cap = array->cap == 0 ? DEFAULT_CAP : array->cap * 2; \
            if (next_cap < array->cap ||                                      \
                next_cap > (size_t)-1 / sizeof(*array->data)) {               \
                return false;                                                 \
            }                                                                 \
                                                                              \
            T* next = realloc(array->data, next_cap * sizeof(*next));         \
            if (next == NULL) return false;                                   \
                                                                              \
            array->data = next;                                               \
            array->cap = next_cap;                                            \
        }                                                                     \
        array->data[array->len++] = value;                                    \
        return true;                                                          \
    };

DEFINE_ARRAY(JArray, struct JValue);
DEFINE_ARRAY(JString, char);

typedef struct ObjectField {
    JValue value;
    JString key;
} ObjectField;

bool JString_cmp(JString a, JString b) {
    if (a.len != b.len) return false;
    for (size_t i = 0; i < a.len; ++i)
        if (a.data[i] != b.data[i]) return false;
    return true;
}

static uint64_t hash(const char* s, size_t len) {
    uint64_t hash = 14695981039346656037ULL;

    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)*s++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

static bool resize(JObject* object) {
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
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].value.type != Undefined)
            JObject_set(object, old[i].key, old[i].value);
    }
    return true;
}

bool JObject_set(JObject* object, JString key, JValue value) {
    if (object->len >= object->cap - object->cap / 4)
        if (!resize(object)) return false;

    size_t h = (hash(key.data, key.len) & (object->cap - 1));
    bool override = false;
    for (size_t i = 0; object->data[h].value.type != Undefined &&
                       !(override = JString_cmp(object->data[h].key, key));
         i++)
        h = (h + (i + i * i) / 2) % object->cap;

    object->data[h] = (struct ObjectField){.value = value, .key = key};
    object->len += !override;
    return true;
}

JValue JObject_get(const JObject object, JString key) {
    size_t h = (hash(key.data, key.len) & (object.cap - 1));
    for (size_t i = 0; !JString_cmp(object.data[h].key, key); i++)
        h = (h + (i + i * i) / 2) % object.cap;
    return object.data[h].value;
}

void JfreeValue(JValue* value) {
    switch (value->type) {
        case Array:
            for (size_t i = 0; i < value->data.array.len; i++) {
                JfreeValue(&value->data.array.data[i]);
            }
            free(value->data.array.data);
            break;
        case String:
            free(value->data.string.data);
            break;
        default:
            break;
    }
    *value = (JValue){0};
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

typedef bool (*valueParser)(const char** in, JValue* out);
static bool parseValue(const char** in, JValue* out);

static bool nullParser(const char** in, JValue* out) {
    if (consumeLiteral(in, "null")) {
        out->type = Null;
        return true;
    }
    return false;
}

static bool boolParser(const char** in, JValue* out) {
    out->type = Bool;
    if (consumeLiteral(in, "true")) {
        out->data.boolean = true;
        return true;
    }
    if (consumeLiteral(in, "false")) {
        out->data.boolean = false;
        return true;
    }
    out->type = Undefined;
    return false;
}

static bool arrayParser(const char** in, JValue* out) {
    *out = (JValue){.type = Array,
                    .data.array = MAKE_ARRAY(JArray, struct JValue)};
    const char* start = *in;
    JValue curr = {0};
    if (!consumeLiteral(in, "[")) goto fail;
    consumeWs(in);
    if (consumeLiteral(in, "]")) goto pass;
    if (!parseValue(in, &curr)) goto fail;
    if (!JArray_append(&out->data.array, curr)) goto fail;
    curr = (JValue){0};
    while (!consumeLiteral(in, "]")) {
        if (!consumeLiteral(in, ",")) goto fail;
        if (!parseValue(in, &curr)) goto fail;
        if (!JArray_append(&out->data.array, curr)) goto fail;
        curr = (JValue){0};
    }
pass:
    return true;
fail:
    JfreeValue(&curr);
    JfreeValue(out);
    *out = (JValue){0};
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

static bool stringParser(const char** in, JValue* out) {
    JString str = MAKE_ARRAY(JString, char);
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

                for (size_t i = 0; i < 4; i++) {
                    if (!consumeSet(in, &hex, "0123456789abcdefABCDEF"))
                        goto fail;
                    code = code * 16 + hexMap[(size_t)hex];
                }

                char chars[4];
                for (size_t i = 0; i < utf8_encode(chars, code); i++)
                    if (!JString_append(&str, chars[i])) goto fail;
            } else {
                if (!(c = specialMap[(size_t)c])) goto fail;
                if (!JString_append(&str, c)) goto fail;
            }
        } else {
            if (c == '\0') goto fail;
            if (!JString_append(&str, c)) goto fail;
        }
    }
    JString_append(&str, '\0');
    *out = (JValue){.type = String, .data = {.string = str}};
    return true;
fail:
    *out = (JValue){0};
    free(str.data);
    *in = start;
    return false;
}

static valueParser valueParsers[] = {nullParser, boolParser, arrayParser,
                                     stringParser};

static bool parseValue(const char** in, JValue* out) {
    *out = (JValue){0};
    const char* next = *in;
    consumeWs(&next);
    for (size_t i = 0; out->type == Undefined &&
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

bool Jparse(const char* in, JValue* out) {
    if (!parseValue(&in, out) || in[0] != '\0') {
        *out = (JValue){0};
        return false;
    }
    return true;
}