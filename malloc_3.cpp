//
// Created by Alon zrihan on 2019-06-18.
//

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
    metaData(size_t size):real_size(size),size(size),is_free(false),next(NULL),prev(NULL){}
    //~metaData() = default;
    size_t getSize(){ return size;}
    size_t getRealSize() { return real_size;}
    bool isFree(){ return is_free;}
    void freeData(){is_free= true;}
    void setRealData(size_t size){
        real_size=size;
        is_free= false;
    }
    void setData(size_t size){
        this->size=size;
        if(size>=real_size)
            real_size=size;
        is_free= false;
    }
    void setNext(metaData* next){
        this->next=next;
    }
    void setPrev(metaData* prev){
        this->prev=prev;
    }
    metaData* getNext(){ return next;}
    metaData* getPrev(){ return prev;}
};

class frameList{
    metaData* head;
    metaData* tail;
public:
    frameList():head(NULL),tail(NULL){}
    //~frameList()= default;
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
    void addFramePtr(metaData* src,metaData* to_add){
        to_add->setNext(src->getNext());
        to_add->setPrev(src);
        src->setNext(to_add);
    }
    metaData* getFreeSpace(size_t size){
        metaData* temp=head;
        while(temp){
            if(temp->getRealSize()>=size && temp->isFree())
                return temp;
            temp=temp->getNext();
        }
        if(tail) {
            if (tail->isFree()) {
                if (sbrk(size - tail->getSize()) == (void *) (-1))
                    return NULL;
                return tail;
            }
        }
        return NULL;
    }
    void removePtr(metaData* to_remove){
        if(to_remove==head && to_remove==tail) {
            head = NULL;
            tail=NULL;
            return;
        }
        if(to_remove==head){
            head=to_remove->getNext();
            to_remove->getNext()->setPrev(NULL);
            return;
        }
        if(to_remove==tail){
            tail=to_remove->getPrev();
            to_remove->getPrev()->setNext(NULL);
            return;
        }
        to_remove->getPrev()->setNext(to_remove->getNext());
        to_remove->getNext()->setPrev(to_remove->getPrev());

    }

};

frameList global_list;
bool first_malloc= true;
int real_size_meta = (sizeof(metaData)%4!=0)? sizeof(metaData)+ (4-(sizeof(metaData)%4)):sizeof(metaData);

void splitFrame(metaData* check){
    int new_size=check->getRealSize()-check->getSize()- real_size_meta;
    if(new_size<128)
        return;
    void* newp=(char*)check+real_size_meta+check->getSize();
    metaData temp(new_size);
    temp.freeData();
    *(metaData*)newp=temp;
    check->setRealData(check->getSize());
    global_list.addFramePtr(check,(metaData*)newp);
}
void* malloc(size_t size){
    if(first_malloc){
        void* tst=sbrk(0);
        size_t diff=(size_t)(char*)tst;
        int mod=(int)diff%4;
        if(mod==0)
            mod=4;
        sbrk(4-mod);
        first_malloc= false;
    }
    if(size==0 || size>=Max)
        return NULL;
    int real_size=size;
    if(size%4!=0)
        real_size=size+(4-size%4);

    void* prev;
    metaData* to_add=global_list.getFreeSpace(real_size);
    if(!to_add){
        prev=sbrk(real_size + real_size_meta);
        if(prev==(void*)(-1))
            return NULL;
        *(metaData*)prev=metaData(real_size); // maybe memecopy
        global_list.addFrame((metaData *)prev);
    }else {
        to_add->setData(real_size);
        splitFrame(to_add);
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
    metaData* to_delete=(metaData*)p - 1;//maybe - 1
    metaData* prev=to_delete->getPrev();
    metaData* next=to_delete->getNext();
    if(next) {
        if(next->isFree()){
            to_delete->setData(next->getRealSize()+ real_size_meta+to_delete->getRealSize());
            global_list.removePtr(next);
        }
    }
    if(prev) {
        if(prev->isFree()){
            prev->setData(prev->getRealSize()+ real_size_meta+to_delete->getRealSize());
            global_list.removePtr(to_delete);
            to_delete=prev;
        }
    }
    to_delete->freeData();
    *(metaData*)p=*to_delete;
    return;

}

void* realloc(void* oldp,size_t size){
    if(size==0 || size>=Max)
        return NULL;
    size_t real_size=size;
    if(size%4!=0)
        real_size=size + (4 - (size%4));
    if(!oldp)
        return malloc(real_size);
    metaData* old_data=(metaData*)oldp - 1;//maybe - 1
    int old_size=old_data->getSize();
    if(!old_data->getNext()&&old_data->getSize()<real_size){
        if(sbrk(real_size-old_size)==(void*)(-1))
            return NULL;
        old_data->setData(real_size);
        old_data=(metaData*)old_data+1;
        return old_data;
    }
    if(old_data->getNext()){
        if(old_data->getNext()->isFree()){
            if(old_data->getSize()+old_data->getNext()->getRealSize()+real_size_meta>=real_size){
                old_data->setData(old_data->getNext()->getRealSize()+ real_size_meta+old_data->getRealSize());
                global_list.removePtr(old_data->getNext());
                old_data->setData(real_size);
                splitFrame(old_data);
                old_data=(metaData*)old_data+1;
                return old_data;
            }
        }
    }
    old_data->freeData();
    void* newp=malloc(real_size);
    if(!newp){
        old_data->setData(old_size);
        return NULL;
    }
    memcpy(newp,oldp,real_size);
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
    return counter* real_size_meta;
}

size_t _size_meta_data(){
    return real_size_meta;
}
