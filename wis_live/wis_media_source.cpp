#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "UsageEnvironment.hh"
#include "wis_media_source.h"


using namespace Poco;
using namespace std;
using namespace std::placeholders;
//pid_t gettid()
//{
//    return syscall(SYS_gettid);
//}
Wis_Media_Source::Wis_Media_Source(UsageEnvironment& Env,const char * name):FramedSource(Env)
  ,_name(name),_started(false),_max_nalu_queue(10)
{

}
Wis_Media_Source::~Wis_Media_Source()
{
    fprintf(stderr,"%s ~Wis_Media_Source() in\n",_name.c_str());
     Pkg_Receiver::instance().set_callback(_name,NULL);
    fprintf(stderr,"%s ~Wis_Media_Source() out\n",_name.c_str());
}
int Wis_Media_Source::start_source()
{
    fprintf(stderr,"Wis_Media_Source::start_source()\n");
    if(!_started)
    {
        nalu_callback = std::bind(&Wis_Media_Source::on_nalu,this,_1,_2,_3);
        Pkg_Receiver::instance().set_callback(_name,&nalu_callback);
        _started = true;
    }
    return 0;
}
void Wis_Media_Source::doStopGettingFrames()
{
    fprintf(stderr,"Wis_Media_Source::doStopGettingFrames()\n");
    if(_started)
    {
        _started = false;

    }
}
void Wis_Media_Source::doGetNextFrame()
{
    getNextFrame1 ();
}
void Wis_Media_Source::getNextFrame (void *ptr)
{
    ((Wis_Media_Source*)ptr)->getNextFrame1();
}

void Wis_Media_Source::getNextFrame1 ()
{
    if(!_started)
        return ;
#if 0
    static FILE * fd = fopen("test.264","rb");
    char buf[4096]={0};
    int n = fread(buf,1,sizeof(buf),fd);
    if(n ==-1 || n == 0)
    {
        fseek(fd,0,SEEK_SET);
        n = fread(buf,1,sizeof(buf),fd);
    }
    fFrameSize = n;
    if (fFrameSize > fMaxSize) {
        fNumTruncatedBytes = fFrameSize - fMaxSize;
        fFrameSize = fMaxSize;
    }
    else {
        fNumTruncatedBytes = 0;
    }
    memcpy(fTo, buf, n);
    afterGetting(this);
#else

    if( 0 ==pop_nalu(_frame_buf) )
    {
        if(_frame_buf.len == 0)
        {
            fprintf(stderr,"wthfffffff\n");
            return ;
        }
        //fprintf(stderr,"get nalu len:%d\n",frame_buf.len);
        gettimeofday(&fPresentationTime, 0);
        fFrameSize = _frame_buf.len;
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
        else {
            fNumTruncatedBytes = 0;
        }
        memcpy(fTo, _frame_buf.buf, fFrameSize);
        afterGetting(this);
    }
    else
    {
        return;
    }
#if 0

    static FILE *fddebug =  fopen(_name.c_str(),"wb");
    if(fddebug)
        fwrite(frame_buf.buf,1,fFrameSize,fddebug);
#endif
#endif

}
int Wis_Media_Source::insert_nalu( const char* data,int length)
{

    //fprintf(stderr,"%s insert nalu:%d\n",_name.c_str(),length);
    if(_started)
    {

        ScopedLock<Mutex> lock(_mutex);

        if(_buf_list.size() >= _max_nalu_queue)
        {
            fprintf(stderr,"%s insert nalu queue more than%d,now pop\n",_name.c_str(),_max_nalu_queue);
            _buf_list.pop_front();
        }
        T_Framebuf buf((unsigned char*)data,length);
        _buf_list.push_back(buf);
        //_pop_event.set();
    }
    return 0;
}
int Wis_Media_Source::pop_nalu(T_Framebuf &framebuf)
{
    if(!_started)
        return -1;
    ScopedLock<Mutex> lock(_mutex);
    if(!_buf_list.empty())// to make sure
    {
        framebuf = *_buf_list.begin();
        _buf_list.pop_front();
        //_insert_event.set();
        return 0;
    }
    else
    {
        //should not stop here,for this will stop all the event loop
        //fprintf(stderr,"get nalu empty\n");
        double delay = 1000.0 / 30;
        int to_delay = delay * 1000;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(to_delay,getNextFrame, this);
        return -1;

    }

}
int Wis_Media_Source::on_nalu(const char *data,int len,const std::string remote)
{
    return insert_nalu(data,len);
}
