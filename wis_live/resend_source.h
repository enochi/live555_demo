#ifndef RESEND_SOURCE_H
#define RESEND_SOURCE_H
#include<string>
#include<functional>
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
class Resend_Source:public Poco::Runnable
{
public:
    Resend_Source();
    Resend_Source(const char *name,FUNC_ON_NALU_RECV *cb,int window_size=256,int queue_size=8192);
    int pkg_handler(const unsigned char *data,int len);
    void combine_callback();
    int start();
    void stop();
    void run();
    ~Resend_Source();
    std::string _name;
    FUN_RESEND_CB *_cb;
    int _window_size;
    int _queue_size;
    volatile bool _running;

    Poco::Thread _thread;
    int _relate_ip;
};

#endif // RESEND_SOURCE_H
