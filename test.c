#include "json.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, \
                    #condition);                                              \
            failures++;                                                        \
        }                                                                      \
    } while (0)

static JValue parse(const char* input) {
    JValue value = {0};
    CHECK(Jparse(input, &value));
    return value;
}

static void expectInvalid(const char* input) {
    JValue value = {.type = String};
    CHECK(!Jparse(input, &value));
    CHECK(value.type == Undefined);
}

static void testNull(void) {
    JValue value = parse(" null \n");
    CHECK(value.type == Null);
    JfreeValue(&value);
}

static void testBooleans(void) {
    JValue trueValue = parse("true");
    JValue falseValue = parse("\tfalse ");
    CHECK(trueValue.type == Bool);
    CHECK(trueValue.data.boolean);
    CHECK(falseValue.type == Bool);
    CHECK(!falseValue.data.boolean);
    JfreeValue(&trueValue);
    JfreeValue(&falseValue);
}

static void testStrings(void) {
    JValue value = parse("\"line\\nquote: \\\"\\\\\\\"\"");
    CHECK(value.type == String);
    CHECK(strcmp(value.data.string.data, "line\nquote: \"\\\"") == 0);
    CHECK(value.data.string.len == strlen(value.data.string.data) + 1);
    JfreeValue(&value);

    value = parse("\"A\\u00e9\\u4e16\"");
    CHECK(value.type == String);
    CHECK(strcmp(value.data.string.data, "A\xc3\xa9\xe4\xb8\x96") == 0);
    JfreeValue(&value);
}

static void testArrays(void) {
    JValue value = parse("[null, true, \"x\", [false]]");
    CHECK(value.type == Array);
    CHECK(value.data.array.len == 4);
    CHECK(value.data.array.data[0].type == Null);
    CHECK(value.data.array.data[1].type == Bool);
    CHECK(value.data.array.data[1].data.boolean);
    CHECK(value.data.array.data[2].type == String);
    CHECK(strcmp(value.data.array.data[2].data.string.data, "x") == 0);
    CHECK(value.data.array.data[3].type == Array);
    CHECK(value.data.array.data[3].data.array.len == 1);
    CHECK(value.data.array.data[3].data.array.data[0].type == Bool);
    CHECK(!value.data.array.data[3].data.array.data[0].data.boolean);
    JfreeValue(&value);

    value = parse("[]");
    CHECK(value.type == Array);
    CHECK(value.data.array.len == 0);
    JfreeValue(&value);
}

static void testInvalidInput(void) {
    expectInvalid("");
    expectInvalid("true trailing");
    expectInvalid("[true,]");
    expectInvalid("[true false]");
    expectInvalid("\"unterminated");
    expectInvalid("\"bad\\q\"");
    expectInvalid("\"bad\\u12\"");
}

int main(void) {
    testNull();
    testBooleans();
    testStrings();
    testArrays();
    testInvalidInput();

    if (failures != 0) {
        fprintf(stderr, "%d test check(s) failed\n", failures);
        return 1;
    }
    puts("all tests passed");
    return 0;
}
