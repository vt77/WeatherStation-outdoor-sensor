#ifndef __JSON_STATIC_STRING
#define __JSON_STATIC_STRING


#include <string.h>

namespace vt77 {

#define MAX_STRING_VALUE_LEN  96

template <unsigned S>
class JsonStaticString
{
    private: 
        char buffer[S];
        bool _append_pair(const char * key, const char * value)
        {
            if(strlen(buffer) + strlen(key) + strlen(value) + 4 >= S)
                return false;
            if(strlen(buffer)>1)
                    strcat(buffer,", ");
            char * ptr =  buffer + strlen(buffer);
            sprintf(ptr,"\"%s\":%s",key,value);
            return true;
        }

    public:
        bool insert(const char * key, const char * value){
            char tmp[MAX_STRING_VALUE_LEN];
            sprintf(tmp,"\"%s\"",value);
            return _append_pair(key,tmp);
        }

        bool insert(const char * key, const int value){
            char number[16];
            sprintf(number,"%d",value);
            return _append_pair(key,number);
        }

        bool insert(const char * key, double value, int precision=2){
            char format[16];
            char number[16];
            sprintf(format,"%%.%df",precision);
            sprintf(number,format,value);
            return _append_pair(key,number);
        }

        void start()
        {
            strcpy(buffer,"{");
        }

        void close()
        {
            strcat(buffer,"}");
        }

        operator const char*()const {
            return buffer;
        }
};

}
#endif