#include <iostream>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "RTSPCommon.hh"
#include "DarwinInjector.hh"
#include "MatroskaFileServerDemux.hh"
using namespace std;
static const char *prog = "program";
static int use_tcp=1;
// Special code for handling Matroska files:
struct MatroskaFileCreationState {
    MatroskaFile* file;
    char watchVariable;
};
static void onMatroskFileCreation(MatroskaFile* mkvFile, void* clientData) {
    MatroskaFileCreationState *creationState = (MatroskaFileCreationState*)clientData;
    creationState->file = mkvFile;
    creationState->watchVariable = 1;
}
void afterPlayingFunc(void* clientData)
{
    //    MatroskaDemux *demux = (MatroskaDemux* )clientData;
    //    double seektime = 0;
    //    demux->seekToTime(seektime);
    exit(-1);

}
void show_help()
{
    fprintf(stderr,"use as:%s file  rtsp uri [workernum] [-udp] \n",prog);
}
int main(int argc, char *argv[])
{

    if(argc<3)
    {
        show_help();
        return -1;
    }
    const char *fileName = argv[1];
    const char *url = argv[2];


    int workerNums = 1;

    if(argc>3)
    {
        workerNums = atoi(argv[3]);
        fprintf(stdout,"set worker num :%d\n",workerNums);
    }
    if(argc>4)
    {
        if(strcmp(argv[4],"-udp") ==0)
        {
            fprintf(stdout,"use udp\n");
            use_tcp = 0;
        }
    }


    TaskScheduler *pSched = BasicTaskScheduler::createNew();
    UsageEnvironment *pEnv = BasicUsageEnvironment::createNew(*pSched);

    //to get the parsed mvk file, wait for mkv parse end
    MatroskaFileCreationState creationState;
    creationState.watchVariable = 0;
    MatroskaFile::createNew(*pEnv,fileName, onMatroskFileCreation,&creationState);
    pEnv->taskScheduler().doEventLoop(&creationState.watchVariable);

    //choose videotrack and create rtpsink for that track
    unsigned videoTrack = creationState.file->chosenVideoTrackNumber();
    struct in_addr dummyDestAddress;
    Groupsock rtpGroupsockVideo(*pEnv, dummyDestAddress, 0, 0);
    RTPSink *vSink = creationState.file->createRTPSinkForTrackNumber(videoTrack,&rtpGroupsockVideo,96);

    //use MatroskaDemux to get MatroskaDemuxedTrack(source ) for that track
    MatroskaDemux *demux = creationState.file->newDemux();
    FramedSource *baseSource = demux->newDemuxedTrackByTrackNumber(videoTrack);
    if (baseSource == NULL) return NULL;
    unsigned estBitrate,trackBefore;

      // Add a framer in front of the source:
    FramedSource *source = creationState.file->createSourceForStreaming(baseSource,videoTrack,estBitrate,trackBefore);

    RTCPInstance* videoRTCP = NULL;

    DarwinInjector * darwinInjector = DarwinInjector::createNew(*pEnv);

    darwinInjector->addStream(vSink, videoRTCP);
    darwinInjector->setWorkerNums(workerNums);
    darwinInjector->setUrl(url,use_tcp);

    vSink->startPlaying(*source, afterPlayingFunc, demux);

    pEnv->taskScheduler().doEventLoop();

    return 0;
}
