//
// Created by Alon zrihan on 2019-06-17.
//
#define Max 100000000
#include "stdlib.h"
#include <unistd.h>

void* malloc(size_t size){
    if(size==0 || size>=Max)
        return NULL;
    void* prev=sbrk(size);
    if(prev==(void*)(-1))
        return NULL;
    return prev;
}