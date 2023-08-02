//
// Created by book on 2023/8/2.
//

#ifndef HSTRING_FRANK_ALLOC_H
#define HSTRING_FRANK_ALLOC_H
#include <iostream>
#define ALLOC_SIZE 20
class alloc{
private:
    struct obj{
        obj *next;
    };

private:
    static const int __ALIGN = 8;
    static const int __MAX = 128;
    static const int __NFREELIST = __MAX/__ALIGN;
    static obj* freelist[__NFREELIST];
    static size_t FREELIST_INDEX(size_t bytes){
        return ((bytes+__ALIGN-1)/__ALIGN)-1;
    }

    static size_t ROUND_UP(size_t bytes) {
        return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
    }

    //向chunk_alloc申请一块内存，并把它挂到对应的链表中,最后返回用户要求的内存块
    static void *refill(size_t size);
    //
    static char *chunk_alloc(size_t size,int &nobjs);
    static char *start_free;
    static char *end_free;
    static size_t heap_size;
public:
    static void *allocate(size_t);
    static void deallocate(void *,size_t);
};

void * alloc::allocate(size_t size) {
    size_t index = FREELIST_INDEX(size);
    obj *res ;
    if(freelist[index]== nullptr){
        res = static_cast<obj*>(refill(ROUND_UP(size)));
        return res;
    }
    res = freelist[index];
    freelist[index] = freelist[index]->next;
    return res;
}

void alloc::deallocate(void *ptr, size_t size) {
    size_t index = FREELIST_INDEX(size);
    obj *p = static_cast<obj*>(ptr);
    p->next = freelist[index];
    freelist[index] = p;
}

char *alloc::chunk_alloc(size_t size,int &nobjs)
{
    size_t bytes_left = end_free - start_free;
    size_t total_bytes = nobjs * size;
    char *res = nullptr;
    if(bytes_left>=total_bytes){
        res = start_free;
        start_free += total_bytes;
        return res;
    }else if(bytes_left >= size){
        nobjs = bytes_left/size;
        total_bytes = nobjs * size;
        res = start_free;
        start_free += total_bytes;
        return res;
    } else{
        //处理碎片
        int index;
        if(bytes_left>=8){
            index = FREELIST_INDEX(bytes_left);
            ((obj*)start_free)->next = freelist[index];
            freelist[index] = (obj*)start_free;
        }

        //申请空间
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);

        res = (char *)malloc(bytes_to_get);
        if(res == nullptr){ //如果分配失败
            size_t tmp_size = size;
            for(;tmp_size<=__MAX;tmp_size+=__ALIGN){
                index = FREELIST_INDEX(tmp_size);
                if(freelist[index]!= nullptr){
                    start_free = (char*)freelist[index];
                    end_free = start_free+tmp_size;
                    freelist[index] = freelist[index]->next;
                    return chunk_alloc(size,nobjs);
                }
                if(tmp_size==__MAX){
                    tmp_size = size;
                }
            }
        }
        start_free = res;
        end_free = res+bytes_to_get;
        heap_size+=bytes_to_get;
        return chunk_alloc(size,nobjs);
    }
}

void *alloc::refill(size_t size)
{
    int nobjs = ALLOC_SIZE;
    char *chunk = chunk_alloc(size,nobjs);
    obj *cur = (obj*)(chunk+size);
    if(nobjs == 1)
        return chunk;
    for(int i = 1;i<nobjs-1;i++){
        cur->next = (obj*)((char *)cur+size);
        cur = cur->next;
    }
    cur->next = nullptr;
    freelist[FREELIST_INDEX(size)] = (obj*)(chunk+size);
    return chunk;
}
size_t alloc::heap_size = 0;
char * alloc::start_free = nullptr;
char * alloc::end_free = nullptr;
alloc::obj* alloc::freelist[__NFREELIST] = {0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,};


#endif //HSTRING_FRANK_ALLOC_H
