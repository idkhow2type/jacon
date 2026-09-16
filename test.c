#include <stdio.h>
#include <stdlib.h>

#include "jacon.h"

typedef enum testStatus { ERROR, PASS, FAIL } TestStatus;

/* Parse text to JSON, then render back to text, and print! */
TestStatus parseData(char* data, size_t len, int printParsingResults) {
    jacString in = {.data = data, .len = len};
    jacValue value;
    if (!jac_parsejs(in, &value)) return FAIL;
    jacString s;
    if (!jac_encode(value, &s)) return FAIL;
    if (printParsingResults) {
        printf("-- in: %s", data);
        printf("-- out: %s", s.data);
    }
    jac_freeValue(&value);
    return PASS;
}

/* Read a file, parse, render back, etc. */
TestStatus testFile(const char* filename, int printParsingResults) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        return ERROR;
    };
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = (char*)malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);
    TestStatus status = parseData(data, len, printParsingResults);
    free(data);
    return status;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) return 1;

    const char* path = argv[1];

    int printParsingResults = 0;

    int result = testFile(path, printParsingResults);

    if (result == PASS) {
        return 0;
    } else {
        return 1;
    }
}
