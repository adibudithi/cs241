/**
 * Deadlock Demolition
 * CS 241 - Fall 2019
 */
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    drm_t *drm = drm_init();
    // TODO your tests here
    drm_destroy(drm);

    return 0;
}
