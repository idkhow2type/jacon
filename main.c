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
                if (i + 1 < value->data.array.len) printf(", ");
            }
            printf("]");

            break;

        default:
            break;
    }
}

int main() {
    JValue* value = calloc(1, sizeof(JValue));
    char* in = "[false,null,true]";
    const char* new = Jparse(in, value);
    if (in == new) {
        printf("err\n");
    } else {
        printValue(value);
        printf("\n");
    }
    JfreeValue(value);
    free(value);
}