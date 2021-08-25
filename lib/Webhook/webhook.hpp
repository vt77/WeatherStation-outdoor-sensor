
#ifndef WEBHOOK_DEBUG
#define WEBHOOK_DEBUG(...)
#endif

#ifndef WEBHOOK_ERROR
#define WEBHOOK_ERROR(...)
#endif

#ifdef _ENABLE_TLS_WEBHOOK
#include <WiFiClientSecure.h>
WiFiClientSecure httpClient;
#define WEBHOOK_PORT 443
#else
#include <WiFiClient.h>
WiFiClient httpClient;
#define WEBHOOK_PORT 80
#endif

#ifdef WEBHOOK_TOKEN_ENCRYPT
#include <MD5Builder.h>
MD5Builder md5build;
#endif

#include "DataSender.hpp"


namespace vt77 {


class Webhook : public DataSender
{
    public:
        void init(const char * tls);
        void send_data(const char * access_token, const char * body);
};

void Webhook::init(const char * tls)
{
 #ifdef AUTH_CERT
 X509List cert(tls);
 httpClient.setTrustAnchors(&cert);
 #endif
 
 #ifdef AUTH_FINGERPRINT
 WEBHOOK_DEBUG("Using fingerprint %s\r\n",tls);   
 httpClient.setFingerprint(tls);
 #endif
 httpClient.setTimeout(15000);
}


/**
 *  Sends data to cloud  over https webhook
 * 
 *  @param access_token  user's access token
 *  @param body json encoded body string
 */
void Webhook::send_data(const char * access_token, const char * body)
{
    const char *host = "vt77.com";
    const char *uri = "/iot/v2/pushes/";

#ifdef WEBHOOK_DEBUG
    IPAddress ip;
    int err = WiFi.hostByName(host, ip) ;
    if(err == 1)
    {
        WEBHOOK_DEBUG("Send to %s IP %s\n",host,ip.toString().c_str());
    }else{
        WEBHOOK_ERROR("Hostname not found %s Error %d\n",host,err);
    }
#endif
    #ifdef WEBHOOK_INSECURE
    //This will disable any checks
    #warning WEBHOOK_INSECURE strongly not recommended
    httpClient.setInsecure();
    #endif
    if (!httpClient.connect(host, WEBHOOK_PORT)) {
        //int err = httpClient.getLastSSLError(p,100);
        WEBHOOK_ERROR("Connection failed !");
        return;
    }

    char tmp[40];
    sprintf(tmp, "POST %s HTTP/1.1", uri);

    httpClient.println(tmp);
    httpClient.print("Host: "); 
    httpClient.println(host);
    httpClient.println("User-Agent: ESP8266Webhook");
    httpClient.print("Access-Token: ");

    #ifdef WEBHOOK_TOKEN_ENCRYPT
        time_t now = time(nullptr);
        struct tm * timeinfo = gmtime(&now);
        sprintf(tmp,"%d%d%d%d",
            timeinfo->tm_year + 1900,
            timeinfo->tm_yday,
            timeinfo->tm_hour,
            timeinfo->tm_min
        )
        md5build.begin();
        md5build.add(access_token); 
        md5build.add(tmp); 
        md5build.calculate(); 
        md5build.getChars(tmp);

        WEBHOOK_DEBUG("Using token %s\n",tmp);
        httpClient.println(tmp);

    #else
        WEBHOOK_DEBUG("Using token %s\n",access_token);
        httpClient.println(access_token);
    #endif
    httpClient.println("Content-Type: application/json");
    httpClient.print("Content-Length: "); httpClient.println(strlen(body));
    httpClient.println("Accept-Encoding: identity");
    httpClient.println(); 
    WEBHOOK_DEBUG("Send body %s\n",body);
    httpClient.println(body);

    WEBHOOK_DEBUG("Request sent\n");
    while (httpClient.connected()) {
        String line = httpClient.readStringUntil('\n');
        if (line == "\r") {
            WEBHOOK_DEBUG("Headers received\n");
            break;
        }
        WEBHOOK_DEBUG("R:%s\n",line.c_str());
    }
    
    String line = httpClient.readStringUntil('\n');
    WEBHOOK_DEBUG("R:DataSize:%s\n",line.c_str());
    line = httpClient.readStringUntil('\n');
    WEBHOOK_DEBUG("R:RESP:%s\n",line.c_str());
    WEBHOOK_DEBUG("Closing connection\n");
};

//WARNING !!! The insertion status not checked !. 
//Please provide enough space to hold all sensor's names and values
#define MAX_JSON_STRING_LENGTH  128
class WebhookProto : public JsonStaticString<MAX_JSON_STRING_LENGTH>
{
    public:
        void start(const char * name)
        {
            JsonStaticString<MAX_JSON_STRING_LENGTH>::start();
            JsonStaticString<MAX_JSON_STRING_LENGTH>::insert("id",name);
        }
};

#define DATA_FORMAT WebhookProto
Webhook datasender;

}