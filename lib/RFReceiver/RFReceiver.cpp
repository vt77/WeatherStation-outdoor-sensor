
#include <Arduino.h>
#include "RFReceiver.h"
#include "DataSequence.h"

#define MIN_GAP_uS	3000
#define MIN_PULSE_LENGTH 250
typedef enum {IDLE,READING,READY} rf_state;
volatile static rf_state state = IDLE;

#define BUFFER_SIZE  128
#define PTR_MASK 0x7F

using namespace vt77;

class RFData : DataSequence<rfpulse>{

    public:
        unsigned int size() const{
		return BUFFER_SIZE;	
	};
        
	unsigned int count() const{
		return (_write_ptr - _read_ptr) % PTR_MASK; 	
	};
        
	rfpulse last() const{
		return _data[_write_ptr-1 % PTR_MASK];	
	};
        
	rfpulse first() const{
		 return _data[_read_ptr];	
	};

        void put(rfpulse p){
		//Save item in buffer abd increment
		_data[_write_ptr++ % PTR_MASK]  = p;
	};

        rfpulse get(){
		//Return item from head and increment
		return _data[_read_ptr++ % PTR_MASK];
	}

	void clear(){
		_read_ptr =  0; 
		_write_ptr = 0; 
	}

	bool full() const{
	     return !(count()<BUFFER_SIZE);
	}

	/*
	rfpulse& operator[] (unsigned int i){
		//Return item from position
		return _data[(_read_ptr + i) & PTR_MASK];
	}
	*/

    private:
	rfpulse _data[BUFFER_SIZE];
	//std::unique_ptr<T[]> buf_;
 	unsigned int _read_ptr;
	unsigned int _write_ptr;	
};

RFData _pulses;

#define MAX_LISTENERS 5
static uint8_t listeners_count = 0;
static vt77::RFDataListener* handlers[MAX_LISTENERS];

void RFReceiver::addListener(vt77::RFDataListener* rfDevice){
	if( listeners_count >= MAX_LISTENERS)
		return;
	handlers[listeners_count++] = rfDevice;
}

void RFReceiver::startReceive(){
	attachInterrupt(RF_RECEIVER_PIN, interruptHandler, CHANGE);
}

void RFReceiver::stopReceive(){
        detachInterrupt(RF_RECEIVER_PIN);
}

void RFReceiver::process(){
    unsigned int count =  _pulses.count();
    if(count > 0)
    {		
        rfpulse pulse = _pulses.get();		
        for(int i=0;i<listeners_count;i++)
        {
            handlers[i] -> process(pulse);
            //yield();
        }
    }
}


void ICACHE_RAM_ATTR RFReceiver::interruptHandler(){
    
     static unsigned long last_time = 0;
     const long time = micros(); 
     unsigned int pulse_length = time - last_time;

     //Last bit is level mark
     pulse_length &=~(1);
     if (digitalRead(RF_RECEIVER_PIN)) 
		pulse_length |= 1;

     if(pulse_length < MIN_PULSE_LENGTH)
     {
	    state = IDLE;
	    //_pulses.clear(); 
     }else if(pulse_length > MIN_GAP_uS)
     {
	    //Long gap found. Start reading 
        state = READING;
     }

     if( state == READING && !_pulses.full() )
   	    _pulses.put( (rfpulse)pulse_length );
     
     last_time = time;
}

RFReceiver::RFReceiver(){
}

