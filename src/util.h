#ifndef PARSE_H_
#define PARSE_H_

struct header {
    char* name;
    char* value;
    struct header* next;
};

/*
Converts the first SIZE characters of STR to lowercase
*/
void str_tolower(char* str, size_t size);

/*
Gets the length of STR to the first occurence of any character in REJECT.
Unlike strcspn(), this only reads a max of SIZE bytes.
Returns -1 if none of the specified characters are found.
*/
ssize_t memcspn(const char* str, const char* reject, size_t size);

/*
Gets the length of STR which consists entirely of characters in ACCEPT.
Unlike strspn(), this only reads a max of SIZE bytes.
*/
ssize_t memspn(const char* str, const char* accept, size_t size);


#endif