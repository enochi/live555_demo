#include <iostream>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "RTSPCommon.hh"
#include "DarwinInjector.hh"
#include "MatroskaFileServerDemux.hh"
using namespace std;

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

int main(int argc, char *argv[])
{
    if(argc<3)
    {
        fprintf(stderr,"use as:%s file workernum [addr]\n",argv[0]);
        return -1;
    }
    const char *fileName = argv[1];
    int workerNums = atoi(argv[2]);
    const char *remoteFile = "test";
    const char *remoteAddr = "127.0.0.1" ;
    if(argc>3)
        remoteAddr = argv[3];
    const char *sessName = "mkvTest";
    const char *sessInfo = "mkvTestInfo";
    unsigned short port = 8554;
    TaskScheduler *pSched = BasicTaskScheduler::createNew();
    UsageEnvironment *pEnv = BasicUsageEnvironment::createNew(*pSched);

    MatroskaFileCreationState creationState;
    creationState.watchVariable = 0;
    MatroskaFile::createNew(*pEnv,fileName, onMatroskFileCreation,&creationState);
    pEnv->taskScheduler().doEventLoop(&creationState.watchVariable);

    unsigned videoTrack = creationState.file->chosenVideoTrackNumber();
    struct in_addr dummyDestAddress;
    Groupsock rtpGroupsockVideo(*pEnv, dummyDestAddress, 0, 0);
    RTPSink *vSink = creationState.file->createRTPSinkForTrackNumber(videoTrack,&rtpGroupsockVideo,96);


    MatroskaDemux *demux = creationState.file->newDemux();
    FramedSource *baseSource = demux->newDemuxedTrackByTrackNumber(videoTrack);
    if (baseSource == NULL) return NULL;
    unsigned estBitrate,trackBefore;

    FramedSource *source = creationState.file->createSourceForStreaming(baseSource,videoTrack,estBitrate,trackBefore);

    RTCPInstance* videoRTCP = NULL;

    DarwinInjector * darwinInjector = DarwinInjector::createNew(*pEnv);

    darwinInjector->addStream(vSink, videoRTCP);
    darwinInjector->setWorkerNums(workerNums);
    darwinInjector->setDestination(remoteAddr,remoteFile,sessName,sessInfo,port);

    vSink->startPlaying(*source, afterPlayingFunc, demux);

     pEnv->taskScheduler().doEventLoop();

    return 0;
}
