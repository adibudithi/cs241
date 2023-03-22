/**
 * Perilous Pointers
 * CS 241 - Fall 2019
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int x = 132;
    int *second = &x;
    second_step(second);

    int** dub2;
    int* dub1;
    x = 8942;
    dub1 = &x;
    dub2 = &dub1;
    double_step(dub2);

    x = 15;
    char* strange = malloc(9 * sizeof(char));
    strange[5] = x;
    strange_step(strange);
    free(strange);

    char* empty = malloc(4 * sizeof(char));
    empty[3] = 0;
    empty_step((void*) empty);
    free(empty);

    char* to = malloc(4 * sizeof(char));
    void* too = (void*) to;
    to[3] = 'u';
    two_step(too, to);
    free(to);

    // three_step(char *first, char *second, char *third)
    char y = 'a';
    three_step(&y, &y + 2, &y + 4);

    char* step = malloc(4 * sizeof(char*));
    step[1] = 0;
    step[2] = 8;
    step[3] = 16;
    step_step_step(step, step, step);
    free(step);

    x = 3;
    char* odd = (char*) &x;
    it_may_be_odd(odd, x);

    // tok_step(char *str);
    char token[] = " ,CS241";
    tok_step(token);

    // the_end(void *orange, void *blue);
    int* end = malloc(sizeof(int));
    *end = 0x801;
    the_end((void*) end, (void*) end); 
    free(end);

    return 0;
}
