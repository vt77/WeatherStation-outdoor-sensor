
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

enum CaptivePortalMode {
    MODE_LOCAL = 0,
    MODE_PUBLIC = 1
};

typedef std::function<void(void)> save_preferences_t;
typedef std::function<String(void)> get_preferences_t;


ESP8266WebServer* get_webserver();

class CaptivePortal
{
    public:
        const static get_preferences_t preferencesJson;
        void start(CaptivePortalMode mode );
        void process();
        void onPreferences(save_preferences_t func);
        ESP8266WebServer* webserver();
        const String get(const char * keyname);
};
