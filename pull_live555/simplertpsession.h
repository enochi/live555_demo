#ifndef SIMPLERTPSESSION_H
#define SIMPLERTPSESSION_H

#include "liveMedia.hh"

class SimpleRtpSession : public MediaSession
{

public:
    using MediaSession::initializeWithSDP;
  static SimpleRtpSession* createNew(UsageEnvironment& env,
                 char const* sdpDescription);
    SimpleRtpSession(UsageEnvironment& env);
     virtual MediaSubsession* createNewMediaSubsession();
};
class SimpleRtpMediaSubsession: public MediaSubsession
{

public:
    SimpleRtpMediaSubsession(MediaSession& parent);
    virtual Boolean createSourceObjects(int useSpecialRTPoffset);
};

#endif // SIMPLERTPSESSION_H
