#include "CaptivePortal.h"
#include <FS.h>
#include <LittleFS.h>

const byte DNS_PORT = 53;
IPAddress apIP(10, 1, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

static save_preferences_t savePref = nullptr;

void update_wifi_settings(const char * ssid, const char * password)
{
    //struct station_config stationConf;
    //wifi_station_get_config (&stationConf);
    if(WiFi.SSID() != ssid || WiFi.psk() != password)
    {  
            Serial.println("Save new WiFi settings");
            WiFi.persistent(true);
            WiFi.begin(ssid, password); //start station with new credentials
    }
}


void start_local_server()
{
    dnsServer.start(DNS_PORT, "*", apIP);
    webServer.serveStatic("/wifisettings",LittleFS,"wifisettings.html");
    webServer.onNotFound([]() {
        webServer.sendHeader("Location", "/wifisettings",true);
        webServer.send(302, "text/plain",""); 
    });

    webServer.on("/wifisave", []() {
          if (webServer.method() != HTTP_POST) {
              webServer.send(405, "text/plain", "Method Not Allowed");
              return;
          }

        //server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
        //server.arg("p").toCharArray(password, sizeof(password) - 1)

        //String message = "POST form was:\n";
        //for (uint8_t i = 0; i < webServer.args(); i++) {
        //    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
        //}
        update_wifi_settings(webServer.arg("n").c_str(),webServer.arg("p").c_str());
        webServer.send(200, "text/html", "<h2>Settings saved.</h2> Device will be restarted in few seconds");
        delay(4000);
        ESP.restart();
    });

    webServer.begin();
}

void start_public_server()
{
    webServer.begin();
    webServer.serveStatic("/",LittleFS,"index.html");
    webServer.serveStatic("/config",LittleFS,"config.dat");
    webServer.on("/preferences",HTTP_GET,[](){
            webServer.send(200, "application/json", CaptivePortal::preferencesJson());
    });
    webServer.on("/preferences",HTTP_POST,[](){
        if(savePref != nullptr)
        {
            savePref();
            webServer.send(200,"text/html", "<h2>Configuration saved.</h2> Please <a href='/restart'>restart</a> your device");
        }
    });

    webServer.on("/restart", [](){
        webServer.send(200, "text/html", "<meta http-equiv='refresh' content='3;url=/' />Wait for restart...");
        delay(1000);
        ESP.restart();
    });

}

void CaptivePortal::start(CaptivePortalMode mode )
{
    if(mode==MODE_LOCAL)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP("vt77IOTCaptive","12345678");
        start_local_server();
    }else
       start_public_server(); 
}

void CaptivePortal::onPreferences(save_preferences_t func)
{
    savePref = func;
}

const String CaptivePortal::get(const char * keyname)
{
    return webServer.arg(keyname);
}


void CaptivePortal::process(){
    dnsServer.processNextRequest();
    webServer.handleClient();
}

ESP8266WebServer* CaptivePortal::webserver(){
    return &webServer;
}


