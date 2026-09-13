#include <stdio.h>
#include <stdlib.h>

#include "../jacon.h"

// Example: Iterating over objects and handling different types
int main() {
    printf("=== Iterating objects and type handling ===\n\n");

    const char* json_string =
        "{"
        "  \"string_value\": \"Hello\","
        "  \"int_value\": 42,"
        "  \"float_value\": 3.14,"
        "  \"bool_value\": true,"
        "  \"null_value\": null,"
        "  \"array_value\": [1, 2, 3],"
        "  \"object_value\": {\"nested\": \"data\"}"
        "}";

    printf("Input JSON:\n%s\n\n", json_string);

    jacValue root;
    if (!jac_parse(json_string, &root)) {
        printf("Error: Failed to parse JSON\n");
        return 1;
    }

    // Iterate over object fields
    printf("Iterating over object fields:\n");
    size_t i = 0;
    jacString key;
    jacValue value;
    while (jacObject_iter(root.data.object, &i, &key, &value)) {
        printf("  Key: ");
        for (size_t j = 0; j < key.len; j++) printf("%c", key.data[j]);
        printf(" -> Type: ");

        // Handle each type
        switch (value.type) {
            case JAC_TYPE_STRING:
                printf("string(%s)\n", value.data.string.data);
                break;
            case JAC_TYPE_INT:
                printf("int(%d)\n", value.data.inumber);
                break;
            case JAC_TYPE_DOUBLE:
                printf("double(%f)\n", value.data.dnumber);
                break;
            case JAC_TYPE_BOOL:
                printf("bool(%s)\n", value.data.boolean ? "true" : "false");
                break;
            case JAC_TYPE_NULL:
                printf("null\n");
                break;
            case JAC_TYPE_ARRAY:
                printf("array(%zu elements)\n", value.data.array.len);
                break;
            case JAC_TYPE_OBJECT:
                printf("object(%zu fields)\n", value.data.object.len);
                break;
            default:
                printf("unknown\n");
        }
    }

    printf("\n");
    jac_freeValue(&root);

    return 0;
}
