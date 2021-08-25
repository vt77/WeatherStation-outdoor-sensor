#ifndef __VT77_PREFERENCES
#define __VT77_PREFERENCES
namespace vt77{


#define CONFIG_FILENAME "config.dat"

template <typename T>
class Preferences{
      T * p; 
      public:
            Preferences(T * _p): p(_p) {};
      bool load(){
            if(LittleFS.exists(CONFIG_FILENAME) )
            {
                File configFile = LittleFS.open(CONFIG_FILENAME, "r");
                if(configFile)
                {
                    size_t size = configFile.size();
                    configFile.readBytes((char *)p, size);
                    configFile.close();
                    return true;
                }
            }else{
                  Serial.print("Config file not exists ");
                  Serial.println(CONFIG_FILENAME);
            }
            return false;
      }

      bool save(){
            File configFile = LittleFS.open(CONFIG_FILENAME, "w");
            if(configFile)
            {
                 configFile.write((char *)p,sizeof(T));
                 configFile.close();
            }else{
                  Serial.print("Error open config file : ");
                  Serial.println(CONFIG_FILENAME);
            }
            return false;
      }
};

}


#endif