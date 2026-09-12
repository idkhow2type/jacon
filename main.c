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
            for (size_t i = 0; i < value.data.array.len; ++i) {
                printValue(value.data.array.data[i]);
                if (i + 1 < value.data.array.len) printf(", ");
            }
            printf("]");
            break;
        case String:
            printf("\"");
            for (size_t i = 0; i < value.data.string.len; ++i)
                printf("%c", value.data.string.data[i]);
            printf("\"");
            break;
        case Number:
            printf("%.2f", value.data.number);
            break;
        case Object:
            printf("{");
            size_t i = 0;
            JString key = {0};
            JValue field = {0};
            for (size_t j = 0;
                 j < value.data.object.len &&
                 JObject_iter(value.data.object, &i, &key, &field);
                 ++j) {
                printValue((JValue){.type = String, .data = {.string = key}});
                printf(": ");
                printValue(field);
                if (j + 1 < value.data.object.len) printf(", ");
            }
            printf("}");
        default:
            break;
    }
}

int main() {
    JValue value = {.type = Object};
    // char in[] = {'t', 'r', 'u', 'e'};
    // char in[] = "{\"abc\":true,\"123\":false}";
    // if (Jparse(in, &value)) {
    //     printValue(value);
    //     printf("\n");
    // } else {
    //     printf("err\n");
    // }
    // JObject_setcstr(&value.data.object, "hello",
    //                 );
    JObject_setcstr2(&value.data.object, "hello", 1.1);
    JObject_setcstr2(&value.data.object, "blah", "abc");
    printValue(value);
    printf("\n");
    JfreeValue(&value);
    // free(value);

    // JObject object = {0};
    // JObject_set(&object, (JString){.data = "abc", .len = 4, .cap = 4},
    //             (JValue){.type = Bool, .data = {.boolean = true}});
    // JObject_set(&object, (JString){.data = "abc", .len = 4, .cap = 4},
    //             (JValue){.type = Bool, .data = {.boolean = false}});
    // JObject_set(&object, (JString){.data = "a", .len = 2, .cap = 2},
    //             (JValue){.type = Null, .data = {0}});
    // JObject_set(&object, (JString){.data = "b", .len = 2, .cap = 2},
    //             (JValue){.type = Number, .data = {.number=-12}});
    // JObject_set(&object, (JString){.data = "c", .len = 2, .cap = 2},
    //             (JValue){.type = Number, .data = {.number=3.14}});
    // printValue(
    //     JObject_get(object, (JString){.data = "abc", .len = 4, .cap = 4}));
    //     printf("\n");
    // printValue(
    //     JObject_get(object, (JString){.data = "a", .len = 2, .cap = 2}));
    //     printf("\n");
    // printValue(
    //     JObject_get(object, (JString){.data = "b", .len = 2, .cap = 2}));
    //     printf("\n");
    // printValue(
    //     JObject_get(object, (JString){.data = "c", .len = 2, .cap = 2}));
    //     printf("\n");
}