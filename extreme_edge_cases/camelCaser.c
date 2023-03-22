/**
 * Extreme Edge Cases
 * CS 241 - Fall 2019
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (!input_str) return NULL;
    int camel_spot = 0;
    int camel_len = 10;

    int input_i = 0;
    int input_len = strlen(input_str);
    char** camel = malloc(camel_len * sizeof(char*));

    if (!camel) return NULL;

    while (input_i < input_len) {

        int word_spot = 0;
        int word_len = 10;
        char* word = (char*) calloc(word_len, sizeof(char));
        
        int upper = 0;
        int alpha = 0;
        while (input_i < input_len && !ispunct(input_str[input_i])) {

            if (isspace(input_str[input_i])) {
                if (alpha) {
                    upper = 1;
                }
                while (input_i < input_len && isspace(input_str[input_i])) {
                    input_i++;
                }
                continue;
            }

            if (isalpha(input_str[input_i])) {
                alpha = 1;
                if (upper) {
                    word[word_spot++] = toupper(input_str[input_i]);
                    upper = 0;
                } else {
                    word[word_spot++] = tolower(input_str[input_i]);
                }
            } else {
                alpha = 1;
                word[word_spot++] = input_str[input_i];
            }

            if (word_spot == word_len) {
                word_len *= 2;
                word = realloc(word, word_len * sizeof(char));
            }

            input_i++;
        }
        
        word[word_spot++] = 0;
        camel[camel_spot++] = word;
        if (camel_spot == camel_len) {
            camel_len *= 2;
            camel = realloc(camel, camel_len * sizeof(char*));
        }
        input_i++;
        
    }
    camel[camel_spot++] = 0;

    if (!ispunct(input_str[input_len - 1])) {
        free(camel[camel_spot - 2]);
        camel[camel_spot - 2] = NULL;
    }


    return camel;
}

void destroy(char **result) {
    // TODO: Implement me!
    char** curr = result;

    while (*curr) {
        free(*curr);
        curr++;
    }
    free(result);
    return;
}
