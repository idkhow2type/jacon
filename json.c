#include "json.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAP 4
#define MAKE_JArray                            \
    (JValue){                                  \
        .type = Array,                         \
        .data = {.array = {.len = 0,           \
                           .cap = DEFAULT_CAP, \
                           .data = calloc(DEFAULT_CAP, sizeof(JValue))}}};

static bool append(struct JArray* array, JValue value) {
    if (array->len >= array->cap) {
        size_t next_cap = array->cap == 0 ? DEFAULT_CAP : array->cap * 2;
        if (next_cap < array->cap ||
            next_cap > (size_t)-1 / sizeof(*array->data)) {
            return false;
        }

        JValue* next = realloc(array->data, next_cap * sizeof(*next));
        if (next == NULL) return false;

        array->data = next;
        array->cap = next_cap;
    }
    array->data[array->len++] = value;
    return true;
}

void JfreeValue(JValue* value) {
    switch (value->type) {
        case Array:
            for (size_t i = 0; i < value->data.array.len; i++) {
                JfreeValue(&value->data.array.data[i]);
            }
            free(value->data.array.data);
        default:
            break;
    }
    *value = (JValue){0};
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
    *out = MAKE_JArray;
    const char* next;
    const char* checkpoint = in;
    JValue curr = {0};
    if ((next = consumeLiteral(checkpoint, "[")) == checkpoint) goto fail;
    checkpoint = next = consumeWs(next);
    if ((next = consumeLiteral(checkpoint, "]")) != checkpoint) goto pass;
    if ((next = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
    checkpoint = next;
    if (!append(&out->data.array, curr)) goto fail;
    curr = (JValue){0};
    while ((next = consumeLiteral(checkpoint, "]")) == checkpoint) {
        checkpoint = next;
        if ((next = consumeLiteral(checkpoint, ",")) == checkpoint) goto fail;
        checkpoint = next;
        if ((next = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
        checkpoint = next;
        if (!append(&out->data.array, curr)) goto fail;
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

static parser valueParsers[] = {nullParser, boolParser, arrayParser};

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