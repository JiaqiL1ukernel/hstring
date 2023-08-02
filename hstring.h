#ifndef __HSTRING_H
#define __HSTRING_H
#include <unistd.h>
#include "frank_alloc.h"
#include <iostream>
#include <cstring>
class hstring {
    typedef unsigned int size_t;
private:
    char *str;
    size_t len;

    size_t set_len(const char *str) {
        len = 0;
        while (*str++ != '\0') ++len;
        ++len;
        return len;
    }

    size_t get_alloc_size()
    {
        return ((len + 7)/8)*8;
    }

    size_t get_len(const char* c)
    {
        size_t len = 0;
        while(*c++ != '\0') ++len;
        return len + 1;
    }

    static void copy_backward(char *end,char *finish,size_t size)
    {
        while(size--){
            *end-- = *finish--;
        }
    }

    static void copy_forward(char *start,char *first,size_t size)
    {
        while(size--){
            *start++ = *first++;
        }
    }

public:
    hstring() = default;

    hstring(const char *c){
        set_len(c);
        str = (char *) alloc::allocate(this->len);
        std::memcpy(str,c,len);
    }

    explicit hstring(size_t len)
    {
        this->len = len;
        this->str = (char *)alloc::allocate(len);
    }

    hstring(const hstring& hstr)
    {
        len = hstr.len;
        str = (char *)alloc::allocate(len);
        memcpy(str,hstr.str,len);
    }

    ~hstring() {
        alloc::deallocate((void *) str, len);
    }



    inline friend std::ostream &operator<<(std::ostream &os, const hstring &str);
    inline hstring& operator=(const hstring& hstr);
    inline hstring operator+(const hstring& hstr1)const;
    inline hstring operator+(const char* c);
    inline hstring operator-(const hstring& hstr1)const;
    inline hstring operator-(const char* c);
    inline hstring& replace(const char *rstr,const char *str);
    inline int find(const char* c);
    inline hstring& operator=(long num);
};

inline hstring& hstring::operator=(long num)
{
    size_t ndigits = 1;
    long num_t = num;
    while(num_t/=10) ++ndigits;
    if(get_alloc_size()<ndigits+1){
        alloc::deallocate(this->str,len);
        this->str = (char *)alloc::allocate(ndigits+1);
    }
    len = ndigits+1;
    char *finish = this->str + ndigits;
    *finish = '\0';
    *--finish = num%10 + 48;
    while(num/=10){
        *--finish = num%10 + 48;
    }
    return *this;
}

inline int hstring::find(const char *c)
{
    char *pstr;
    if((pstr = strstr(this->str,c)) == NULL){
        return -1;
    }else{
        return pstr - this->str;
    }
}

inline hstring& hstring::replace(const char *rstr, const char *str)
{
    int flag = 0;
    char *pstr = strstr(this->str,rstr);
    if(pstr == NULL){
        printf("can not find %s inside %s\n",rstr, this->str);
        return *this;
    }
    size_t new_len = this->len - (get_len(rstr) - get_len(str));
    char *old_str = this->str;
    size_t old_len = len;
    if(get_alloc_size() < new_len){
        old_str = this->str;
        this->str = (char *)alloc::allocate(new_len);
        flag = 1;
    }
    this->len = new_len;

    size_t elem_before = pstr - old_str;
    size_t elem_after = old_len - elem_before - get_len(rstr) +1;
    if(new_len >= old_len)
        copy_backward(this->str+new_len-1,old_str+old_len-1,elem_after);
    else
        copy_forward(this->str+elem_before+ get_len(str)-1,old_str+elem_before+ get_len(rstr)-1,elem_after);
    copy_forward(this->str,old_str,elem_before);
    copy_forward(this->str+elem_before,(char *)str, get_len(str)-1);
    if(flag){
        alloc::deallocate(old_str,old_len);
    }

    return *this;
}

inline std::ostream &operator<<(std::ostream &os, const hstring &str)
{
    return os<<str.str;
}

inline hstring hstring::operator-(const hstring &hstr1) const
{
    const char* pstr = std::strstr(this->str,hstr1.str);
    if(pstr == NULL){
        printf("can not find %s inside %s\n",hstr1.str,this->str);
        return NULL;
    }else{
        size_t new_len = this->len - hstr1.len + 1;
        hstring res(new_len);
        res.len = new_len;
        res.str = (char *)alloc::allocate(new_len);
        const char *this_str = this->str;
        char *res_str = res.str;
        size_t hstr1_len = hstr1.len;
        while(new_len--){
            if(this_str == pstr){
                while(--hstr1_len)
                    ++this_str;
            }else{
                *res_str++ = *this_str++;
            }
        }
        return res;
    }
}

inline hstring hstring::operator-(const char* c)
{
    const char* pstr = std::strstr(this->str,c);
    if(pstr == NULL){
        printf("can not find %s inside %s\n",c,this->str);
        return NULL;
    }else{
        size_t new_len = this->len - get_len(c) + 1;
        hstring res(new_len);
        res.len = new_len;
        res.str = (char *)alloc::allocate(new_len);
        const char *this_str = this->str;
        char *res_str = res.str;
        size_t hstr1_len = get_len(c);
        while(new_len--){
            if(this_str == pstr){
                while(--hstr1_len)
                    ++this_str;
            }else{
                *res_str++ = *this_str++;
            }
        }
        return res;
    }
}

inline hstring& hstring::operator=(const hstring& hstr)
{
    if(hstr.len > this->get_alloc_size()){
        alloc::deallocate(str, len);
        this->str = (char *)alloc::allocate(hstr.len);
        len = hstr.len;
        std::memcpy(str, hstr.str, len);

    }else{
        std::memcpy(this->str, hstr.str,hstr.len);
        len = hstr.len;
    }
    return *this;
}

inline hstring hstring::operator+(const char* c)
{
    size_t len = get_len(c);
    hstring res(len + this->len);
    strcpy(res.str, this->str);
    strcat(res.str, c);
    res.len = len + this->len;
    return res;
}

inline hstring hstring::operator+(const hstring& hstr1)const
{
        size_t new_len = hstr1.len + this->len;
        std::cout<<"new_len = "<<new_len<<std::endl;
        hstring res(new_len);
        std::memcpy(res.str, this->str,this->len);
        strcat(res.str, hstr1.str);
        return res;
}



#endif