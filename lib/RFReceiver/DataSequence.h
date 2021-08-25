

namespace vt77{

    template <class T>
    class DataSequence{
        public:
        virtual unsigned int size() const = 0;
        virtual unsigned int count() const = 0;
        virtual T last() const = 0;
        virtual T first() const = 0;
        virtual void put(T data) = 0;
        virtual T get()  = 0;
        virtual void clear() = 0;
        virtual bool full() const = 0;
        //T& operator[] (unsigned int i); 	
    };
}
