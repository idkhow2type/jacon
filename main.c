#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"

int main() {
    jacValue value = {.type = JAC_TYPE_OBJECT};
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

    // jacObject_setval(&value.data.object, "abc", value);
    // jacObject_setjs(&value.data.object,
    //               ((jacString){.data = "123", .len = 3, .cap = 0}), 1);
    jacObject_set(&value.data.object, "ggethrytu", NULL);
    jacObject_set(&value.data.object, "blah", "abc");
    char* encoded = jac_encode(value).data;
    printf("%s\n", encoded);
    free(encoded);
    jac_freeValue(&value);

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