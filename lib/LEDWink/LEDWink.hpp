namespace vt77 {

enum wink_status_t{
    on=1,
    off =0
};


#define BLINK_1HZ 0
#define BLINK_FAST 1
#define BLINK_SHORT 2
#define BLINK_DOUBLE 3

#define PATTERN_NONE 0xFF


typedef struct { unsigned long timing[4]; } blink_pattern;

blink_pattern patterns [] = {
    {1000,1000,1000,1000},
    {200,200,200,200},
    {200,1000,200,1000},
    {200,200,200,1000},
};

class LEDWink{


    private:
        bool ledStatus;
        bool btnStatus;
        unsigned long last_time;
        blink_pattern* pattern = nullptr;
        unsigned int pattern_pos = 0;
        virtual void onButtonPress(unsigned long ms){
            //Override to handle button press
        }
 
    public:
        bool setledstatus(wink_status_t status){
            if(status == wink_status_t::on)
            {
                pinMode(LEDWINK, OUTPUT);
                ledStatus = true;
            }else{
                pinMode(LEDWINK, INPUT);
                ledStatus = false;
            }
            return  ledStatus;
        }

        bool getledstatus()
        {
            return ledStatus;
        }

        void setpattern(int p)
        {
            if( p==PATTERN_NONE )
            {
                setledstatus(wink_status_t::off);
                pattern = nullptr;
                return;
            }

            pattern = &patterns[p];
            pattern_pos = 0;
            last_time = millis();
            setledstatus(wink_status_t::on);
        }

        void init()
        {
            digitalWrite(LEDWINK, LOW);
            setledstatus(wink_status_t::off);
            btnStatus = digitalRead(LEDWINK);
        }

        void process()
        {

            static unsigned long btn_press_start;

            if(pattern != nullptr )
            {
                
                unsigned long currentMillis = millis();
                if ( (currentMillis - last_time) >= pattern->timing[pattern_pos])
                {
                    setledstatus(pattern_pos&1 ? wink_status_t::off : wink_status_t::on);
                    pattern_pos = (pattern_pos + 1) & 0x3;
                    last_time = currentMillis;
                }
            }

            if( ledStatus )
                return;

            bool bs = digitalRead(LEDWINK);
            if(bs != btnStatus)
            {
                //debounce
                delay(50);
                if( bs != btnStatus )
                {
                    if(!bs)
                    {
                        btn_press_start =  millis();
                    }else{
                        onButtonPress(millis()-btn_press_start);
                    }
                    btnStatus = bs;
                }                
            }

        }
};


}