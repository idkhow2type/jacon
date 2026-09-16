#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jacon.h"

void printJs(jacString s) {
    for (size_t i = 0; i < s.len; i++) {
        printf("%c", s.data[i]);
    }
}

bool jsCmp(jacString a, jacString b) {
    if (a.len != b.len) return false;
    for (size_t i = 0; i < a.len; i++) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}

jacString readFile(const char* filename) {
    // fprintf(stderr,"READING %s\n",filename);
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = (char*)malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);
    return (jacString){.data = data, .len = len, .isView = false};
}

bool endsWith(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) return false;

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s [PATH TO TEST DIR]\n", argv[0]);
        return 1;
    };

    DIR* dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }
    struct dirent* entry;
    char* path = malloc(sizeof(char) * 256);
    while ((entry = readdir(dir)) != NULL) {
        if (!endsWith(entry->d_name, ".json")) continue;
        strcpy(path, argv[1]);
        strcat(path, entry->d_name);
        jacString json = readFile(path);
        jacValue parse;
        char expect = entry->d_name[0];
        char actual = jac_parsejs(json, &parse) ? 'y' : 'n';
        if (expect != 'i' && expect != actual) {
            printf("%s failed: expected %c, got %c\n", entry->d_name, expect,
                   actual);
            continue;
        }
        if (expect != 'y') {
            continue;
        }
        jacString encode;
        if (!jac_encode(parse, &encode)) {
            printf("%s failed: can't encode\n", entry->d_name);
            continue;
        }
        jacValue reparse;
        if (!jac_parsejs(encode, &reparse)) {
            printf("%s failed: can't reparse, encode=", entry->d_name);
            printJs(encode);
            printf("\n");
            continue;
        }
        jacString reencode;
        if (!jac_encode(reparse, &reencode)) {
            printf("%s failed: can't reencode, encode=", entry->d_name);
            printJs(encode);
            printf("\n");
            continue;
        }
        if (!jsCmp(encode, reencode)) {
            printf("%s failed: wrong reencode, encode=", entry->d_name);
            printJs(encode);
            printf(", reencode=");
            printJs(reencode);
            printf("\n");
            continue;
        }
    }
}
