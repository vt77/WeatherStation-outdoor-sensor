#include <unity.h>
#include<iostream>
#include <stdio.h>
#include "JsonStaticString.h"


unsigned long millis()
{
    return 0;
}

class MockString : public std::string
{
    public:
        MockString(const char * s):std::string(s){}
        bool startsWith(const char *s)
        {
            return (std::string::rfind(s, 0) == 0);
        }
};

class MockWiFiClient
{
    public:

    int connect(const char * host, int port)
    {
            std::cout << "MockConnect to " <<  host << ":" << port << std::endl;
            return 1;
    }

    int print(const char * data )
    {
        //Check strings
        TEST_ASSERT_MESSAGE(strstr(data,"#room1Hum#63") != 0,"H1 value wrong");
        TEST_ASSERT_MESSAGE(strstr(data,"#room1Temp#25.1") != 0,"T1 value wrong");
        TEST_ASSERT_MESSAGE(strstr(data,"##") != 0,"End of packet missing");
        return 1;
    }

    MockString readStringUntil(char c)
    {
        MockString s = "OK";
        return s;
    }

    int read(char * buff, int len)
    {
        snprintf(buff,len,"OK\r\n");
        return strlen(buff);
    }

    int  available()
    {
        return 1;
    }

    void stop()
    {
        std::cout << "Closing client " << std::endl;
    }
};



#define NARODMON_DEBUG(fmt,...) std::printf("[NARODMON][DEBUG]" fmt "\n", ## __VA_ARGS__)
#define NARODMON_ERROR(fmt,...) std::printf("[NARODMON][ERROR]" fmt "\n", ## __VA_ARGS__)

#define NARODMON_DEVICENAME "WeatherStation"
#define NARODMON_USER "test"

//Mock WiFiClient class
#define WiFiClient MockWiFiClient
#define String MockString

#include "narodmon.hpp"
#include "SensorsCollection.hpp"

//vt77::DATA_FORMAT narodmonProto;
vt77::NarodmonProtoAmbient narodmonProto;


vt77::device_desc_t data = {
    "room1",
     251,
     63,
     1,0   
};

void test_narodmon(void) {

    narodmonProto.start("00:00:00:00:00");
    narodmonProto.insert(&data);
    narodmonProto.close();
    vt77::datasender.send_data(0,(const char *)narodmonProto);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_narodmon);
    UNITY_END();
    std::cout << narodmonProto << std::endl;
}

