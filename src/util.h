#ifndef PARSE_H_
#define PARSE_H_

/*
Converts the first SIZE characters of STR to lowercase
*/
void str_tolower(char* str, size_t size);

/*
Gets the length of STR to the first occurence of any character in REJECT.
Unlike strcspn(), this only reads a max of SIZE bytes.
*/
size_t memcspn(const char* str, const char* reject, size_t size);

#endif