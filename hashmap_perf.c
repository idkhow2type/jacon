#define _POSIX_C_SOURCE 199309L
#include "json.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double elapsedSeconds(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

static size_t parseSize(const char* text, const char* name) {
    char* end = NULL;
    unsigned long long value = strtoull(text, &end, 10);
    if (*text == '\0' || *end != '\0' || value == 0 ||
        value > (unsigned long long)SIZE_MAX) {
        fprintf(stderr, "invalid %s: %s\n", name, text);
        exit(EXIT_FAILURE);
    }
    return (size_t)value;
}

int main(int argc, char** argv) {
    size_t count = argc > 1 ? parseSize(argv[1], "count") : 1000000;
    size_t repetitions = argc > 2 ? parseSize(argv[2], "repetitions") : 5;
    if (argc > 3) {
        fprintf(stderr, "usage: %s [count] [repetitions]\n", argv[0]);
        return EXIT_FAILURE;
    }

    jacString* keys = calloc(count, sizeof(*keys));
    char (*keyData)[32] = calloc(count, sizeof(*keyData));
    if (keys == NULL || keyData == NULL) {
        fprintf(stderr, "could not allocate benchmark data\n");
        free(keys);
        free(keyData);
        return EXIT_FAILURE;
    }

    jacObject object = {0};
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {
        int length = snprintf(keyData[i], sizeof(keyData[i]), "key-%zu", i);
        if (length < 0 || (size_t)length >= sizeof(keyData[i])) {
            fprintf(stderr, "key formatting failed\n");
            return EXIT_FAILURE;
        }
        keys[i] = (jacString){.data = keyData[i], .len = (size_t)length};
        jacValue value = {.type = JAC_TYPE_DOUBLE, .data.dnumber = (double)i};
        if (!jacObject_setjsval(&object, keys[i], value)) {
            fprintf(stderr, "insert failed at key %zu\n", i);
            return EXIT_FAILURE;
        }
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    double insertSeconds = elapsedSeconds(start, end);

    volatile double checksum = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    for (size_t repetition = 0; repetition < repetitions; ++repetition) {
        for (size_t i = 0; i < count; ++i)
            checksum += jacObject_getjs(object, keys[i]).data.dnumber;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    double lookupSeconds = elapsedSeconds(start, end);

    size_t iterated = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    for (size_t repetition = 0; repetition < repetitions; ++repetition) {
        size_t index = 0;
        jacString key = {0};
        jacValue value = {0};
        while (jacObject_iter(object, &index, &key, &value)) {
            checksum += value.data.dnumber;
            ++iterated;
        }
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    double iterationSeconds = elapsedSeconds(start, end);

    printf("hashmap performance (%zu keys, %zu lookup repetitions)\n", count,
           repetitions);
    printf("insert: %.3f ms total, %.1f ns/key\n", insertSeconds * 1000.0,
           insertSeconds * 1e9 / (double)count);
    printf("lookup: %.3f ms total, %.1f ns/lookup\n",
           lookupSeconds * 1000.0,
           lookupSeconds * 1e9 / (double)(count * repetitions));
        printf("iterate: %.3f ms total, %.1f ns/entry\n",
            iterationSeconds * 1000.0,
            iterationSeconds * 1e9 / (double)iterated);
    printf("checksum: %.0f\n", checksum);

    free(object.data);
    free(keyData);
    free(keys);
    return EXIT_SUCCESS;
}
