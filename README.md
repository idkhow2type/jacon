# jacon

fully spec compliant json library

## usage

download `jacon.h` and `jacon.c`, include it, done

## examples

(view more in `examples/`)

```c
#include <stdio.h>
#include <stdlib.h>

#include "jacon.h"

int main() {
    jacValue value;
    if (jac_parse("{\"menu\": {"
                  "  \"id\": \"file\","
                  "  \"value\": \"File\","
                  "  \"popup\": {"
                  "    \"menuitem\": ["
                  "      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},"
                  "      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},"
                  "      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}"
                  "    ]"
                  "  }"
                  "}}",
                  &value)) {
        jacString s;
        if (jac_encode(value, &s)) {
            for (size_t i = 0; i < s.len; i++) printf("%c", s.data[i]);
            printf("\n");
            free(s.data);
        }
    } else
        printf("err\n");
    jac_freeValue(&value);
}
```

stdout

```
{"menu":{"id":"file","popup":{"menuitem":[{"onclick":"CreateNewDoc()","value":"New"},{"onclick":"OpenDoc()","value":"Open"},{"onclick":"CloseDoc()","value":"Close"}]},"value":"File"}}
```

## building

- `make test` to build and run tests
- `make examples` to build examples

## todo

- [ ] support custom text encodings
- [ ] nested object get/set, i.e. `jacObject_get(&obj,"key","subkey",...)`
- [x] custom test harness for library apis instead of just parsing test

## credits

- [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite) for their test cases
- [this gist](https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5) for their UTF-8 encoder
- FNV-1a hash
- various ais for being very annoying
