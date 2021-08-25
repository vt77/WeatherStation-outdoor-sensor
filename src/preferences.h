
#include "defines.h"

#define DEVICE_ID "ambient"

#define AUTH_FINGERPRINT 1


#ifndef DATASENDER
#define DATASENDER WEBHOOK
#endif


#ifndef NARODMON_DEVICENAME
#define NARODMON_DEVICENAME "AmbientMonitor"
#endif

//This is mandatory for narodmon. 
//Please set your own id in narodmon system
//#define NARODMON_USER "unknown"

#ifndef _ENABLE_HTTP_SERVER
#define WIFI_SSID  "your_wifi_ssid"
#define WIFI_PASSWORD  "your_wifi_password"
#endif

#define SEND_INTERVAL 20
#define TOKEN "your_api_token"
#define FINGERPRINT "0E:1B:6C:BA:BD:02:20:60:9A:11:4B:0D:49:25:D5:77:29:58:DA:9A"


#define ANOTHER_DEVICE_TEST 1


//Uncomment for debug purposes
#define SENSORS_DEBUG(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[SENSORS][DEBUG]" fmt "\r\n"), ## __VA_ARGS__)
#define SENSORS_ERROR(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[SENSORS][ERROR]" fmt "\r\n"), ## __VA_ARGS__)
#define WSUNIT_DEBUG(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[WSUNIT][DEBUG]" fmt "\r\n"), ## __VA_ARGS__)
#define WSUNIT_ERROR(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[WSUNIT][ERROR]" fmt "\r\n"), ## __VA_ARGS__)
#define WEBHOOK_ERROR(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[WEBHOOK][ERROR]" fmt "\r\n"), ## __VA_ARGS__)
#define WEBHOOK_DEBUG(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[WEBHOOK][DEBUG]" fmt "\r\n"), ## __VA_ARGS__)
//#define NARODMON_ERROR(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[NARODMON][ERROR]" fmt "\r\n"), ## __VA_ARGS__)
//#define NARODMON_DEBUG(fmt, ...)  Serial.printf_P((PGM_P)PSTR( "[NARODMON][DEBUG]" fmt "\r\n"), ## __VA_ARGS__)
