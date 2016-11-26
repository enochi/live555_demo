#include "resend_source.h"
#include "Poco/RunnableAdapter.h"
#include <stdio.h>
#include <unistd.h>
//#include "arq_to_rtsp_server/pkg_to_nalu.h"
#include "log.h"
using namespace Poco;
Resend_Source::Resend_Source()
{

}

Resend_Source::~Resend_Source()
{
    fprintf(stderr,"%s ~Resend_Source()\n",_name.c_str());
}
Resend_Source::Resend_Source(const char *name,FUN_RESEND_CB *cb,int window_size,int queue_size):_name(name),_cb(cb)
  ,_window_size(window_size),_queue_size(queue_size),_running(false)
{
    _relate_ip = inet_addr(_name.c_str());
}
int Resend_Source::pkg_handler(const unsigned char *data,int len)
{
    return 0;
}

void Resend_Source::run()
{

    fprintf(stderr,"combine_callback thread start\n");
#if 0

    FILE * fd = fopen("test.264","rb");
    if(fd == NULL)
    {
        fprintf(stderr,"open file error\n");
        return ;
    }
    while(_running)
    {
        unsigned char buf[4096] = {0};
        int n = fread(buf,1,sizeof(buf),fd);
        if(n == -1 || n == 0)
        {

            fseek(fd,0,SEEK_SET);
            continue;
        }
        (*_cb)(buf,n);
        usleep(4000);
    }
    fclose(fd);
#else
    while(_running)
    {

    }
#endif
    fprintf(stderr,"combine_callback thread stop\n");
}
int Resend_Source::start()
{
    int ret = -1;
     fprintf(stderr,"Pkg_To_Nalu::add_handler %s  return :%d\n",_name.c_str(),ret);
    _running = true;
    _thread.start(*this);
    return 0;
}
void Resend_Source::stop()
{
    fprintf(stderr,"Resend_Source::stop()\n");
    if(_running)
    {

        _running = false;
        _thread.join();
    }
     fprintf(stderr,"Resend_Source::stop() out\n");
}
