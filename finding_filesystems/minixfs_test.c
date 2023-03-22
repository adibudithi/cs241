/**
* Finding Filesystems Lab
* CS 241 - Fall 2018
*/

#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void simple_create_inode_test(file_system* fs);
void create_multiple_inodes_test(file_system* fs);
void create_a_shit_ton_of_inodes_test(file_system* fs, size_t TEST_AMOUNT);
void reading_and_writing_from_files_basic_test(file_system* fs);
void reading_and_writing_from_files_overwrite_test(file_system* fs);
void test_reading_at_offset(file_system* fs);
void test_virtual_read(file_system* fs);


int main(int argc, char *argv[]) {
    // Write tests here!
    printf("\n\n-------TESTING-------\n\n");
    file_system* fs = open_fs(argv[1]);
    simple_create_inode_test(fs);
    create_multiple_inodes_test(fs);
    create_a_shit_ton_of_inodes_test(fs, 100);
    reading_and_writing_from_files_basic_test(fs);
    reading_and_writing_from_files_overwrite_test(fs);
    test_reading_at_offset(fs);
    test_virtual_read(fs);
}

void simple_create_inode_test(file_system* fs) {

    assert(get_inode(fs, "/to_create") == NULL); 
    inode *created = minixfs_create_inode_for_path(fs, "/to_create");
    inode *expected = get_inode(fs, "/to_create"); 

    assert(created == expected);
    assert(created != NULL);
    minixfs_create_inode_for_path(fs, "/to_create");

    printf("[TEST 1] Passed creating one inode!\n\n");
}

void create_multiple_inodes_test(file_system* fs) {

    assert(get_inode(fs, "/inode_a") == NULL);
    assert(get_inode(fs, "/inode_b") == NULL);

    inode *inode_1 = minixfs_create_inode_for_path(fs, "/inode_a");
    inode *inode_2 = minixfs_create_inode_for_path(fs, "/inode_b");

    inode *expected_1 = get_inode(fs, "/inode_a"); // should be the same as created
    inode *expected_2 = get_inode(fs, "/inode_b"); // should be the same as created

    assert(inode_1 == expected_1);
    assert(inode_2 == expected_2);
    assert(minixfs_create_inode_for_path(fs, "/inode_a") == NULL);
    assert(minixfs_create_inode_for_path(fs, "/inode_b") == NULL);

    printf("[TEST 2] Passed creating two inodes!\n\n");
}

void create_a_shit_ton_of_inodes_test(file_system* fs, size_t TEST_AMOUNT) {
    inode* created[TEST_AMOUNT];
    inode* expected[TEST_AMOUNT];
    for (size_t i = 0; i < TEST_AMOUNT; i++) {
        char name[20];
        sprintf(name, "/inode_%lu", i);
        assert(get_inode(fs, name) == NULL);
        created[i] = minixfs_create_inode_for_path(fs, name);
        assert(created[i] != NULL);
    }

    for (size_t i = 0; i < TEST_AMOUNT; i++) {
        char name[20];
        sprintf(name, "/inode_%lu", i);
        expected[i] = get_inode(fs, name);
    }
    
    for (size_t i = 0; i < TEST_AMOUNT; i++) {
        assert(created[i] == expected[i]);
        char name[20];
        sprintf(name, "/inode_%lu", i);
        assert(minixfs_create_inode_for_path(fs, name) == NULL);
    }

    printf("[TEST 3] Passed creating %lu inodes!\n\n", TEST_AMOUNT);
}

void reading_and_writing_from_files_basic_test(file_system* fs) {

    const char* buffer = strdup("This is a long string to test that we can write it");
    off_t wr_offset = 0;
    ssize_t wr_res = minixfs_write(fs, "/write_test_1", (const void*) buffer, strlen(buffer), &wr_offset);
    assert(wr_res == (ssize_t)strlen(buffer));
    assert(wr_offset == (off_t)strlen(buffer));
    printf("[TEST 4] Writing sets result and offset correctly\n");
    
    char buf[10000];
    off_t r_offset = 0;
    ssize_t r_res = minixfs_read(fs, "/write_test_1", &buf, strlen(buffer), &r_offset);
    assert(r_res == (ssize_t)strlen(buffer));
    assert(r_offset == (off_t)strlen(buffer));
    printf("[TEST 4] Reading sets result and offset correctly\n");

    // printf("buf = %s and buffer = %s\n", buf, buffer);

    assert(strcmp(buf, buffer) == 0);
    printf("[TEST 4] Buffer read/written matches for a length of %lu\n\n", strlen(buffer));
    
}

