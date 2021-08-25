#include <functional>

#define MAX_TASKS 10

namespace vt77 {


int count = 0;
class Tasker
{
    typedef std::function<void(void)> callback_function_t;
    typedef struct 
    {
        unsigned long delay;
        unsigned long lastMillis;
        callback_function_t callback;
    }__task_t;
    
    
    private:
        __task_t tasks[MAX_TASKS];
    public:
         /**
         *  Attach job to tasker
         *  
         *  @param delay in seconds to execute task
         *  @param task  callback function to be executed
         * 
         */
        void attach(int delay,callback_function_t task){
            tasks[count].delay  = delay * 1000L; //In msec
            tasks[count].callback = task;
            tasks[count++].lastMillis = millis();
        }

        void process()
        {   
            for(int i=0;i<count;i++)
            {
                if(millis() - tasks[i].lastMillis > tasks[i].delay )
                {
                    tasks[i].lastMillis = millis();
                    tasks[i].callback();
                }
            }
        }
};

}