#include <stdio.h>
#include <stdlib.h>

#include "../jacon.h"

// Example: Creating JSON objects and arrays from scratch
int main() {
    printf("=== Creating JSON from scratch ===\n\n");

    // Create a person object with basic fields
    jacValue person = {.type = JAC_TYPE_OBJECT};
    jacObject_set(&person.data.object, "name", "Alice Johnson");
    jacObject_set(&person.data.object, "age", 30);
    jacObject_set(&person.data.object, "active", true);

    // Create an array of hobbies
    jacValue hobbies = {.type = JAC_TYPE_ARRAY};
    jacArray_append(&hobbies.data.array, (jacValue)jacValue_from("Reading"));
    jacArray_append(&hobbies.data.array, (jacValue)jacValue_from("Gaming"));
    jacArray_append(&hobbies.data.array, (jacValue)jacValue_from("Coding"));

    // Add the array to the person object
    jacObject_setval(&person.data.object, "hobbies", hobbies);

    // Create an address sub-object
    jacValue address = {.type = JAC_TYPE_OBJECT};
    jacObject_set(&address.data.object, "street", "123 Main St");
    jacObject_set(&address.data.object, "city", "Portland");
    jacObject_set(&address.data.object, "zip", 97201);

    // Add address to person
    jacObject_setval(&person.data.object, "address", address);

    // Encode and print
    jacString json = jac_encode(person);
    printf("Generated JSON:\n");
    for (size_t i = 0; i < json.len; i++) printf("%c", json.data[i]);
    printf("\n\n");
    
    // Clean up
    free(json.data);
    jac_freeValue(&person);

    return 0;
}
