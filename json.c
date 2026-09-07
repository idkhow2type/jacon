#include "json.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append(struct JArray* array, JValue value) {
    if (array->len >= array->cap) {
        array->cap *= 2;
        if (array->cap == 0) array->cap = 1;
        JValue* new = calloc(array->cap, sizeof(JValue));
        memcpy(new, array->data, array->len * sizeof(JValue));
        if (array->cap > 1) free(array->data);
        array->data = new;
    }
    array->data[array->len++] = value;
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

static char* consumeWs(char* in) {
    for (; isWs(in[0]); ++in);
    return in;
}

static char* consumeLiteral(char* in, char* lit) {
    char* start = in;
    for (; lit[0] != '\0' && in[0] == lit[0]; ++in, ++lit);
    return lit[0] == '\0' ? in : start;
}

typedef char* (*parser)(char* in, JValue* out);

static char* nullParser(char* in, JValue* out) {
    char* new = consumeLiteral(in, "null");
    if (new != in) {
        out->type = Null;
    }
    return new;
}

static char* boolParser(char* in, JValue* out) {
    char* new;
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

static char* arrayParser(char* in, JValue* out) {
    char* new;
    char* checkpoint = in;
    JValue curr = {0};
    if ((new = consumeLiteral(checkpoint, "[")) == checkpoint) return in;
    checkpoint = new = consumeWs(new);
    if ((new = consumeLiteral(checkpoint, "]")) != checkpoint) {
        out->type = Array;
        return new;
    };
    if ((new = parse(checkpoint, &curr)) == checkpoint) return in;
    checkpoint = new;
    append(&out->data.array, curr);
    while ((new = consumeLiteral(checkpoint, "]")) == checkpoint) {
        checkpoint = new;
        if ((new = consumeLiteral(checkpoint, ",")) == checkpoint) return in;
        checkpoint = new;
        curr.type = Undefined;
        if ((new = parse(checkpoint, &curr)) == checkpoint) return in;
        checkpoint = new;
        append(&out->data.array, curr);
    }
    out->type = Array;
    return new;
}

parser valueParsers[] = {nullParser, boolParser, arrayParser};

char* parse(char* in, JValue* out) {
    char* new = consumeWs(in);
    for (size_t i = 0;
         out->type == Undefined && i < sizeof(valueParsers) / sizeof(parser);
         ++i) {
        new = valueParsers[i](new, out);
    }
    new = consumeWs(new);
    return out->type != Undefined ? new : in;
}