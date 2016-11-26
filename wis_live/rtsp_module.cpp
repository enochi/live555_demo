#include "rtsp_module.h"
#include "BasicUsageEnvironment.hh"
#include "resend_protocol/pkg_recevicer.h"
Rtsp_Module::Rtsp_Module()
{

}

Rtsp_Module::~Rtsp_Module()
{

}

int Rtsp_Module::start(int port)
{
    // env
    fStop = 0;
    Pkg_Receiver::instance().init(4602);
    Pkg_Receiver::instance().start(&fStop);
    _scheduler = BasicTaskScheduler::createNew();
    _env = BasicUsageEnvironment::createNew(*_scheduler);

    Wis_Rtsp_Server *rtspServer = Wis_Rtsp_Server::createNew(*_env, port);
    if (!rtspServer) {
        fprintf(stderr, "ERR: create RTSPServer err port:%d\n",port);
        ::exit(-1);
    }
    // run loop
    _env->taskScheduler().doEventLoop(&fStop);
    fprintf(stderr,"rtsp module end !!!\n");
    return 0;
}
int Rtsp_Module::stop()
{
    Pkg_Receiver::instance().stop();
    fStop = ~0;
    return 0;
}