void reading_and_writing_from_files_overwrite_test(file_system* fs) {

    char* buffer = strdup("This is a long string to test that we can write it");
    off_t wr_offset = 0;
    ssize_t wr_res = minixfs_write(fs, "/write_test_2", (const void*) buffer, strlen(buffer), &wr_offset);
    assert(wr_res == (ssize_t)strlen(buffer));
    assert(wr_offset == (off_t)strlen(buffer));
    printf("[TEST 5] Writing sets result and offset correctly\n");
    
    char* buffer2 = strdup("short string that will overwrite the original string to make it a longer length 🙂");
    off_t wr_offset2 = 10;
    ssize_t wr_res2 = minixfs_write(fs, "/write_test_2", (const void*) buffer2, strlen(buffer2), &wr_offset2);
    assert(wr_res2 == (ssize_t)strlen(buffer2));
    assert(wr_offset2 == (off_t) (10 + strlen(buffer2)));
    printf("[TEST 5] Overwriting sets result and offset correctly\n");

    char buf[10000];
    memset(&buf, 0, 10000);
    char* expected = strdup("This is a short string that will overwrite the original string to make it a longer length 🙂");
    off_t r_offset = 0;
    ssize_t r_res = minixfs_read(fs, "/write_test_2", &buf, 10000, &r_offset);
    assert(r_res == (ssize_t) strlen(expected));
    assert(r_offset == (off_t)strlen(expected));
    printf("[TEST 5] Reading sets result and offset correctly\n");
    assert(strcmp(buf, expected) == 0);
    printf("[TEST 5] Buffer read/written overwriting matches for a length of %lu\n\n", strlen(buf));
    

    free(buffer);
    free(buffer2);
    free(expected);
    
}

void test_reading_at_offset(file_system* fs) {
    char* buffer = strdup("This is a long string to test that we can write it");
    off_t wr_offset = 0;
    ssize_t wr_res = minixfs_write(fs, "/read_test_1", (const void*) buffer, strlen(buffer), &wr_offset);
    assert(wr_res == (ssize_t)strlen(buffer));
    assert(wr_offset == (off_t)strlen(buffer));
    printf("[TEST 6] Writing sets result and offset correctly\n");
    
    char buf3[10000];
    memset(&buf3, 0, 10000);
    char* expected3 = strdup("This is a short string that will overwrite the original string to make it a longer length 🙂");
    off_t r_offset3 = 0;
    ssize_t r_res3 = minixfs_read(fs, "/write_test_2", &buf3, 10000, &r_offset3);
    assert(r_res3 == (ssize_t) strlen(expected3));
    assert(r_offset3 == (off_t)strlen(expected3));
    printf("[TEST 6] Rereading after other write sets result and offset correctly\n");
    assert(strcmp(buf3, expected3) == 0);
    printf("[TEST 6] Buffer read/written second time matches for a length of %lu\n", strlen(buf3));

    char buf[10000];
    memset(&buf, 0, 10000);
    char* expected = strdup("is a long string to test that we can write it");
    off_t r_offset = 5;
    ssize_t r_res = minixfs_read(fs, "/read_test_1", &buf, 10000, &r_offset);
    assert(r_res == (ssize_t) (strlen(expected)));
    assert(r_offset == (off_t) strlen(buffer));
    printf("[TEST 6] Reading at offset sets result and offset correctly\n");
    // fprintf(stderr, "%s\n", buf);
    assert(strcmp(buf, expected) == 0);
    printf("[TEST 6] Buffer read at offset matches for a length of %lu\n", strlen(buf));

    char buf2[10000];
    memset(&buf2, 0, 10000);
    char* expected2 = strdup("is a long string to test that we can write");
    off_t r_offset2 = 5;
    ssize_t r_res2 = minixfs_read(fs, "/read_test_1", &buf2, strlen(expected2), &r_offset2);
    assert(r_res2 == (ssize_t) (strlen(expected2)));
    assert(r_offset2 == (off_t) (5 + strlen(expected2)));
    printf("[TEST 6] Reading with count sets result and offset correctly\n");
    assert(strcmp(buf2, expected2) == 0);
    printf("[TEST 6] Buffer read with count matches for a length of %lu\n\n", strlen(buf));

    free(expected);
    free(expected2);
    free(buffer);
}

void test_virtual_read(file_system* fs) {
    printf("[TEST 7]\n");
    char buf[10000];
    memset(&buf, 0, 10000);
    off_t vr_offset = 0;
    minixfs_virtual_read(fs, "info", &buf, 10000, &vr_offset);
    printf("%s\n", buf);
}