//
// Created by Alon zrihan on 2019-06-17.
//

#include "stdlib.h"
#include <unistd.h>
#include <stdio.h>
#include "string.h"
#define Max 100000000



class metaData{
    size_t real_size;
    size_t size;
    bool is_free;
    metaData* next;
    metaData* prev;
public:
    metaData(size_t size):real_size(size),size(size),is_free(false),next(nullptr),prev(nullptr){}
    ~metaData()= default;
    size_t getSize(){ return size;}
    size_t getRealSize() { return real_size;}
    bool isFree(){ return is_free;}
    void freeData(){is_free= true;}
    void setData(size_t size){
        this->size=size;
        is_free= false;
    }
    void setNext(metaData* next){
        this->next=next;
    }
    void setPrev(metaData* prev){
        this->prev=prev;
    }
    metaData* getNext(){ return next;}
};

class frameList{
    metaData* head;
    metaData* tail;
public:
    frameList():head(nullptr),tail(nullptr){}
    ~frameList()= default;
    metaData* getHead(){ return head;}
    void addFrame(metaData* to_add){
        if(!head){
            head=to_add;
            tail=to_add;
        }else{
            tail->setNext(to_add);
            to_add->setPrev(tail);
            tail=to_add;
        }
    }
    metaData* getFreeSpace(size_t size){
        metaData* temp=head;
        while(temp){
            if(temp->getRealSize()>=size && temp->isFree())
                return temp;
            temp=temp->getNext();
        }
        return NULL;
    }

};

frameList global_list;

void* malloc(size_t size){
    if(size==0 || size>=Max)
        return NULL;
    void* prev;
    metaData* to_add=global_list.getFreeSpace(size);
    if(!to_add){
        prev=sbrk(size + sizeof(metaData));
        if(prev==(void*)(-1))
            return NULL;
        *(metaData*)prev=metaData(size); // maybe memecopy
        global_list.addFrame((metaData *)prev);
    }else {
        to_add->setData(size);
        prev = to_add; //maybe memecopy
    }
    prev=(metaData*)prev+ 1;
    return prev;
}

void* calloc(size_t num,size_t size){
    if(size==0 || size>=Max)
        return NULL;
    void* check=malloc(num*size);
    if(!check)
        return NULL;
    memset(check,0,size);
    return check;
}

void free(void* p){
    if(!p)
        return;
    void* temp=(metaData*)p - 1;//maybe - 1
    metaData to_add=*(metaData*) temp;
    to_add.freeData();
    *(metaData*)temp=to_add;
    metaData test=*((metaData*)p - 1);
    return;

}

void* realloc(void* oldp,size_t size){
    if(size==0 || size>=Max)
        return NULL;
    if(!oldp)
        return malloc(size);
    void* temp=(metaData*)oldp - 1;//maybe - 1
    metaData old_data=*(metaData*) temp;
    old_data.freeData();
    *((metaData*)oldp -1)=old_data;
    int old_size=old_data.getSize();
    void* newp=malloc(size);
    if(!newp){
        old_data.setData(old_size);
        *((metaData*)oldp -1)=old_data;
        return NULL;
    }
    memcpy(newp,oldp,size);
    return newp;
}

size_t _num_free_blocks(){
    int counter=0;
    metaData* temp=global_list.getHead();
    while (temp){
        if(temp->isFree())
            counter++;
        temp=temp->getNext();
    }
    return counter;
}

size_t _num_free_bytes(){
    int counter=0;
    metaData* temp=global_list.getHead();
    while (temp){
        if(temp->isFree())
            counter+=temp->getRealSize();
        temp=temp->getNext();
    }
    return counter;
}

size_t _num_allocated_blocks(){
    int counter=0;
    metaData* temp=global_list.getHead();
    while (temp){
        counter++;
        temp=temp->getNext();
    }
    return counter;
}

size_t _num_allocated_bytes(){
    int counter=0;
    metaData* temp=global_list.getHead();
    while (temp){
        counter+=temp->getRealSize();
        temp=temp->getNext();
    }
    return counter;
}

size_t _num_meta_data_bytes(){
    int counter=_num_allocated_blocks();
    return counter* sizeof(metaData);
}

size_t _size_meta_data(){
    return sizeof(metaData);
}