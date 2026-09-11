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

static void testObjects(void) {
    JValue value = parse("{\"name\": \"Ada\", \"active\": true, "
                         "\"flags\": [true, false]}");
    CHECK(value.type == Object);
    CHECK(value.data.object.len == 3);

    JString nameKey = {.data = "name", .len = 5};
    JString activeKey = {.data = "active", .len = 7};
    JString flagsKey = {.data = "flags", .len = 6};
    JValue name = JObject_get(value.data.object, nameKey);
    JValue active = JObject_get(value.data.object, activeKey);
    JValue flags = JObject_get(value.data.object, flagsKey);
    CHECK(name.type == String);
    CHECK(strcmp(name.data.string.data, "Ada") == 0);
    CHECK(active.type == Bool);
    CHECK(active.data.boolean);
    CHECK(flags.type == Array);
    CHECK(flags.data.array.len == 2);

    size_t index = 0;
    size_t iterated = 0;
    JString key = {0};
    JValue field = {0};
    while (JObject_iter(value.data.object, &index, &key, &field)) iterated++;
    CHECK(iterated == value.data.object.len);
    JfreeValue(&value);

    value = parse("{}");
    CHECK(value.type == Object);
    CHECK(value.data.object.len == 0);
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
    expectInvalid("{\"key\" true}");
    expectInvalid("{\"key\": true,}");
}

static void testHashMap(void) {
    JObject object = {0};
    JString firstKey = {.data = "a", .len = 1};
    JString collisionKey = {.data = "e", .len = 1};
    JString thirdKey = {.data = "third", .len = 5};
    JValue first = {.type = Number, .data.number = 1};
    JValue collision = {.type = Number, .data.number = 2};
    JValue third = {.type = Number, .data.number = 3};
    char* allocatedKeys[5] = {0};

    CHECK(JObject_set(&object, firstKey, first));
    CHECK(JObject_set(&object, collisionKey, collision));
    CHECK(JObject_set(&object, thirdKey, third));
    CHECK(object.len == 3);
    CHECK(JObject_get(object, firstKey).data.number == 1);
    CHECK(JObject_get(object, collisionKey).data.number == 2);
    CHECK(JObject_get(object, thirdKey).data.number == 3);

    for (size_t i = 0; i < 5; ++i) {
        char* keyData = malloc(2);
        CHECK(keyData != NULL);
        if (keyData == NULL) continue;
        allocatedKeys[i] = keyData;
        keyData[0] = (char)('f' + i);
        keyData[1] = '\0';
        JString key = {.data = keyData, .len = 1};
        JValue value = {.type = Number, .data.number = 10 + i};
        CHECK(JObject_set(&object, key, value));
    }
    CHECK(object.cap > 4);
    CHECK(object.len == 8);
    CHECK(JObject_get(object, firstKey).data.number == 1);
    CHECK(JObject_get(object, collisionKey).data.number == 2);

    JValue replacement = {.type = Number, .data.number = 99};
    CHECK(JObject_set(&object, firstKey, replacement));
    CHECK(object.len == 8);
    CHECK(JObject_get(object, firstKey).data.number == 99);

    for (size_t i = 0; i < 5; ++i) free(allocatedKeys[i]);
    free(object.data);
}

int main(void) {
    testNull();
    testBooleans();
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
