

#include "RFDataListener.h"

namespace vt77{

class RFReceiver{
    public:
        RFReceiver();
        static void addListener(RFDataListener *device);
        void startReceive();
        void stopReceive();       
        void process();
        int count();
  private:
      static void interruptHandler();        
};

}