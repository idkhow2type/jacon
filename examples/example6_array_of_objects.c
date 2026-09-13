#include <stdio.h>
#include <stdlib.h>

#include "../jacon.h"

// Example: Working with arrays of objects
int main() {
    printf("=== Working with arrays of objects ===\n\n");

    // Parse JSON with an array of product objects
    const char* json_string =
        "{"
        "  \"store\": \"Tech Shop\","
        "  \"products\": ["
        "    {\"id\": 1, \"name\": \"Laptop\", \"price\": 999.99, \"in_stock\": true},"
        "    {\"id\": 2, \"name\": \"Mouse\", \"price\": 29.99, \"in_stock\": true},"
        "    {\"id\": 3, \"name\": \"Keyboard\", \"price\": 79.99, \"in_stock\": false}"
        "  ]"
        "}";

    printf("Input JSON:\n%s\n\n", json_string);

    jacValue root;
    if (!jac_parse(json_string, &root)) {
        printf("Error: Failed to parse JSON\n");
        return 1;
    }

    // Access store name
    jacValue *store_name = jacObject_get(&root.data.object, "store");
    printf("Store: %s\n\n", store_name->data.string.data);

    // Process products array
    jacValue *products = jacObject_get(&root.data.object, "products");
    printf("Products (%zu total):\n", products->data.array.len);

    for (size_t i = 0; i < products->data.array.len; i++) {
        jacValue product = products->data.array.data[i];

        printf("\nProduct #%zu:\n", i + 1);

        jacValue* id = jacObject_get(&product.data.object, "id");
        printf("  ID: %d\n", id->data.inumber);

        jacValue* name = jacObject_get(&product.data.object, "name");
        printf("  Name: %s\n", name->data.string.data);

        jacValue* price = jacObject_get(&product.data.object, "price");
        printf("  Price: $%.2f\n", price->data.dnumber);

        jacValue* in_stock = jacObject_get(&product.data.object, "in_stock");
        printf("  In Stock: %s\n", in_stock->data.boolean ? "Yes" : "No");
    }

    printf("\n");
    jac_freeValue(&root);

    return 0;
}
