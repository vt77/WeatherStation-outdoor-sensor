
#include <WeatherStationSensor.h>


#ifndef SENSORS_DEBUG
#define SENSORS_DEBUG(...)
#endif

#ifndef SENSORS_ERROR
#define SENSORS_ERROR(...)
#endif


namespace vt77{


#define MAX_SENSORS_JSON_STRING_LENGTH 256

typedef struct{
    const char * name;
    float temp;
    int  hum;
    int  bat;
    unsigned long last_seen;
    bool ready = false;
    bool avaliable()
    {
        if(this->name == nullptr)
            return false;
        return this->ready;
    }
}device_desc_t;

#define DEVICES_COUNT 3

device_desc_t devices[DEVICES_COUNT] =  {
    {nullptr,0},
    {nullptr,0},
    {nullptr,0},
};

static char buffer[MAX_SENSORS_JSON_STRING_LENGTH];

/* 
 Format :
 [ 
    {'temp': 31.42, 'hum': 58.5, 'dir': 2, 'id': 'room1', 'speed': 0.5},
 ]
 */
class SensorsCollectionJson{
    public:
        void start(const char * name) const
        {
            sprintf(buffer,"{\"mes\":\"%s\",\"devices\":[",name);
        }


        bool insert(const device_desc_t *d,bool full=false) const
        {
            SENSORS_DEBUG("[INSERT]%s T:%.1f H:%d",d->name,d->temp,d->hum);
            JsonStaticString<128> entry;
            entry.start();
            entry.insert("id",d->name);
            entry.insert("temp",d->temp,1);
            entry.insert("hum",d->hum);
            entry.insert("bat",d->bat);
            if(full)
            {
                entry.insert("last",(double)d->last_seen,0);
            }
            entry.close();
            strcat(buffer,(const char *)entry);
            strcat(buffer,",");
            return true;
        }

        void close() const{
            //Remove last comma
            if(buffer[strlen(buffer)-1]==',')
                    buffer[strlen(buffer)-1] = '\0';
            strcat(buffer,"]}");
        }

        operator const char*()const {
            return (const char*)buffer;
        }
};

template <class T>
class SensorsCollection: public WeatherStationSensor
{
    public:
        SensorsCollection(auto receiver,const char *name)
        {
            this->name = name;
            receiver.addListener(this);
        }

        void setChannelNames(const char * names[DEVICES_COUNT])
        {
            for(int i = 0; i<DEVICES_COUNT ; i++)
            {
                if(names[i] != nullptr)
                    devices[i].name = names[i];
            }
        }

        operator const char*()  {
                dataSenderBuffer.start(name);
                for(int i=0;i<DEVICES_COUNT;i++)
                {   
                    if(devices[i].avaliable())
                    {
                        dataSenderBuffer.insert(&devices[i]);
                        devices[i].ready = false;
                    }
                }
                dataSenderBuffer.close();
                return (const char*)dataSenderBuffer;
        }

    //It should be protected. It's still public because of my tests (see bellow)
    //protected:
        void onWeatherData(const WeatherData wd) override {
            SENSORS_DEBUG("[WEATHERDATA][0x%X][%d] T: %.1f H: %d",wd.id,wd.chan,wd.temp/10.0,wd.hum);
            if(wd.chan >= DEVICES_COUNT)
            {
                SENSORS_ERROR("Channel too big %d of %d",wd.chan,DEVICES_COUNT);
                return;
            }

            devices[wd.chan].hum = wd.hum;
            devices[wd.chan].temp = wd.temp / 10.0;
            devices[wd.chan].bat = wd.batt;
            devices[wd.chan].last_seen = time(nullptr);
            devices[wd.chan].ready = true;
        }

    private:
        T dataSenderBuffer;
        const char * name;
};


//Classes to override Data sender handlers
//Simple webhook
class WebhookProtoAmbient : public SensorsCollectionJson{
};


#ifdef NarodmonProto

//Class for narodmon handler
class NarodmonProtoAmbient : public NarodmonProto
{
    public:
        bool insert(const device_desc_t *d) const
        {
            char name [64];
            sprintf(name,"%sTemp",d->name);
            NarodmonProto::insert(name,d->temp/10.0,1);
            sprintf(name,"%sHum",d->name);
            NarodmonProto::insert(name,d->hum,0);
            return true;
        }
};

#endif

#ifdef ANOTHER_DEVICE_TEST

/*
   NOTE ! This ugly class for my own case . I'm using devices from different suppliers (for tests).
   You better use same model for all
*/
template<class D>
class AnotherDevice : WeatherStationSensor{
    public:
        AnotherDevice(auto receiver, D *decorated) : d(decorated)
        {
            setup({500,8,4,2});
            setProtocol(vt77::wsproto_t::PROTO_DGR8H);
            receiver.addListener(this);
        }

    protected:    
        void onWeatherData(const WeatherData wd) override {
            d->onWeatherData(wd);
        }

    private:
        D * d;
};

#endif

}