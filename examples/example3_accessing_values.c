#include <stdio.h>
#include <stdlib.h>

#include "../jacon.h"

// Example: Parsing JSON and accessing nested values
int main() {
    printf("=== Parsing and accessing nested JSON ===\n\n");

    const char* json_string =
        "{"
        "  \"database\": {"
        "    \"host\": \"localhost\","
        "    \"port\": 5432,"
        "    \"credentials\": {"
        "      \"username\": \"admin\","
        "      \"password\": \"secret123\""
        "    },"
        "    \"tables\": [\"users\", \"posts\", \"comments\"]"
        "  }"
        "}";

    printf("Input JSON:\n%s\n\n", json_string);

    jacValue root;
    if (!jac_parse(json_string, &root)) {
        printf("Error: Failed to parse JSON\n");
        return 1;
    }

    // Access the database object
    jacValue *db = jacObject_get(&root.data.object, "database");
    printf("Database host: %s\n",
           jacObject_get(&db->data.object, "host")->data.string.data);

    // Access nested credentials
    jacValue *creds = jacObject_get(&db->data.object, "credentials");
    printf("Username: ");
    jacValue *username = jacObject_get(&creds->data.object, "username");
    if (username->type == JAC_TYPE_STRING) {
        for (size_t i = 0; i < username->data.string.len; i++) {
            printf("%c", username->data.string.data[i]);
        }
        printf("\n");
    }

    // Access the tables array
    jacValue *tables = jacObject_get(&db->data.object, "tables");
    printf("Tables (%zu total):\n", tables->data.array.len);
    for (size_t i = 0; i < tables->data.array.len; i++) {
        printf("  - %s\n", tables->data.array.data[i].data.string.data);
    }

    printf("\n");
    jac_freeValue(&root);

    return 0;
}
