#include "simplertpsession.h"



SimpleRtpSession *SimpleRtpSession::createNew(UsageEnvironment &env, const char *sdpDescription)
{
    SimpleRtpSession* newSession = new SimpleRtpSession(env);
    if (newSession != NULL) {
      if (!newSession->initializeWithSDP(sdpDescription)) {
        delete newSession;
        return NULL;
      }
    }


    return newSession;
}

SimpleRtpSession::SimpleRtpSession(UsageEnvironment &env)
    :MediaSession(env)
{

}

MediaSubsession *SimpleRtpSession::createNewMediaSubsession()
{
      return new SimpleRtpMediaSubsession(*this);

}

SimpleRtpMediaSubsession::SimpleRtpMediaSubsession(MediaSession &parent)
    :MediaSubsession(parent)
{

}

Boolean SimpleRtpMediaSubsession::createSourceObjects(int useSpecialRTPoffset)
{
    char* mimeType
      = new char[strlen(mediumName()) + strlen(codecName()) + 2] ;
    sprintf(mimeType, "%s/%s", mediumName(), codecName());
    fReadSource = fRTPSource
      = SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                       fRTPTimestampFrequency, mimeType,
                       (unsigned)useSpecialRTPoffset,
                       True);
    delete[] mimeType;
    return True;

}
