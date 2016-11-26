#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "wis_media_subsession.h"
#include "wis_media_source.h"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamFramer.hh"
pid_t gettid()
{
    return syscall(SYS_gettid);
}
//Wis_Media_SubSession::Wis_Media_SubSession():_media_source(NULL)
//{

//}

Wis_Media_SubSession::~Wis_Media_SubSession()
{
    fprintf(stderr,"~Wis_Media_SubSession()\n");
}
Wis_Media_SubSession::Wis_Media_SubSession (UsageEnvironment &env, const char *name)
    : OnDemandServerMediaSubsession(env, True) // reuse the first source
{
    fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
    _media_name = name;
    _media_source = NULL;
    //mp_sdp_line = 0;
}

void Wis_Media_SubSession::startStream(unsigned clientSessionId, void* streamToken,
           TaskFunc* rtcpRRHandler,
           void* rtcpRRHandlerClientData,
           unsigned short& rtpSeqNum,
                       unsigned& rtpTimestamp,
           ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                       void* serverRequestAlternativeByteHandlerClientData)
{
    fprintf(stderr,"Wis_Media_SubSession::startStream %s \n",_media_name.c_str());

//     _media_source->start_source();

    OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken,
                     rtcpRRHandler, rtcpRRHandlerClientData,
                     rtpSeqNum, rtpTimestamp,
                     serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
}

RTPSink *Wis_Media_SubSession::createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
{
    fprintf(stderr, "[%d] %s create rtp sink\n", gettid(), _media_name.c_str());
    return H264VideoRTPSink::createNew(envir(), rtpsock, type);
}

FramedSource *Wis_Media_SubSession::createNewStreamSource (unsigned sid, unsigned &bitrate)
{
     fprintf(stderr, "[%d] %s create stream source\n", gettid(), _media_name.c_str());
    bitrate = 500;
    _media_source = new Wis_Media_Source(envir(),_media_name.c_str());
    _media_source->start_source();
    return H264VideoStreamFramer::createNew(envir(), _media_source);

}
void Wis_Media_SubSession::chkForAuxSDPLine (void *ptr)
{
    Wis_Media_SubSession *This = (Wis_Media_SubSession *)ptr;
    This->chkForAuxSDPLine1();
}

void Wis_Media_SubSession::chkForAuxSDPLine1()
{
    fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
    if (fDummyRTPSink->auxSDPLine())
        _done = 0xff;
    else {
        int delay = 100*1000;	// 100ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(delay,
                                                                 chkForAuxSDPLine, this);
    }
}
const char *Wis_Media_SubSession::getAuxSDPLine (RTPSink *sink, FramedSource *source)
{
    fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
    if (!_sdp_line.empty())
        return _sdp_line.c_str();
    else
        return NULL;

//    fDummyRTPSink = sink;
//    fDummyRTPSink->startPlaying(*source, 0, 0);
//    //mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
//    chkForAuxSDPLine(this);
//    _done = 0;
//    envir().taskScheduler().doEventLoop(&_done);
//    _sdp_line = fDummyRTPSink->auxSDPLine();
//    fDummyRTPSink->stopPlaying();

//    return _sdp_line.c_str();
}
