/**
 * Vector
 * CS 241 - Fall 2019
 */
#include "vector.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Write your test cases here

    // initialization test
    vector* vec = vector_create(NULL, NULL, NULL);
    printf("empty vector size = %zu\n", vector_size(vec));
    printf("empty vector capacity = %zu\n", vector_capacity(vec));

    // add a null element test
    vector_push_back(vec, NULL);

    printf("--pushing null--\n");
    printf("pushed null vector size = %zu\n", vector_size(vec));
    printf("pushed null vector capacity = %zu\n", vector_capacity(vec));

    // remove a null element from back test
    vector_pop_back(vec);

    printf("--popping null--\n");
    printf("popped null vector size = %zu\n", vector_size(vec));
    printf("popped null empty vector capacity = %zu\n", vector_capacity(vec));

    // add a char element test
    char let = 'a';
    char* let_ptr = &let;
    vector_push_back(vec, let_ptr);

    printf("--pushing 1 char--\n");
    printf("first element = %c\n", *((char*) vector_get(vec, 0)));
    printf("pushed 1 char vector size = %zu\n", vector_size(vec));
    printf("pushed 1 char vector capacity = %zu\n", vector_capacity(vec));

    // remove a char from back element test
    vector_pop_back(vec);

    printf("--popping 1 char--\n");
    printf("popped 1 char vector size = %zu\n", vector_size(vec));
    printf("popped 1 char empty vector capacity = %zu\n", vector_capacity(vec));

    // add an int element test
    int num = 0;
    int* num_ptr = &num;
    vector_push_back(vec, num_ptr);

    printf("--pushing 1 int--\n");
    printf("first element = %d\n", *((int*) vector_get(vec, 0)));
    printf("pushed 1 int vector size = %zu\n", vector_size(vec));
    printf("pushed 1 int vector capacity = %zu\n", vector_capacity(vec));

    // remove an int element from back test
    vector_pop_back(vec);
    
    printf("--popping 1 int--\n");
    printf("popped 1 int vector size = %zu\n", vector_size(vec));
    printf("popped 1 int empty vector capacity = %zu\n", vector_capacity(vec));

    // add 8 elements test
    int one = 1;
    int two = 2; 
    int three = 3;
    int four = 4;
    int five = 5;
    int six = 6;
    int seven = 7;
    int eight = 8;
    int* one_ptr = &one;
    int* two_ptr = &two;
    int* three_ptr = &three;
    int* four_ptr = &four;
    int* five_ptr = &five;
    int* six_ptr = &six;
    int* seven_ptr = &seven;
    int* eight_ptr = &eight;
    vector_push_back(vec, one_ptr);
    vector_push_back(vec, two_ptr);
    vector_push_back(vec, three_ptr);
    vector_push_back(vec, four_ptr);
    vector_push_back(vec, five_ptr);
    vector_push_back(vec, six_ptr);
    vector_push_back(vec, seven_ptr);
    vector_push_back(vec, eight_ptr);

    printf("--pushing 8 ints--\n");
    printf("pushed 8 ints vector size = %zu\n", vector_size(vec));
    printf("pushed 8 ints vector capacity = %zu\n", vector_capacity(vec));
    
    // access 8 elements test
    printf("--accessing 8 ints--\n");
    printf("first element = %d\n", *((int*) vector_get(vec, 0)));
    printf("second element = %d\n", *((int*) vector_get(vec, 1)));
    printf("third element = %d\n", *((int*) vector_get(vec, 2)));
    printf("fourth element = %d\n", *((int*) vector_get(vec, 3)));
    printf("fifth element = %d\n", *((int*) vector_get(vec, 4)));
    printf("sixth element = %d\n", *((int*) vector_get(vec, 5)));
    printf("seventh element = %d\n", *((int*) vector_get(vec, 6)));
    printf("eighth element = %d\n", *((int*) vector_get(vec, 7)));

    // insert an int element test
    int nine = 9;
    int* nine_ptr = &nine;
    vector_insert(vec, 4, nine_ptr);

    printf("--inserting 1 int--\n");
    printf("first element = %d\n", *((int*) vector_get(vec, 0)));
    printf("second element = %d\n", *((int*) vector_get(vec, 1)));
    printf("third element = %d\n", *((int*) vector_get(vec, 2)));
    printf("fourth element = %d\n", *((int*) vector_get(vec, 3)));
    printf("fifth element = %d\n", *((int*) vector_get(vec, 4)));
    printf("sixth element = %d\n", *((int*) vector_get(vec, 5)));
    printf("seventh element = %d\n", *((int*) vector_get(vec, 6)));
    printf("eighth element = %d\n", *((int*) vector_get(vec, 7)));
    printf("ninth element = %d\n", *((int*) vector_get(vec, 8)));

    printf("inserted 1 int vector size = %zu\n", vector_size(vec));
    printf("inserted 1 int empty vector capacity = %zu\n", vector_capacity(vec));

    // change an element test
    int ten = 10;
    int* ten_ptr = &ten;
    vector_set(vec, 7, ten_ptr);
    printf("--setting 1 int--\n");
    printf("first element = %d\n", *((int*) vector_get(vec, 0)));
    printf("second element = %d\n", *((int*) vector_get(vec, 1)));
    printf("third element = %d\n", *((int*) vector_get(vec, 2)));
    printf("fourth element = %d\n", *((int*) vector_get(vec, 3)));
    printf("fifth element = %d\n", *((int*) vector_get(vec, 4)));
    printf("sixth element = %d\n", *((int*) vector_get(vec, 5)));
    printf("seventh element = %d\n", *((int*) vector_get(vec, 6)));
    printf("eighth element = %d\n", *((int*) vector_get(vec, 7)));
    printf("ninth element = %d\n", *((int*) vector_get(vec, 8)));
    printf("set 1 int vector size = %zu\n", vector_size(vec));
    printf("set 1 int empty vector capacity = %zu\n", vector_capacity(vec));

    // remove an element test
    vector_erase(vec, 2);
    printf("--erasing 1 int--\n");
    printf("first element = %d\n", *((int*) vector_get(vec, 0)));
    printf("second element = %d\n", *((int*) vector_get(vec, 1)));
    printf("third element = %d\n", *((int*) vector_get(vec, 2)));
    printf("fourth element = %d\n", *((int*) vector_get(vec, 3)));
    printf("fifth element = %d\n", *((int*) vector_get(vec, 4)));
    printf("sixth element = %d\n", *((int*) vector_get(vec, 5)));
    printf("seventh element = %d\n", *((int*) vector_get(vec, 6)));
    printf("eighth element = %d\n", *((int*) vector_get(vec, 7)));
    printf("erase 1 int vector size = %zu\n", vector_size(vec));
    printf("erase 1 int empty vector capacity = %zu\n", vector_capacity(vec));

    // free vector test
    vector_destroy(vec);
    printf("vector destroyed\n");

    printf("<(O_O)>\n");

    return 0;
}
