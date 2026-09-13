#include <assert.h>
#include <stdio.h>

#include "../jacon.h"

void printValue(jacValue value) {
    jacString s = jac_encode(value);
    for (size_t i = 0; i < s.len; i++) printf("%c", s.data[i]);
}

int main() {
    // jacValue value;
    // if (jac_parse("{\"a\":1}", &value)) {
    //     jacValue* sub= jacObject_get(&value.data.object,"123");
    //     assert(sub==NULL);

    //     // printf("\n");
    // } else
    //     printf("err\n");
    // jac_freeValue(&value);
    jacValue value = {.type = JAC_TYPE_OBJECT};
    jacString key = {.data = "abc\x20", .len = 4, .isView = true};
    jacObject_setjs(&value.data.object, key, 123);
    printValue(value);
    printf("\n");
    jac_freeValue(&value);
}