#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../jacon.h"

void printValue(const jacValue value) {
    jacString s;
    jac_encode(value, &s);
    for (size_t i = 0; i < s.len; i++) {
        printf("%c", s.data[i]);
    }
    free(s.data);
}

void printJs(jacString s) {
    for (size_t i = 0; i < s.len; i++) {
        printf("%c", s.data[i]);
    }
}

bool jsCmp(jacString a, jacString b) {
    if (a.len != b.len) return false;
    for (size_t i = 0; i < a.len; i++) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}

int main() {
    jacValue parse;
    char expect = 'y';
    char actual = jac_parse("[\"\\u0000\"]", &parse) ? 'y' : 'n';
    if (expect != 'i' && expect != actual) {
        printf("expected %c, got %c\n", expect, actual);
        return 1;
    }
    if (expect != 'y') {
        return 1;
    }
    jacString encode;
    if (!jac_encode(parse, &encode)) {
        printf("can't encode\n");
        return 1;
    }
    jacValue reparse;
    if (!jac_parsejs(encode, &reparse)) {
        printf("can't reparse, encode=");
        printJs(encode);
        printf("\n");
        return 1;
    }
    jacString reencode;
    if (!jac_encode(reparse, &reencode)) {
        printf("can't reencode, encode=");
        printJs(encode);
        printf("\n");
        return 1;
    }
    if (!jsCmp(encode, reencode)) {
        printf("wrong reencode, encode=");
        printJs(encode);
        printf(", reencode=");
        printJs(reencode);
        printf("\n");
        return 1;
    }

    return 0;
}
