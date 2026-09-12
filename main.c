#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

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
    JObject_setcstr2(&value.data.object, "hello", NULL);
    JObject_setcstr2(&value.data.object, "blah", "abc");
    char* encoded = Jencode(value).data;
    printf("%s\n",encoded);
    free(encoded);
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