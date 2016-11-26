#include "wis_rtsp_server.h"
#include "MediaSink.hh"
#include "wis_media_subsession.h"
#include "UsageEnvironment.hh"
#include "BasicUsageEnvironment.hh"
Wis_Rtsp_Server * Wis_Rtsp_Server::createNew(UsageEnvironment& Env,  Port OurPort,
                    UserAuthenticationDatabase* pAuthDatabase,
                      unsigned int iReclamationTestSeconds /*= 65*/)
{
    int iOurSocket = setUpOurSocket(Env, OurPort);
    if (iOurSocket != -1)
    {
        return new Wis_Rtsp_Server(Env, iOurSocket, OurPort, pAuthDatabase, iReclamationTestSeconds);
    }
    return NULL;
}
Wis_Rtsp_Server::Wis_Rtsp_Server(UsageEnvironment& Env, int iOurSocket,
                     Port OurPort,
                     UserAuthenticationDatabase* pAuthDatabase, unsigned int iReclamationTestSeconds)
  : RTSPServer(Env, iOurSocket, OurPort, pAuthDatabase, iReclamationTestSeconds)
{
    OutPacketBuffer::maxSize = 800000; // allow for some possibly large H.264 frames
}

Wis_Rtsp_Server::~Wis_Rtsp_Server()
{
}

ServerMediaSession * Wis_Rtsp_Server:: lookupServerMediaSession(char const* streamName,Boolean isFirstLookupInSession)
{
    ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
    if (sms == NULL)
    {
          sms = ServerMediaSession::createNew(envir(), streamName, streamName);
            addServerMediaSession(sms);
        sms->addSubsession(Wis_Media_SubSession::createNew(envir(),streamName));

    }
    return sms;
}
