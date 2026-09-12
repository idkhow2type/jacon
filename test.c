#include "json.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

static jacValue parse(const char* input) {
    jacValue value = {0};
    CHECK(jac_parse(input, &value));
    return value;
}

static void expectInvalid(const char* input) {
    jacValue value = {.type = JAC_TYPE_STRING};
    CHECK(!jac_parse(input, &value));
    CHECK(value.type == JAC_TYPE_UNDEFINED);
}

static void testNull(void) {
    jacValue value = parse(" null \n");
    CHECK(value.type == JAC_TYPE_NULL);
    jac_freeValue(&value);
}

static void testBooleans(void) {
    jacValue trueValue = parse("true");
    jacValue falseValue = parse("\tfalse ");
    CHECK(trueValue.type == JAC_TYPE_BOOL);
    CHECK(trueValue.data.boolean);
    CHECK(falseValue.type == JAC_TYPE_BOOL);
    CHECK(!falseValue.data.boolean);
    jac_freeValue(&trueValue);
    jac_freeValue(&falseValue);
}

static void testGenericValues(void) {
    bool boolInput = true;
    jacValue nullValue = jacValue_from((void*)0);
    jacValue boolValue = jacValue_from(boolInput);
    jacValue intValue = jacValue_from(7);
    jacValue floatValue = jacValue_from(1.5f);
    jacValue doubleValue = jacValue_from(2.5);
    jacValue stringValue = jacValue_from("hello");
    const char* constString = "world";
    jacValue constStringValue = jacValue_from(constString);

    CHECK(nullValue.type == JAC_TYPE_NULL);
    CHECK(boolValue.type == JAC_TYPE_BOOL && boolValue.data.boolean);
    CHECK(intValue.type == JAC_TYPE_DOUBLE && intValue.data.dnumber == 7);
    CHECK(floatValue.type == JAC_TYPE_DOUBLE && floatValue.data.dnumber == 1.5);
    CHECK(doubleValue.type == JAC_TYPE_DOUBLE && doubleValue.data.dnumber == 2.5);
    CHECK(stringValue.type == JAC_TYPE_STRING);
    CHECK(strcmp(stringValue.data.string.data, "hello") == 0);
    CHECK(constStringValue.type == JAC_TYPE_STRING);
    CHECK(strcmp(constStringValue.data.string.data, "world") == 0);

    jacObject object = {0};
    jacString key = {.data = "number", .len = 7};
    CHECK(jacObject_setjs(&object, key, 42));
    CHECK(jacObject_get(object, key).data.number == 42);
    CHECK(jacObject_set(&object, "text", "value"));
    jacString textKey = jacValue_from("text").data.string;
    jacValue value = jacObject_get(object, textKey);
    CHECK(value.type == JAC_TYPE_STRING);
    CHECK(strcmp(value.data.string.data, "value") == 0);
    jac_freeValue(&(jacValue){.type = JAC_TYPE_OBJECT, .data.object = object});
}

static void testStrings(void) {
    jacValue value = parse("\"line\\nquote: \\\"\\\\\\\"\"");
    CHECK(value.type == JAC_TYPE_STRING);
    CHECK(strcmp(value.data.string.data, "line\nquote: \"\\\"") == 0);
    CHECK(value.data.string.len == strlen(value.data.string.data) + 1);
    jac_freeValue(&value);

    value = parse("\"A\\u00e9\\u4e16\"");
    CHECK(value.type == JAC_TYPE_STRING);
    CHECK(strcmp(value.data.string.data, "A\xc3\xa9\xe4\xb8\x96") == 0);
    jac_freeValue(&value);
}

static void testArrays(void) {
    jacValue value = parse("[null, true, \"x\", [false]]");
    CHECK(value.type == JAC_TYPE_ARRAY);
    CHECK(value.data.array.len == 4);
    CHECK(value.data.array.data[0].type == JAC_TYPE_NULL);
    CHECK(value.data.array.data[1].type == JAC_TYPE_BOOL);
    CHECK(value.data.array.data[1].data.boolean);
    CHECK(value.data.array.data[2].type == JAC_TYPE_STRING);
    CHECK(strcmp(value.data.array.data[2].data.string.data, "x") == 0);
    CHECK(value.data.array.data[3].type == JAC_TYPE_ARRAY);
    CHECK(value.data.array.data[3].data.array.len == 1);
    CHECK(value.data.array.data[3].data.array.data[0].type == JAC_TYPE_BOOL);
    CHECK(!value.data.array.data[3].data.array.data[0].data.boolean);
    jac_freeValue(&value);

    value = parse("[]");
    CHECK(value.type == JAC_TYPE_ARRAY);
    CHECK(value.data.array.len == 0);
    jac_freeValue(&value);
}

