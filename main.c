#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

void printValue(const JValue value) {
    switch (value.type) {
        case Null:
            printf("null");
            break;
        case Bool:
            printf("%s", value.data.boolean ? "true" : "false");
            break;
        case Array:
            printf("[");
            for (size_t i = 0; i < value.data.array.len; i++) {
                printValue(value.data.array.data[i]);
                if (i + 1 < value.data.array.len) printf(", ");
            }
            printf("]");
            break;
        case String:
            printf("\"");
            for (size_t i = 0; i < value.data.string.len; i++) {
                printf("%c", value.data.string.data[i]);
            }
            printf("\"");
            break;
        case Number:
            printf("%.2f",value.data.number);
            break;
        default:
            break;
    }
}

int main() {
    // JValue* value = calloc(1, sizeof(JValue));
    // // char in[] = {'t', 'r', 'u', 'e'};
    // char in[] = "\"\\u1234\"";
    // if (Jparse(in, value)) {
    //     printValue(value);
    //     printf("\n");
    // } else {
    //     printf("err\n");
    // }
    // JfreeValue(value);
    // free(value);

    JObject object = {.len = 0, .cap = 4, .data = calloc(4, sizeof(ObjectField))};
    JObject_set(&object, (JString){.data = "abc", .len = 4, .cap = 4},
                (JValue){.type = Bool, .data = {.boolean = true}});
    JObject_set(&object, (JString){.data = "abc", .len = 4, .cap = 4},
                (JValue){.type = Bool, .data = {.boolean = false}});
    JObject_set(&object, (JString){.data = "a", .len = 2, .cap = 2},
                (JValue){.type = Null, .data = {0}});
    JObject_set(&object, (JString){.data = "b", .len = 2, .cap = 2},
                (JValue){.type = Number, .data = {.number=-12}});
    JObject_set(&object, (JString){.data = "c", .len = 2, .cap = 2},
                (JValue){.type = Number, .data = {.number=3.14}});
    printValue(
        JObject_get(object, (JString){.data = "abc", .len = 4, .cap = 4}));
        printf("\n");
    printValue(
        JObject_get(object, (JString){.data = "a", .len = 2, .cap = 2}));
        printf("\n");
    printValue(
        JObject_get(object, (JString){.data = "b", .len = 2, .cap = 2}));
        printf("\n");
    printValue(
        JObject_get(object, (JString){.data = "c", .len = 2, .cap = 2}));
        printf("\n");
}