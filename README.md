# jacon
fully spec compliant json library

## usage
download `jacon.h` and `jacon.c`, include it, done

## examples
(view more in `examples/`)
```c
#include <stdio.h>

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
        jacString s = jac_encode(value);
        for (size_t i = 0; i < s.len; i++) printf("%c", s.data[i]);
        printf("\n");
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

## credits
- [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite) for their test cases and harness (a modified fork is linked in this repo as a submodule)
- [this gist](https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5) for their UTF-8 encoder
- FNV-1a hash
- various ais for being very annoying