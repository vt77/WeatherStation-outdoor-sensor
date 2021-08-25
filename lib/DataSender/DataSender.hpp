

namespace vt77 {
    class DataSender
    {
        public:
            virtual void init(const char * dsn) = 0;
            virtual void send_data(const char * access_token, const char * body)=0;
    };
}