static void testObjects(void) {
    jacValue value = parse("{\"name\": \"Ada\", \"active\": true, "
                         "\"flags\": [true, false]}");
    CHECK(value.type == JAC_TYPE_OBJECT);
    CHECK(value.data.object.len == 3);

    jacString nameKey = {.data = "name", .len = 5};
    jacString activeKey = {.data = "active", .len = 7};
    jacString flagsKey = {.data = "flags", .len = 6};
    jacValue name = jacObject_get(value.data.object, nameKey);
    jacValue active = jacObject_get(value.data.object, activeKey);
    jacValue flags = jacObject_get(value.data.object, flagsKey);
    CHECK(name.type == JAC_TYPE_STRING);
    CHECK(strcmp(name.data.string.data, "Ada") == 0);
    CHECK(active.type == JAC_TYPE_BOOL);
    CHECK(active.data.boolean);
    CHECK(flags.type == JAC_TYPE_ARRAY);
    CHECK(flags.data.array.len == 2);

    size_t index = 0;
    size_t iterated = 0;
    jacString key = {0};
    jacValue field = {0};
    while (jacObject_iter(value.data.object, &index, &key, &field)) iterated++;
    CHECK(iterated == value.data.object.len);
    jac_freeValue(&value);

    value = parse("{}");
    CHECK(value.type == JAC_TYPE_OBJECT);
    CHECK(value.data.object.len == 0);
    jac_freeValue(&value);
}

static void testInvalidInput(void) {
    expectInvalid("");
    expectInvalid("true trailing");
    expectInvalid("[true,]");
    expectInvalid("[true false]");
    expectInvalid("\"unterminated");
    expectInvalid("\"bad\\q\"");
    expectInvalid("\"bad\\u12\"");
    expectInvalid("{\"key\" true}");
    expectInvalid("{\"key\": true,}");
}

static void testHashMap(void) {
    jacObject object = {0};
    jacString firstKey = {.data = "a", .len = 1};
    jacString collisionKey = {.data = "e", .len = 1};
    jacString thirdKey = {.data = "third", .len = 5};
    jacValue first = {.type = JAC_TYPE_DOUBLE, .data.dnumber = 1};
    jacValue collision = {.type = JAC_TYPE_DOUBLE, .data.dnumber = 2};
    jacValue third = {.type = JAC_TYPE_DOUBLE, .data.dnumber = 3};
    char* allocatedKeys[5] = {0};

    CHECK(jacObject_setjsval(&object, firstKey, first));
    CHECK(jacObject_setjsval(&object, collisionKey, collision));
    CHECK(jacObject_setjsval(&object, thirdKey, third));
    CHECK(object.len == 3);
    CHECK(jacObject_get(object, firstKey).data.number == 1);
    CHECK(jacObject_get(object, collisionKey).data.number == 2);
    CHECK(jacObject_get(object, thirdKey).data.number == 3);

    for (size_t i = 0; i < 5; ++i) {
        char* keyData = malloc(2);
        CHECK(keyData != NULL);
        if (keyData == NULL) continue;
        allocatedKeys[i] = keyData;
        keyData[0] = (char)('f' + i);
        keyData[1] = '\0';
        jacString key = {.data = keyData, .len = 1};
        jacValue value = {.type = JAC_TYPE_DOUBLE, .data.dnumber = 10 + i};
        CHECK(jacObject_setjsval(&object, key, value));
    }
    CHECK(object.cap > 4);
    CHECK(object.len == 8);
    CHECK(jacObject_get(object, firstKey).data.number == 1);
    CHECK(jacObject_get(object, collisionKey).data.number == 2);

    jacValue replacement = {.type = JAC_TYPE_DOUBLE, .data.dnumber = 99};
    CHECK(jacObject_setjsval(&object, firstKey, replacement));
    CHECK(object.len == 8);
    CHECK(jacObject_get(object, firstKey).data.number == 99);

    for (size_t i = 0; i < 5; ++i) free(allocatedKeys[i]);
    free(object.data);
}

int main(void) {
    testNull();
    testBooleans();
    testGenericValues();
    testStrings();
    testArrays();
    testObjects();
    testInvalidInput();
    testHashMap();

    if (failures != 0) {
        fprintf(stderr, "%d test check(s) failed\n", failures);
        return 1;
    }
    puts("all tests passed");
    return 0;
}
