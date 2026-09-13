#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

int main() {
    jacValue value;
    if (jac_parse("123\0", &value)) {
        jacString s = jac_encode(value);
        for (size_t i = 0; i < s.len; i++) printf("%c", s.data[i]);
        printf("\n");
    } else
        printf("err\n");
}