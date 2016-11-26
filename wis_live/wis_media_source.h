#ifndef WIS_MEDIA_SOURCE_H
#define WIS_MEDIA_SOURCE_H
#include <string>
#include <list>
#include "FramedSource.hh"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "resend_protocol/pkg_recevicer.h"
#define MAX_FRAME_SIZE 1024*1024

typedef struct _T_Framebuf
{
    _T_Framebuf(const unsigned char* data,int length)
    {
        memcpy(buf,data,length);
        len = length;
    }
    _T_Framebuf(){}
   unsigned char buf[MAX_FRAME_SIZE];
   int len;
}T_Framebuf;

class Wis_Media_Source : public FramedSource
{
public:
    int  on_nalu(const char *data,int len,const std::string remote);
    int start_source();
    Wis_Media_Source(UsageEnvironment& Env,const char * name);
    virtual ~Wis_Media_Source();
protected:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
    virtual unsigned maxFrameSize() const
    {
        return MAX_FRAME_SIZE;
    }
private:
    static void getNextFrame (void *ptr);
     void getNextFrame1 ();
     int insert_nalu( const char* data,int len);
     int pop_nalu(T_Framebuf &framebuf);
    std::string _name;
    std::list<T_Framebuf>  _buf_list;
    volatile bool _started;
    int _max_nalu_queue;
   // Resend_Source *_resend_source;
    Poco::Mutex _mutex;
//    Poco::Event _insert_event;
//    Poco::Event _pop_event;
//    Poco::Event  _wait_data_end;
    FUNC_ON_NALU_RECV nalu_callback;
   // volatile char _have_data;
    T_Framebuf _frame_buf;
    TaskToken _task_token;
};



#endif // WIS_MEDIA_SOURCE_H
