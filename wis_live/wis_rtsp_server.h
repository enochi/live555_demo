#ifndef WIS_RTSP_SERVER_H
#define WIS_RTSP_SERVER_H
#include "RTSPServer.hh"
class Wis_Rtsp_Server : public RTSPServer
{
  public:
    static Wis_Rtsp_Server* createNew(UsageEnvironment& Env, Port ourPort = 554,
                                      UserAuthenticationDatabase* authDatabase = NULL,
                                      unsigned reclamationSeconds = 65);
private:
    /// called only by CreateNew();
    Wis_Rtsp_Server(UsageEnvironment& Env, int iOurSocket, Port OurPort,
            UserAuthenticationDatabase* AuthDatabase, unsigned int iReclamationTestSeconds);

    virtual ~Wis_Rtsp_Server();


    /// redefined virtual functions
    virtual ServerMediaSession* lookupServerMediaSession( char const* StreamName, Boolean isFirstLookupInSession = True);

};

#endif // WIS_RTSP_SERVER_H
