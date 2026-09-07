#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

int main() {
    JValue* value = calloc(1, sizeof(JValue));
    int err = parse("  fals  \n", value);
    if (err) {
        printf("err\n");
    } else {
        printf("%d\n", *(bool*)value->data);
    }
    free(value);
}