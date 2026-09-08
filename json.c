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
static int utf8_encode(char* out, uint32_t utf) {
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

/* Define an abstract parser as
const char* parser(const char* in, void* out?, ...);
parsers take in a string and try to perform its parse logic
if success
    parsed value set to out if available
    returns new string with parsed prefix removed
else
    out set to undefined state (depending on type)
    returns in unchanged
*/

static const char* consumeWs(const char* in) {
    for (; isWs(in[0]); ++in);
    return in;
}

static const char* consumeLiteral(const char* in, const char* lit) {
    const char* start = in;
    for (; lit[0] != '\0' && in[0] == lit[0]; ++in, ++lit);
    return lit[0] == '\0' ? in : start;
}

static const char* consumeSet(const char* in, char* out, const char* set) {
    return in[0] != '\0' && strchr(set, in[0]) != NULL ? (*out = in[0], ++in)
                                                       : in;
}

typedef const char* (*parser)(const char* in, JValue* out);
static const char* parseValue(const char* in, JValue* out);

static const char* nullParser(const char* in, JValue* out) {
    const char* next = consumeLiteral(in, "null");
    if (next != in) {
        out->type = Null;
    }
    return next;
}

static const char* boolParser(const char* in, JValue* out) {
    const char* next;
    out->type = Bool;
    if ((next = consumeLiteral(in, "true")) != in) {
        out->data.boolean = true;
    } else if ((next = consumeLiteral(in, "false")) != in) {
        out->data.boolean = false;
    } else {
        out->type = Undefined;
    }
    return next;
}

static const char* arrayParser(const char* in, JValue* out) {
    *out = (JValue){.type = Array,
                    .data.array = MAKE_ARRAY(JArray, struct JValue)};
    const char* next;
    const char* checkpoint = in;
    JValue curr = {0};
    if ((next = consumeLiteral(checkpoint, "[")) == checkpoint) goto fail;
    checkpoint = next = consumeWs(next);
    if ((next = consumeLiteral(checkpoint, "]")) != checkpoint) goto pass;
    if ((next = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
    checkpoint = next;
    if (!JArray_append(&out->data.array, curr)) goto fail;
    curr = (JValue){0};
    while ((next = consumeLiteral(checkpoint, "]")) == checkpoint) {
        checkpoint = next;
        if ((next = consumeLiteral(checkpoint, ",")) == checkpoint) goto fail;
        checkpoint = next;
        if ((next = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
        checkpoint = next;
        if (!JArray_append(&out->data.array, curr)) goto fail;
        curr = (JValue){0};
    }
pass:
    return next;
fail:
    JfreeValue(&curr);
    JfreeValue(out);
    *out = (JValue){0};
    return in;
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

static const char* stringParser(const char* in, JValue* out) {
    JString str = MAKE_ARRAY(JString, char);
    const char* next;
    const char* checkpoint = in;
    if ((next = consumeLiteral(checkpoint, "\"")) == checkpoint) goto fail;
    checkpoint = next;
    char flag = 0;

    while (flag ||
           (!flag && (next = consumeLiteral(checkpoint, "\"")) == checkpoint)) {
        checkpoint = next;
        char c = next[0];
        checkpoint = ++next;
        switch (flag) {
            case 'u':
                // this is kinda hacky
                checkpoint = --next;

                char hex;
                uint32_t code = 0;

                for (size_t i = 0; i < 4; i++) {
                    if ((next = consumeSet(next, &hex,
                                           "0123456789abcdefABCDEF")) ==
                        checkpoint)
                        goto fail;
                    checkpoint = next;
                    code *= 16;
                    code += hexMap[(size_t)hex];
                }

                char chars[4];
                for (int i = 0; i < utf8_encode(chars, code); i++) {
                    JString_append(&str, chars[i]);
                }

                flag = 0;
                break;
            case '\\':
                if (c == 'u') {
                    flag = 'u';
                    continue;
                }
                c = specialMap[(size_t)c];
                if (!c) goto fail;
                JString_append(&str, c);
                flag = 0;
                break;
            default:
                switch (c) {
                    case '\\':
                        flag = c;
                        break;
                    case '\0':
                        goto fail;
                    default:
                        JString_append(&str, c);
                        break;
                }
                break;
        }
    }
    JString_append(&str, '\0');
    *out = (JValue){.type = String, .data = {.string = str}};
    return next;
fail:
    *out = (JValue){0};
    free(str.data);
    return in;
}

static parser valueParsers[] = {nullParser, boolParser, arrayParser,
                                stringParser};

static const char* parseValue(const char* in, JValue* out) {
    *out = (JValue){0};
    const char* next = consumeWs(in);
    for (size_t i = 0;
         out->type == Undefined && i < sizeof(valueParsers) / sizeof(parser);
         ++i) {
        next = valueParsers[i](next, out);
    }
    next = consumeWs(next);
    return out->type != Undefined ? next : in;
}

const char* Jparse(const char* in, JValue* out) {
    const char* next = parseValue(in, out);
    if (next[0] != '\0' || out->type == Undefined) {
        *out = (JValue){0};
        return in;
    }
    return next;
}