#include <stdio.h>
#include <stdlib.h>

#include "../jacon.h"

int main() {
    jacValue value;
    if (jac_parse("{\"menu\": {"
                  "  \"id\": \"file\","
                  "  \"value\": \"File\","
                  "  \"popup\": {"
                  "    \"menuitem\": ["
                  "      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},"
                  "      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},"
                  "      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}"
                  "    ]"
                  "  }"
                  "}}",
                  &value)) {
        jacString s = jac_encode(value);
        for (size_t i = 0; i < s.len; i++) printf("%c", s.data[i]);
        printf("\n");
        free(s.data);
    } else
        printf("err\n");
    jac_freeValue(&value);
}