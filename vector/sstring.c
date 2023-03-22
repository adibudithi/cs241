/**
 * Vector
 * CS 241 - Fall 2019
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    vector* string;
};

sstring* sstring_create() {
    sstring* this = malloc(sizeof(sstring));
    this->string = vector_create(char_copy_constructor, char_destructor, char_default_constructor);
    return this;
}

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring* sstr = sstring_create();
    for (size_t i = 0; i < strlen(input); i++) {
        vector_push_back(sstr->string, (void*) input + i);
    }
    return sstr;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char* cstr = malloc((vector_size(input->string) + 1) * sizeof(char));
    for (size_t i = 0; i < vector_size(input->string); i++) {
        cstr[i] = *(char*) vector_get(input->string, i);
    }
    cstr[strlen(cstr)] = '\0';
    return cstr;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    for (size_t i = 0; i < vector_size(addition->string); i++) {
        vector_push_back(this->string, (void*) vector_get(addition->string, i));
    }
    return vector_size(this->string);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector* split = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    sstring* sstr = sstring_create();
    for (size_t i = 0; i < vector_size(this->string); i++) {
        if (*(char*) vector_get(this->string, i) == delimiter) {
            char * cstr = (void*) sstring_to_cstr(sstr);
            vector_push_back(split, cstr);
            vector_clear(sstr->string);
            free(cstr);
        } else {
            vector_push_back(sstr->string, (void*) vector_get(this->string, i));
        }
    }
    char * cstr = sstring_to_cstr(sstr);
    vector_push_back(split, (void*) cstr);
    sstring_destroy(sstr);
    free(cstr);
    return split;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    int count = 0;

    return (count == 0) ? -1 : 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char* slice;
    sstring* sstr = sstring_create();
    for (int i = start; i < end; i++) {
        vector_push_back(sstr->string, vector_get(this->string, i));
    }
    slice = sstring_to_cstr(sstr);
    sstring_destroy(sstr);
    return slice;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->string);
    free(this);
    this = NULL;
}
