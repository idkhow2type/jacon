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
            for (size_t i = 0; i < value->data.array.len; i++) {
                printValue(&value->data.array.data[i]);
                if (i + 1 < value->data.array.len) printf(", ");
            }
            printf("]");
            break;
        case String:
            printf("\"");
            for (size_t i = 0; i < value->data.string.len; i++) {
                printf("%c", value->data.string.data[i]);
            }
            printf("\"");

        default:
            break;
    }
}

int main() {
    JValue* value = calloc(1, sizeof(JValue));
    // char in[] = {'t', 'r', 'u', 'e'};
    char in[] = "[true,false,null,[true,[true]]]";
    if (Jparse(in, value)) {
        printValue(value);
        printf("\n");
    } else {
        printf("err\n");
    }
    JfreeValue(value);
    free(value);
}