
#include "RFDataListener.h"
#include <functional>

namespace vt77{

typedef struct{
   uint16_t osc;  /* Oscillator */
   uint8_t  gap;
   uint8_t  one;
   uint8_t  zero;
}rftimings_t;

typedef struct{
      uint16_t id;
      uint8_t hum;
      int16_t temp;
      uint8_t cond;
      uint8_t chan;
      uint8_t txmode;
      uint8_t batt;
}WeatherData;



typedef enum {
    PROTO_DEBUG = 0,
    PROTO_DGR8S = 1,
    PROTO_DGR8H = 2
} wsproto_t;

using WeatherSensorCallback = std::function<void(WeatherData wd)>;


class WeatherStationSensor : public RFDataListener{
    public:
        WeatherStationSensor(wsproto_t protocol=PROTO_DGR8S);
        void process(rfpulse pulse) final;
        void setup(const rftimings_t &timings);
        void setProtocol(wsproto_t prot);
    protected:
        virtual void onWeatherData(const WeatherData wd) = 0;
    private:
        void parse(uint8_t id, uint32_t data);
        rftimings_t timings;
        wsproto_t proto;
        uint32_t data_byte;
        uint8_t bits_count;
        uint8_t device_id;
};

}