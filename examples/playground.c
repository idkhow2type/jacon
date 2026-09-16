#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../jacon.h"

void printValue(const jacValue value) {
    jacString s;
    jac_encode(value,&s);
    for (size_t i = 0; i < s.len; i++) {
        printf("%c", s.data[i]);
    }
    free(s.data);
}

int main() {
    jacValue value;
    if (jac_parse("\"\\t\"", &value)) {
        printValue(value);
        printf("\n");
    } else
        printf("err\n");

    return 0;
}
