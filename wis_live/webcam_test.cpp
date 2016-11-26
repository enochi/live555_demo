#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>



#include "capture.h"
#include "vcompress.h"

static UsageEnvironment *_env = 0;

#define SINK_PORT 3030

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240
#define FRAME_PER_SEC 5.0

pid_t gettid()
{
    return syscall(SYS_gettid);
}


// 使用 webcam + x264
class WebcamFrameSource : public FramedSource
{
    void *mp_capture, *mp_compress;	// v4l2 + x264 encoder
    int m_started;
    void *mp_token;

public:
    WebcamFrameSource (UsageEnvironment &env)
        : FramedSource(env)
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        mp_capture = capture_open("/dev/video0", VIDEO_WIDTH, VIDEO_HEIGHT, PIX_FMT_YUV420P);
        if (!mp_capture) {
            fprintf(stderr, "%s: open /dev/video0 err\n", __func__);
            exit(-1);
        }

        mp_compress = vc_open(VIDEO_WIDTH, VIDEO_HEIGHT, FRAME_PER_SEC);
        if (!mp_compress) {
            fprintf(stderr, "%s: open x264 err\n", __func__);
            exit(-1);
        }

        m_started = 0;
        mp_token = 0;
    }

    ~WebcamFrameSource ()
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);

        if (m_started) {
            envir().taskScheduler().unscheduleDelayedTask(mp_token);
        }

        if (mp_compress)
            vc_close(mp_compress);
        if (mp_capture)
            capture_close(mp_capture);
    }

protected:
    virtual void doGetNextFrame ()
    {
        if (m_started) return;
        m_started = 1;

        // 根据 fps, 计算等待时间
        double delay = 1000.0 / FRAME_PER_SEC;
        int to_delay = delay * 1000;	// us

        mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,
                                                               getNextFrame, this);
    }
    virtual unsigned maxFrameSize() const        // 这个很重要, 如果不设置, 可能导致 getNextFrame() 出现 fMaxSize 小于实际编码帧的情况, 导致图像不完整
    {    return 100*1024; }
private:
    static void getNextFrame (void *ptr)
    {
        ((WebcamFrameSource*)ptr)->getNextFrame1();
    }

    void getNextFrame1 ()
    {
        // capture:
        Picture pic;
        if (capture_get_picture(mp_capture, &pic) < 0) {
            fprintf(stderr, "==== %s: capture_get_picture err\n", __func__);
            m_started = 0;
            return;
        }

        // compress
        const void *outbuf;
        int outlen;
        if (vc_compress(mp_compress, pic.data, pic.stride, &outbuf, &outlen) < 0) {
            fprintf(stderr, "==== %s: vc_compress err\n", __func__);
            m_started = 0;
            return;
        }

        int64_t pts, dts;
        int key;
        vc_get_last_frame_info(mp_compress, &key, &pts, &dts);

        // save outbuf
        gettimeofday(&fPresentationTime, 0);
        fFrameSize = outlen;
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
        else {
            fNumTruncatedBytes = 0;
        }

        memmove(fTo, outbuf, fFrameSize);

        // notify
        afterGetting(this);

        m_started = 0;
    }
};

class WebcamOndemandMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static WebcamOndemandMediaSubsession *createNew (UsageEnvironment &env, FramedSource *source)
    {
        return new WebcamOndemandMediaSubsession(env, source);
    }

protected:
    WebcamOndemandMediaSubsession (UsageEnvironment &env, FramedSource *source)
        : OnDemandServerMediaSubsession(env, True) // reuse the first source
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        mp_source = source;
        mp_sdp_line = 0;
    }

    ~WebcamOndemandMediaSubsession ()
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        if (mp_sdp_line) free(mp_sdp_line);
    }

private:
    static void afterPlayingDummy (void *ptr)
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        // ok
        WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession*)ptr;
        This->m_done = 0xff;
    }

    static void chkForAuxSDPLine (void *ptr)
    {
        WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession *)ptr;
        This->chkForAuxSDPLine1();
    }

    void chkForAuxSDPLine1 ()
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        if (mp_dummy_rtpsink->auxSDPLine())
            m_done = 0xff;
        else {
            int delay = 100*1000;	// 100ms
            nextTask() = envir().taskScheduler().scheduleDelayedTask(delay,
                                                                     chkForAuxSDPLine, this);
        }
    }

protected:
    virtual const char *getAuxSDPLine (RTPSink *sink, FramedSource *source)
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        if (mp_sdp_line) return mp_sdp_line;

        mp_dummy_rtpsink = sink;
        mp_dummy_rtpsink->startPlaying(*source, 0, 0);
        //mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
        chkForAuxSDPLine(this);
        m_done = 0;
        envir().taskScheduler().doEventLoop(&m_done);
        mp_sdp_line = strdup(mp_dummy_rtpsink->auxSDPLine());
        mp_dummy_rtpsink->stopPlaying();

        return mp_sdp_line;
    }

    virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        return H264VideoRTPSink::createNew(envir(), rtpsock, type);
    }

    virtual FramedSource *createNewStreamSource (unsigned sid, unsigned &bitrate)
    {
        fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        bitrate = 500;
        return H264VideoStreamFramer::createNew(envir(), new WebcamFrameSource(envir()));
    }

private:
    FramedSource *mp_source;	// 对应 WebcamFrameSource
    char *mp_sdp_line;
    RTPSink *mp_dummy_rtpsink;
    char m_done;
};

static void test_task (void *ptr)
{
    fprintf(stderr, "test: task ....\n");
    _env->taskScheduler().scheduleDelayedTask(100000, test_task, 0);
}

static void test (UsageEnvironment &env)
{
    fprintf(stderr, "test: begin...\n");

    char done = 0;
    int delay = 100 * 1000;
    env.taskScheduler().scheduleDelayedTask(delay, test_task, 0);
    env.taskScheduler().doEventLoop(&done);

    fprintf(stderr, "test: end..\n");
}

int main (int argc, char **argv)
{
    // env
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    _env = BasicUsageEnvironment::createNew(*scheduler);

    // test
    //test(*_env);

    // rtsp server
    RTSPServer *rtspServer = RTSPServer::createNew(*_env, 8554);
    if (!rtspServer) {
        fprintf(stderr, "ERR: create RTSPServer err\n");
        ::exit(-1);
    }

    // add live stream
    do {
        WebcamFrameSource *webcam_source = 0;

        ServerMediaSession *sms = ServerMediaSession::createNew(*_env, "webcam", 0, "Session from /dev/video0");
        sms->addSubsession(WebcamOndemandMediaSubsession::createNew(*_env, webcam_source));
        rtspServer->addServerMediaSession(sms);

        char *url = rtspServer->rtspURL(sms);
        *_env << "using url \"" << url << "\"\n";
        delete [] url;
    } while (0);

    // run loop
    _env->taskScheduler().doEventLoop();

    return 1;
}
