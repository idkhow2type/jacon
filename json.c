#include "json.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool isWs(char c) {
    char ws[] = {0x20, 0xA, 0xD, 0x9};
    for (size_t i = 0; i < 4; i++) {
        if (c == ws[i]) return true;
    }
    return false;
}

// returns a new char* with leading ws removed
char* consumeWs(char* in) {
    for (; isWs(in[0]); in++);
    return in;
}

// try to match leading characters with lit.
// returns a new char* with lit removed if match
// returns original char* otherwise
char* consumeLiteral(char* in, char* lit) {
    char* start = in;
    for (; lit[0] != '\0' && in[0] == lit[0]; ++in, ++lit);
    return lit[0] == '\0' ? in : start;
}

typedef char* (*parser)(char* in, JValue* out);

char* nullParse(char* in, JValue* out) {
    char* new = consumeLiteral(in, "null");
    if (new != in) {
        out->type = Null;
        out->data = "null";
    }
    return new;
}

char* boolParse(char* in, JValue* out) {
    char* new;
    out->type = Bool;
    out->data = malloc(sizeof(bool));
    if ((new = consumeLiteral(in, "true")) != in) {
        *(bool*)out->data = true;
    } else if ((new = consumeLiteral(in, "false")) != in) {
        *(bool*)out->data = false;
    } else {
        out->type=Undefined;
        free(out->data);
        out->data=NULL;
    }
    return new;
}

parser parsers[] = {nullParse, boolParse};

// 0 = ok, else err
int parse(char* in, JValue* out) {
    in = consumeWs(in);
    char* start = in;
    for (size_t i = 0; in == start && i < sizeof(parsers) / sizeof(parser);
         i++) {
        in = parsers[i](in, out);
    }
    in = consumeWs(in);
    return in == start;
}