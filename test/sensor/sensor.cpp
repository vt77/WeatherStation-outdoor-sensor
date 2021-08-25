#include <unity.h>
#include<iostream>
#include <stdio.h>


#define WSUNIT_DEBUG(fmt, ...)  std::printf("[WSUNIT][DEBUG]" fmt "\n", ## __VA_ARGS__)
#define WSUNIT_ERROR(fmt, ...)  std::printf("[WSUNIT][ERROR]" fmt "\n", ## __VA_ARGS__)

#include "WeatherStationSensor.h"

#define DEVICE_ID1   0x56  
#define DEVICE_DATA1 0x6AA11942

#define DEVICE_ID2 0x9D
#define DEVICE_DATA2 0x5C012F42


using namespace vt77;

unsigned long millis()
{
    return 0;
}

bool parse_weather_data_DGR8S(WeatherData *wd, const uint32_t data);
bool parse_weather_data_DGR8H(WeatherData *wd, const uint32_t data);


void test_sensor_proto(void) {

    WeatherData wd;
    parse_weather_data_DGR8S(&wd,DEVICE_DATA1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(281,wd.temp,"DGR8S Test1 Wrong temperature");
    TEST_ASSERT_EQUAL_INT_MESSAGE(66,wd.hum,"DGR8S Test1 Wrong humidity");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2,wd.chan,"DGR8S Test1 Wrong humidity");

    parse_weather_data_DGR8S(&wd,DEVICE_DATA2);
    TEST_ASSERT_EQUAL_INT_MESSAGE(303,wd.temp,"DGR8S Test2 Wrong temperature");
    TEST_ASSERT_EQUAL_INT_MESSAGE(66,wd.hum,"DGR8S Test2 Wrong humidity");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0,wd.chan,"DGR8S Test2 Wrong humidity");

    //TEST_ASSERT_MESSAGE(jsonString.insert("str1","test1"),"Inser string 1");
    //TEST_ASSERT_MESSAGE(jsonString.insert("str2","test2"),"Inser string 2");
    //TEST_ASSERT_MESSAGE(jsonString.insert("float",(double)32.2),"Insert float");
    //TEST_ASSERT_MESSAGE(jsonString.insert("float",(int)28),"Insert int");
    //jsonString.close();
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_sensor_proto);
    UNITY_END();
    //std::cout << jsonString << std::endl;
}






