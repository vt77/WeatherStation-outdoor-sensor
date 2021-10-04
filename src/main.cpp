//Static preferences (defaults)
#include "preferences.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <FS.h>
#include <LittleFS.h>
#include <Preferences.hpp>

#include <CaptivePortal.h>
#include "JsonStaticString.h"

#include <LEDWink.hpp>


using namespace vt77;

#ifdef DATASENDER
#include "tasker.hpp"
#include DATASENDER
Tasker tasker;
#endif


#include <RFReceiver.h>
#include <SensorsCollection.hpp>


typedef struct{
  unsigned int interval;
  char token[16];
  char fingerprint[64];
}deviceconfig_t;

deviceconfig_t deviceconfig = {
    SEND_INTERVAL
};
Preferences<deviceconfig_t> preferences(&deviceconfig);

CaptivePortal captivePortal;
const get_preferences_t CaptivePortal::preferencesJson = []() -> String {
    JsonStaticString<256> json;
    json.start();
    json.insert("interval",(int)deviceconfig.interval);
    json.insert("token",deviceconfig.token);
    json.insert("fingerprint",deviceconfig.fingerprint);
    json.close();
    return String(json);    
};



//Most propably DATA_FORMAT defined in DATASENDER
#ifndef DATA_FORMAT
#define COLLECTION_DATA_CLASS(a) SensorsCollectionJson
#else
#define COLLECTION_CLASS_BUILDER(NAME) NAME ## Ambient
#define COLLECTION_DATA_CLASS(a) COLLECTION_CLASS_BUILDER(a)
#endif


RFReceiver rfReceiver;

class LedWinkButtonHandler : public LEDWink
{
    private:
        void onButtonPress(unsigned long ms) override {
            
            //Long press 5 seconds reboots device in AP mode 
            if(ms > 4000)
            {
                
                Serial.println("Erase config");
                ESP.eraseConfig();
                rfReceiver.stopReceive();
                WiFi.persistent(true);
                WiFi.disconnect(true);
            }

            Serial.println("Restart MCU");
            delay(1000);
            WiFi.disconnect();
            ESP.restart();
            
        }   
};

LedWinkButtonHandler ledWinker;
SensorsCollection<COLLECTION_DATA_CLASS(DATA_FORMAT)> sensorsCollection(rfReceiver,DEVICE_ID);

#ifdef ANOTHER_DEVICE_TEST
//NOTE ! It's just for my own case, because I'm using different devices (for tests)
//You better buy same model for all devices 
AnotherDevice<SensorsCollection<COLLECTION_DATA_CLASS(DATA_FORMAT)>> anotherDevice(rfReceiver,&sensorsCollection);
#endif

const char * channelNames[] = 
{
    "kids",     //Channel 0
    "bathr",    //Channel 1
    "bedr",     //Channel 2
    "livingr"   //Channel 3
};


void setup() {

    Serial.begin(115200);
    WiFi.printDiag(Serial);
    
    if (!LittleFS.begin()) {
      Serial.println("LittleFS mount failed");
      return;
    }

    ledWinker.init();

    wl_status_t status = WL_IDLE_STATUS;
    if(WiFi.SSID().isEmpty())
    {
        //Last AP connection failed or button reset occures
        Serial.println("[WIFI]Using local AP mode");
        status = WL_CONNECT_FAILED;

    }else{

        Serial.print("[WIFI]Connect to AP ");
        Serial.println(WiFi.SSID());
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        WiFi.setAutoReconnect(true);

    }

    
    do{
            if( status == WL_CONNECT_FAILED || status == WL_WRONG_PASSWORD || status == WL_CONNECT_FAILED )
            {
                Serial.print("[WIFI]Connection failed : 0x");
                Serial.println(status,HEX);
                #ifdef _ENABLE_HTTP_SERVER
                Serial.println("[WIFI]Start captive portal");
                captivePortal.start(MODE_LOCAL);
                ledWinker.setpattern(BLINK_DOUBLE);
                #endif
                return;
            }
            Serial.print(".");
            delay(500);
            ledWinker.setledstatus(wink_status_t::on);
            delay(200);
            ledWinker.setledstatus(wink_status_t::off);
    }
    while ( ( status = WiFi.status() ) != WL_CONNECTED );

    delay(2000);
    Serial.println(" Done");
    Serial.print("[WIFI]Connected, IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("[WEBSRV]Start");
    captivePortal.start(MODE_PUBLIC);
    captivePortal.onPreferences(std::bind([](deviceconfig_t *config){
        strncpy( config->fingerprint,captivePortal.get("fingerprint").c_str(),64);
        strncpy( config->token,captivePortal.get("token").c_str(),16);
        config->interval = captivePortal.get("interval").toInt();
        Serial.print("[WEBSRV]Save preferences ");
        Serial.println(CaptivePortal::preferencesJson());
        preferences.save();
    },&deviceconfig));

    captivePortal.webserver()->on("/data",HTTP_GET,[](){
            SensorsCollectionJson data;
            data.start("data");
            for(int i=0;i<DEVICES_COUNT;i++)
            {   
                data.insert(&devices[i],true);
            }
            data.close();            
            captivePortal.webserver()->send(200, "application/json", static_cast<const char*>(data));
    });

#ifdef DATASENDER
    tasker.attach(deviceconfig.interval ,std::bind([](){
        const char * data = static_cast<const char*>(sensorsCollection);

        if(data == nullptr)
        {
            Serial.println("[WARNING][DATASENDER]No devices to send");
            return;
        }

        ledWinker.setledstatus(wink_status_t::on);
        Serial.print("[SEND]");
        char timeStr[32];
        time_t now = time(nullptr);
        struct tm * timeinfo = gmtime(&now);
        sprintf(timeStr,"[%02d-%02d-%4d %02d:%02d:%02d]",
            timeinfo->tm_mday,
            timeinfo->tm_mon,
            timeinfo->tm_year + 1900,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec
        );
        Serial.print(timeStr);
        Serial.println(data);      
        datasender.send_data(deviceconfig.token,data);
        Serial.print("[FREEMEM] ");
        Serial.println(ESP.getFreeHeap());
        ledWinker.setledstatus(wink_status_t::off);
    }));
#endif


    if (!MDNS.begin("ambmon")) {             // Start the mDNS responder for ambmon.local
        Serial.println("Error setting up MDNS responder!");
    }
    MDNS.addService("http", "tcp", 80);

    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.println("Start listener");

    sensorsCollection.setChannelNames(channelNames);
    sensorsCollection.setProtocol(vt77::wsproto_t::PROTO_DGR8S);
    //chan2.setup({500,8,4,2});
    //chan2.setProtocol(vt77::wsproto_t::PROTO_DGR8H);
    rfReceiver.startReceive(); 
}

void loop() {
    captivePortal.process();
#ifdef DATASENDER
    tasker.process();
#endif
    rfReceiver.process();
    ledWinker.process();
    MDNS.update();
}
