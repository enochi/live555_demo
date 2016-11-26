#ifndef WIS_MEDIA_SUBSESSION_H
#define WIS_MEDIA_SUBSESSION_H
#include "OnDemandServerMediaSubsession.hh"
#include "wis_media_source.h"
class Wis_Media_SubSession : public OnDemandServerMediaSubsession
{
  public:
    static Wis_Media_SubSession *createNew (UsageEnvironment &env, const char *name)
   {
       return new Wis_Media_SubSession(env, name);
   }
protected:
   Wis_Media_SubSession (UsageEnvironment &env, const char *name);
    virtual void startStream(unsigned clientSessionId, void* streamToken,
               TaskFunc* rtcpRRHandler,
               void* rtcpRRHandlerClientData,
               unsigned short& rtpSeqNum,
                           unsigned& rtpTimestamp,
               ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                           void* serverRequestAlternativeByteHandlerClientData);

    virtual FramedSource* createNewStreamSource( unsigned clientSessionId,  unsigned& estBitrate );
    //virtual void closeStreamSource(FramedSource *inputSource) ;
    virtual const char *getAuxSDPLine (RTPSink *sink, FramedSource *source);
    virtual RTPSink* createNewRTPSink( Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic,  FramedSource* inputSource);

private:
   std::string _media_name;
   char _done;
   std::string _sdp_line;
    Wis_Media_Source* _media_source;
    RTPSink* fDummyRTPSink;
    static void chkForAuxSDPLine (void *ptr);
    void chkForAuxSDPLine1 ();
    ~Wis_Media_SubSession();


};

#endif // WIS_MEDIA_SUBSESSION_H
