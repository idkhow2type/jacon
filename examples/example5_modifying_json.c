#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../jacon.h"

// Example: Modifying parsed JSON
int main() {
    printf("=== Modifying parsed JSON ===\n\n");

    const char* original = "{\"user\": {\"name\": \"Bob\", \"age\": 25}}";
    printf("Original JSON:\n%s\n\n", original);

    jacValue root;
    if (!jac_parse(original, &root)) {
        printf("Error: Failed to parse JSON\n");
        return 1;
    }

    // Modify the user object
    jacValue *user = jacObject_get(&root.data.object, "user");
    jacObject_set(&user->data.object, "age", 26);
    jacObject_set(&user->data.object, "email", "bob@example.com");
    jacObject_set(&user->data.object, "verified", true);

    // Add a settings object
    jacValue settings = {.type = JAC_TYPE_OBJECT};
    jacObject_set(&settings.data.object, "theme", "dark");
    jacObject_set(&settings.data.object, "notifications", true);
    jacObject_setval(&root.data.object, "settings", settings);

    // Print the modified JSON
    jacString modified;
    if (jac_encode(root, &modified)) {
        printf("Modified JSON:\n");
        for (size_t i = 0; i < modified.len; i++) printf("%c", modified.data[i]);
        printf("\n\n");
        free(modified.data);
    } else {
        printf("Error: Failed to encode JSON\n");
    }

    jac_freeValue(&root);

    return 0;
}
