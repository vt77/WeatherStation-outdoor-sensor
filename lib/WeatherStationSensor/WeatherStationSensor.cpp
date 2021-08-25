#include "WeatherStationSensor.h"
#include <functional>


#ifdef DEBUG_ESP_PORT
#define WSUNIT_DEBUG(fmt, ...)  DEBUG_ESP_PORT.printf_P((PGM_P)PSTR( "[WSUNIT][DEBUG]" fmt "\r\n"), ## __VA_ARGS__)
#define WSUNIT_ERROR(fmt, ...)  DEBUG_ESP_PORT.printf_P((PGM_P)PSTR( "[WSUNIT][ERROR]" fmt "\r\n"), ## __VA_ARGS__)
#endif


#ifndef WSUNIT_DEBUG
#define WSUNIT_DEBUG(...)
#endif

#ifndef WSUNIT_ERROR
#define WSUNIT_ERROR(...)
#endif


#define PACKET_LENGTH  36

using namespace vt77;

#ifdef UNIT_TEST
unsigned long millis();
#else
#include <Arduino.h>
#endif

/*
Parses DGR8S protocol 

Code    ID    Batt  TX  CH    X        Temp(10)       Hum(8)        
1001  10110011 0     0   10   00      0011010101     00101110 0
*/

bool parse_weather_data_DGR8S(WeatherData *wd, const uint32_t data)
{

       wd->hum  = data & 0xFF; /* humidity */
       wd->temp = (data >> 8 ) & 0x3FF; /* temperature */
       wd->cond = ( data >> 18 ) & 0x3; /* condition */
       wd->chan = ( data >> 20 ) & 0x3;  /* channel */
       wd->txmode = ( data >> 22 ) & 0x1;  /* tx_mode */
       wd->batt =  ( data >> 23 ) & 0x1;  /* battery */
       //(data >> 24) & 0xFF,  /* deviceid */

       return true;
}

/*
    Parses DGR8H protocol

   ID    Bat  Fix  Ch   Temp         Fix   Hum        
01000110  1   0    10   000100110001 1111 00110101

*/
bool parse_weather_data_DGR8H(WeatherData *wd, const uint32_t data)
{

      uint16_t temp = (data >> 12) & 0xFFF;
      wd->temp = (temp & 0x800) == 0 ? temp : temp | (-1 ^ 0xFFF);
      wd->hum = data & 0xFF;
      wd->chan = data >> 24 & 0x3;
      wd->batt = data >> 27 & 0x1;

      return true;
}

/*
 This is proto of our weather station
 See https://github.com/vt77/WeatherStation
   
  Temp        Hum        Speed    Dir   Ch(4)  But ( (2v-5v ) 3v / 16 ~ 0.2v )
XXXXXXX.X   XXXXXXX.X   XXXXX.X  XXXX   XX     XXXX       

*/
bool parse_weather_data_VT77(WeatherData *wd, const uint32_t data)
{

      uint16_t temp = (data >> 24) & 0xFF;
      wd->temp = (temp & 0x80) == 0 ? temp : temp | (-1 ^ 0xFFF);
      wd->hum = (data >> 16) & 0xFF;
      wd->chan = data >> 4 & 0x3;
      wd->batt = data & 0xF;
      return true;
}


bool parse_weather_debug(WeatherData *wd, const uint32_t data)
{
    return true;
}

using processFunction =  std::function<bool(WeatherData *wd, const uint32_t data)>;
const processFunction protocolHandlers[] = {
    parse_weather_debug,
    parse_weather_data_DGR8S,
    parse_weather_data_DGR8H
};

const char * proto_names[] = {
    "DUMP",
    "DGR8S",
    "DGR8H"
};


void WeatherStationSensor::parse(uint8_t id, uint32_t data)
{
    WSUNIT_DEBUG("[DATA]Device : 0x%X  Data 0x%X",id,data);

	static uint32_t confirm = 0;
	static unsigned long packet_gap = 0;
    unsigned long currentMillis = millis();

	//Skip repats
	if ( currentMillis - packet_gap < 3000 ){
		return;
	}

	/* First chance */	
 	if( confirm != data)
	{
	  	confirm = data;
		return; 
	}

    packet_gap = currentMillis;
    confirm = 0;

    WeatherData weather_data;
    weather_data.id = id;
    if( protocolHandlers[this->proto](&weather_data,data) )
    {
        WSUNIT_DEBUG("[DATA][%s][0x%X][%d] Temp: %d  Hum: %d ",proto_names[this->proto],id,weather_data.chan,weather_data.temp,weather_data.hum);
        this->onWeatherData(weather_data);
    }
}


void WeatherStationSensor::process(rfpulse pulse)
{
	bool is_high = (pulse & 0x01);

	if(!is_high){
        /* Filter noise */
		 if(pulse < 350 || pulse > 750 )
			    bits_count = 0;
		 return;
	}

	/* Calc len in osc periods */	
	uint8_t periods = uint8_t( (pulse + (timings.osc >> 1)) / timings.osc );
	if(periods == 0)
	{
		bits_count = 0;
		return;
	}

	/* Gap in packet */
	if(periods >= timings.gap)
	{

        //WSUNIT_DEBUG("[RESET]Len : %d  Periods :  %d",bits_count,periods);

		if(bits_count == PACKET_LENGTH)
        {
			parse(device_id,data_byte);
        }
		//This is ugly hack because some devices sending 0 in the end of packet
		else if(bits_count == PACKET_LENGTH + 1)
			parse(device_id,data_byte>>1);

		device_id = 0;
		bits_count = 0;
		return;	
	}

    //WSUNIT_DEBUG("[STORE]Len : %d  Periods :  %d",bits_count,periods);

    data_byte <<= 1;
    if ( periods == timings.one )
        data_byte |= 1;

    if(bits_count == 7)
		device_id = data_byte & 0xFF;
    
    bits_count++;
}

WeatherStationSensor::WeatherStationSensor(wsproto_t protocol)
{
    proto = protocol;
    setup({950,8,4,2});
}

void WeatherStationSensor::setup(const rftimings_t &t)
{
    timings = t;   
}

void WeatherStationSensor::setProtocol(wsproto_t protocol)
{
    proto = protocol;
}

