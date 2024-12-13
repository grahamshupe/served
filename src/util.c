/*
General helper functions
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"


void str_tolower(char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        str[i] = tolower(str[i]);
    }
}

ssize_t memcspn(const char* str, const char* reject, size_t size) {
    ssize_t reject_len = strlen(reject);
    for (ssize_t count = 0; count < size; count++) {
        for (ssize_t i = 0; i < reject_len; i++) {
            if (reject[i] == str[count]) {
                return count;
            }
        }
    }
    return -1;
}

ssize_t memspn(const char* str, const char* accept, size_t size) {
    ssize_t accept_len = strlen(accept);
    for (size_t count = 0; count < size; count++) {
        for (size_t i = 0; i < accept_len; i++) {
            if (accept[i] != str[count]) {
                return count;
            }
        }
    }
}
