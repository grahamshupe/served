/*
General helper functions
*/

#include <string.h>
#include <ctype.h>
#include "util.h"


void str_tolower(char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        str[i] = tolower(str[i]);
    }
}

size_t memcspn(const char* str, const char* reject, size_t size) {
    size_t count = 0;
    size_t reject_len = strlen(reject);
    for (; count < size; count++) {
        for (size_t i = 0; i < reject_len; i++) {
            if (reject[i] == str[count]) {
                return count;
            }
        }
    }
    return count;
}