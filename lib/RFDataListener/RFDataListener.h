#ifndef __VT77_RFDDATALISTENER
#define __VT77_RFDDATALISTENER

#include <stdint.h>

typedef uint32_t rfpulse;

namespace vt77{
class RFDataListener{
    public:
        virtual void process(rfpulse pulse) = 0;
};

}

#endif