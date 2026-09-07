#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

void printValue(const JValue* value) {
    switch (value->type) {
        case Null:
            printf("null");
            break;
        case Bool:
            printf("%s", value->data.boolean ? "true" : "false");
            break;
        case Array:
            printf("[");
            for (int i = 0; i < value->data.array.len; i++) {
                printValue(&value->data.array.data[i]);
                printf(", ");
            }
            printf("]");

            break;

        default:
            break;
    }
}

void freeValue(JValue* value) {
    switch (value->type) {
        case Array:
            for (int i = 0; i < value->data.array.len; i++) {
                freeValue(&value->data.array.data[i]);
            }
            free(value->data.array.data);
        default:
            break;
    }
}

int main() {
    JValue* value = calloc(1, sizeof(JValue));
    char* in = "[true,[false,[null,[null,[null]]]]]";
    char* new = parse(in, value);
    if (in == new) {
        printf("err\n");
    } else {
        printValue(value);
        printf("\n");
    }
    freeValue(value);
    free(value);
}