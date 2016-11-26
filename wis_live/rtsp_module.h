#ifndef RTSP_MODULE_H
#define RTSP_MODULE_H
#include "wis_rtsp_server.h"
#include "BasicUsageEnvironment.hh"
class Rtsp_Module
{
public:
    Rtsp_Module();
    ~Rtsp_Module();
    int start(int port =554);
    int stop();
private:
    TaskScheduler * _scheduler;
    UsageEnvironment *_env;
    TaskToken *_task_token;
    char fStop;
};

#endif // RTSP_MODULE_H
