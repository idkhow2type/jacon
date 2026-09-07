#include "json.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append(struct JArray* array, JValue value) {
    if (array->len >= array->cap) {
        array->cap *= 2;
        JValue* new = calloc(array->cap, sizeof(JValue));
        memcpy(new, array->data, array->len * sizeof(JValue));
        free(array->data);
        array->data = new;
    }
    array->data[array->len++] = value;
}

void JfreeValue(JValue* value) {
    switch (value->type) {
        case Array:
            for (int i = 0; i < value->data.array.len; i++) {
                JfreeValue(&value->data.array.data[i]);
            }
            free(value->data.array.data);
        default:
            break;
    }
}

static bool isWs(char c) {
    char ws[] = {0x20, 0xA, 0xD, 0x9};
    for (size_t i = 0; i < 4; ++i) {
        if (c == ws[i]) return true;
    }
    return false;
}

/* Define an abstract parser as
char* parser(char* in, void* out?, ...);
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

static const char* consumeLiteral(const char* in, char* lit) {
    const char* start = in;
    for (; lit[0] != '\0' && in[0] == lit[0]; ++in, ++lit);
    return lit[0] == '\0' ? in : start;
}


typedef const char* (*parser)(const char* in, JValue* out);
const char* parseValue(const char* in, JValue* out);

static const char* nullParser(const char* in, JValue* out) {
    const char* new = consumeLiteral(in, "null");
    if (new != in) {
        out->type = Null;
    }
    return new;
}

static const char* boolParser(const char* in, JValue* out) {
    const char* new;
    out->type = Bool;
    if ((new = consumeLiteral(in, "true")) != in) {
        out->data.boolean = true;
    } else if ((new = consumeLiteral(in, "false")) != in) {
        out->data.boolean = false;
    } else {
        out->type = Undefined;
    }
    return new;
}

static const char* arrayParser(const char* in, JValue* out) {
    *out = (JValue){.type = Array,
                    .data = {.array = {.len = 0,
                                       .cap = 256,
                                       .data = calloc(256, sizeof(JValue))}}};
    const char* new;
    const char* checkpoint = in;
    JValue curr = {0};
    if ((new = consumeLiteral(checkpoint, "[")) == checkpoint) goto fail;
    checkpoint = new = consumeWs(new);
    if ((new = consumeLiteral(checkpoint, "]")) != checkpoint) goto pass;
    if ((new = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
    checkpoint = new;
    append(&out->data.array, curr);
    curr = (JValue){0};
    while ((new = consumeLiteral(checkpoint, "]")) == checkpoint) {
        checkpoint = new;
        if ((new = consumeLiteral(checkpoint, ",")) == checkpoint) goto fail;
        checkpoint = new;
        if ((new = parseValue(checkpoint, &curr)) == checkpoint) goto fail;
        checkpoint = new;
        append(&out->data.array, curr);
        curr = (JValue){0};
    }
pass:
    return new;
fail:
    JfreeValue(&curr);
    JfreeValue(out);
    *out = (JValue){0};
    return in;
}

parser valueParsers[] = {nullParser, boolParser, arrayParser};

const char* parseValue(const char* in, JValue* out) {
    *out = (JValue){0};
    const char* new = consumeWs(in);
    for (size_t i = 0;
         out->type == Undefined && i < sizeof(valueParsers) / sizeof(parser);
         ++i) {
        new = valueParsers[i](new, out);
    }
    new = consumeWs(new);
    return out->type != Undefined ? new : in;
}

const char* Jparse(const char* in, JValue* out) {
    const char* new = parseValue(in, out);
    if (new[0] == '\0')
        return new;
    else {
        *out = (JValue){0};
        return in;
    }
}