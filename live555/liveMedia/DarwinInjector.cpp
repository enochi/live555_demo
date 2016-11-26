/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// An object that redirects one or more RTP/RTCP streams - forming a single
// multimedia session - into a 'Darwin Streaming Server' (for subsequent
// reflection to potentially arbitrarily many remote RTSP clients).
// Implementation

#include "DarwinInjector.hh"
#include "GroupsockHelper.hh"
#include "RTPSink.hh"
#include "RTSPClient.hh"
#include <string>
#include <sstream>
////////// SubstreamDescriptor definition //////////

class SubstreamDescriptor {
public:
    SubstreamDescriptor(RTPSink* rtpSink, RTCPInstance* rtcpInstance, unsigned trackId);
    ~SubstreamDescriptor();

    SubstreamDescriptor*& next() { return fNext; }
    RTPSink* rtpSink() const { return fRTPSink; }
    RTCPInstance* rtcpInstance() const { return fRTCPInstance; }
    char const* sdpLines() const { return fSDPLines; }

private:
    SubstreamDescriptor* fNext;
    RTPSink* fRTPSink;
    RTCPInstance* fRTCPInstance;
    char* fSDPLines;
};


////////// DarwinInjector implementation //////////

DarwinInjector* DarwinInjector::createNew(UsageEnvironment& env,
                                          char const* applicationName,
                                          int verbosityLevel) {
    return new DarwinInjector(env, applicationName, verbosityLevel);
}

Boolean DarwinInjector::lookupByName(UsageEnvironment& env, char const* name,
                                     DarwinInjector*& result) {
    result = NULL; // unless we succeed

    Medium* medium;
    if (!Medium::lookupByName(env, name, medium)) return False;

    //  if (!medium->isDarwinInjector()) {
    //    env.setResultMsg(name, " is not a 'Darwin injector'");
    //    return False;
    //  }

    result = (DarwinInjector*)medium;
    return True;
}

DarwinInjector::DarwinInjector(UsageEnvironment& env,
                               char const* applicationName, int verbosityLevel)
    : Medium(env),
      fApplicationName(strDup(applicationName)), fVerbosityLevel(verbosityLevel),
      fSubstreamSDPSizes(0),
      fHeadSubstream(NULL), fTailSubstream(NULL), fLastTrackId(0) ,fWorkerNums(1){
}

DarwinInjector::~DarwinInjector() {
    delete fHeadSubstream;
    delete[] (char*)fApplicationName;
}

void DarwinInjector::addStream(RTPSink* rtpSink, RTCPInstance* rtcpInstance) {
    if (rtpSink == NULL) return; // "rtpSink" should be non-NULL

    SubstreamDescriptor* newDescriptor = new SubstreamDescriptor(rtpSink, rtcpInstance, ++fLastTrackId);
    if (fHeadSubstream == NULL)
    {
        fHeadSubstream = fTailSubstream = newDescriptor;
    }
    else
    {
        fTailSubstream->next() = newDescriptor;
        fTailSubstream = newDescriptor;
    }

    fSubstreamSDPSizes += strlen(newDescriptor->sdpLines());
}

// Define a special subclass of "RTSPClient" that has a pointer field to a "DarwinInjector".  We'll use this to implement RTSP ops:
class RTSPClientForDarwinInjector: public RTSPClient 
{
public:

    RTSPClientForDarwinInjector(UsageEnvironment& env, char const* rtspURL, int verbosityLevel, char const* applicationName,
                                DarwinInjector* ourDarwinInjector)
        : RTSPClient(env, rtspURL,verbosityLevel, applicationName,0,-1),
          fOurDarwinInjector(ourDarwinInjector) {}
    virtual ~RTSPClientForDarwinInjector() {}
    DarwinInjector* fOurDarwinInjector;
};
struct RtspTransport
{

    bool use_tcp;
    char chn_id_min;
    char chn_id_max;
    unsigned short server_port_min;
    unsigned short server_port_max;
//    unsigned short client_port_min;
//    unsigned short client_port_max;
};

//Transport: RTP/AVP/TCP;unicast;mode=receive;interleaved=0-1
 //RTP/AVP;unicast;client_port=47672-47673;server_port=57200-57201
static int parseTransport(const char *str,RtspTransport *transport){
    std::string tmp = str;
    int pos = tmp.find("RTP/AVP/TCP");
    int n = 0;
    if(pos != -1){
        transport->use_tcp = true;
        pos = tmp.find("interleaved");
        if(pos != -1){
             n = sscanf(tmp.c_str()+pos,"interleaved=%u-%u",&transport->chn_id_min,&transport->chn_id_max);
             if(n != 2)
                 return -1;
        }else{
            return -1;
        }

    }else {
       pos = tmp.find("server_port");
       if(pos != -1){
           n=sscanf(tmp.c_str()+pos,"server_port=%u-%u",&transport->server_port_min,&transport->server_port_max);
           if(n != 2)
               return -1;
       }
    }
    return 0;
}
Boolean DarwinInjector::setUrl(char const* url, Boolean useTcp) {
    char tmp[256]={0};
    strcpy(tmp,url);
    std::string remoteUrl = url;
    int pos = remoteUrl.rfind(".sdp");
    if(pos  != -1)  {
        remoteUrl.erase(pos);
    }
    const char *remoteFile = "test";
    const char *remoteAddr = NULL;
    int port = 8554;
    if(strstr(tmp,"rtsp://") != NULL){

        char * p = tmp+ strlen("rtsp://");
        remoteAddr = p;
        p = strpbrk(p,":/");
        if(p) {//have port or remote filename
            if(*p == ':'){//have port{
                port = atoi(p+1);
            }
            *p='\0';
            //get remote file
            p = strrchr(p+1,'/');
            if(p && p[1] != '\0')
                remoteFile = p+1;
        }
        fprintf(stdout,"set remote addr :%s:%d,remote file:%s \n",remoteAddr,port,remoteFile);

    }else {
        envir()<<"set url error "<<url;
        return False;
    }

    for(int i=0;i<fWorkerNums;++i){
        do {
            // Begin by creating our RTSP client object:
            std::stringstream actualUrlss;
            actualUrlss<<remoteUrl<<"_"<<i<<".sdp";

            RTSPClient *client = new RTSPClientForDarwinInjector(envir(), actualUrlss.str().c_str(), fVerbosityLevel, fApplicationName, this);
            if (client == NULL) break;
            // Get the remote RTSP server's IP address:

            struct in_addr addr;{
                NetAddressList addresses(remoteAddr);
                if (addresses.numAddresses() == 0) break;
                NetAddress const* address = addresses.firstAddress();
                addr.s_addr = *(unsigned*)(address->data());
            }
            // Construct a SDP description for the session that we'll be streaming:
            char const* const sdpFmt =
                    "v=0\r\n"
                    "o=- %u %u IN IP4 127.0.0.1\r\n"
                    "s=%s_%d\r\n"
                    "i=%s\r\n"
                    "c=IN IP4 %s\r\n"
                    "t=0 0\r\n";

            unsigned sdpLen = strlen(sdpFmt)
                    + 20 /* max int len */ + 20 /* max int len */
                    + strlen(remoteFile)+10
                    + strlen(remoteFile)
                    + strlen(remoteAddr)

                    + fSubstreamSDPSizes;
            unsigned const sdpSessionId = our_random32();
            unsigned const sdpVersion = sdpSessionId;
            char sdp[2048]={0};
            sprintf(sdp, sdpFmt,
                    sdpSessionId, sdpVersion, // o= line
                    remoteFile,i, // s= line
                    remoteFile, // i= line
                    remoteAddr// c= line
                    //                    sessionName, // a=x-qt-text-nam: line

                    //                    sessionInfo, // a=x-qt-text-inf: line
                    //                    fApplicationName, // a=x-qt-text-cmt: line
                    //                    sessionAuthor, // a=x-qt-text-aut: line
                    //                    sessionCopyright // a=x-qt-text-cpy: line
                    );
            char* p = &sdp[strlen(sdp)];
            SubstreamDescriptor* ss;
            for (ss = fHeadSubstream; ss != NULL; ss = ss->next()) {
                sprintf(p, "%s", ss->sdpLines());
                p += strlen(p);
            }

            // Do a RTSP "ANNOUNCE" with this SDP description:

            Authenticator* authToUse = NULL;

            fWatchVariable = 0;
            (void)client->sendAnnounceCommand(sdp, genericResponseHandler, authToUse);

            // Now block (but handling events) until we get a response:
            envir().taskScheduler().doEventLoop(&fWatchVariable);

            if (fResultCode != 0) break; // an error occurred with the RTSP "ANNOUNCE" command

            // Next, tell the remote server to start receiving the stream from us.
            // (To do this, we first create a "MediaSession" object from the SDP description.)
            MediaSession *mediaSession = MediaSession::createNew(envir(), sdp,MediaSession::SESSION_PUSH);
            if (mediaSession == NULL) break;

            MediaSubsessionIterator iter(*mediaSession);
            MediaSubsession* subsession;
            ss = fHeadSubstream;
            unsigned streamChannelId = 0;
            while ((subsession = iter.next()) != NULL) {
                if (!subsession->initiate()) break;

                fWatchVariable = 0;
                (void)client->sendSetupCommand(*subsession, genericResponseHandler,
                                               True /*streamOutgoing*/,
                                               useTcp /*streamUsingTCP*/);
                // Now block (but handling events) until we get a response:
                envir().taskScheduler().doEventLoop(&fWatchVariable);

                if (fResultCode != 0) break; // an error occurred with the RTSP "SETUP" command

                //RtspTransport transport;
                //if(parseTransport(fResultString.c_str(),&transport) !=0) break;


                // Tell this subsession's RTPSink and RTCPInstance to use
                // the RTSP TCP connection:
                if(useTcp)
                {
                    ss->rtpSink()->addStreamSocket(client->socketNum(), streamChannelId++);
                    if(ss->rtcpInstance() == NULL) streamChannelId++;
                    if (ss->rtcpInstance() != NULL) {
                        ss->rtcpInstance()->addStreamSocket(client->socketNum(),
                                                            streamChannelId++);
                    }
                }
                else
                {
                    //!!todo sessionid not 4 byte
                    unsigned int session_id = atoi(subsession->sessionId());
                    fprintf(stderr,"get remote addr:%s,port:%d\n",subsession->connectionEndpointName(),subsession->clientPortNum());
                    ss->rtpSink()->groupsockBeingUsed().addDestination(addr,subsession->clientPortNum(),session_id);
                }
                ss = ss->next();
            }
            if (subsession != NULL) break; // an error occurred above

            // Tell the RTSP server to start:
            fWatchVariable = 0;
            (void)client->sendPlayCommand(*mediaSession, genericResponseHandler);

            // Now block (but handling events) until we get a response:
            envir().taskScheduler().doEventLoop(&fWatchVariable);

            if (fResultCode != 0) break; // an error occurred with the RTSP "PLAY" command

            // Finally, make sure that the output TCP buffer is a reasonable size:
            increaseSendBufferTo(envir(), client->socketNum(), 100*1024);
            fSessions.push_back(DarwinClientSession(client,mediaSession));

        } while (0);

    }
    if(fSessions.size() == fWorkerNums)
    {
        fprintf(stderr,"setup all workers success\n");
        return true;
    }
    else
    {
        fprintf(stderr,"failed to  set %d workers \n",fWorkerNums-fSessions.size());
        closeAll();
        return false;
    }
}

Boolean DarwinInjector::isDarwinInjector() const {
    return True;
}

void DarwinInjector::closeAll()
{

    for(std::vector<DarwinClientSession>::iterator iter=fSessions.begin();iter!=fSessions.end();++iter ) { // close down and delete the session
        iter->fClient->sendTeardownCommand(*iter->fSession, NULL);
        Medium::close(iter->fSession);
        Medium::close(iter->fClient);
    }
    fSessions.clear();
}
//RTP/AVP;unicast;client_port=47672-47673;server_port=57200-57201

void DarwinInjector::genericResponseHandler(RTSPClient* rtspClient, int responseCode, char* responseString) {
    fprintf(stdout,"recv respons code:%d str:%s\n",responseCode,responseString);
    DarwinInjector* di = ((RTSPClientForDarwinInjector*)rtspClient)-> fOurDarwinInjector;
    di->genericResponseHandler1(responseCode, responseString);
}

void DarwinInjector::genericResponseHandler1(int responseCode, char* responseString) {
    // Set result values:
    fResultCode = responseCode;
    if(responseString)
        fResultString = responseString;
    // Signal a break from the event loop (thereby returning from the blocking command):
    fWatchVariable = ~0;
}

////////// SubstreamDescriptor implementation //////////

SubstreamDescriptor::SubstreamDescriptor(RTPSink* rtpSink,
                                         RTCPInstance* rtcpInstance, unsigned trackId)
    : fNext(NULL), fRTPSink(rtpSink), fRTCPInstance(rtcpInstance) {
    // Create the SDP description for this substream
    char const* mediaType = fRTPSink->sdpMediaType();
    unsigned char rtpPayloadType = fRTPSink->rtpPayloadType();
    char const* rtpPayloadFormatName = fRTPSink->rtpPayloadFormatName();
    unsigned rtpTimestampFrequency = fRTPSink->rtpTimestampFrequency();
    unsigned numChannels = fRTPSink->numChannels();
    char* rtpmapLine;
    if (rtpPayloadType >= 96) {
        char* encodingParamsPart;
        if (numChannels != 1) {
            encodingParamsPart = new char[1 + 20 /* max int len */];
            sprintf(encodingParamsPart, "/%d", numChannels);
        } else {
            encodingParamsPart = strDup("");
        }
        char const* const rtpmapFmt = "a=rtpmap:%d %s/%d%s\r\n";
        unsigned rtpmapFmtSize = strlen(rtpmapFmt)
                + 3 /* max char len */ + strlen(rtpPayloadFormatName)
                + 20 /* max int len */ + strlen(encodingParamsPart);
        rtpmapLine = new char[rtpmapFmtSize];
        sprintf(rtpmapLine, rtpmapFmt,
                rtpPayloadType, rtpPayloadFormatName,
                rtpTimestampFrequency, encodingParamsPart);
        delete[] encodingParamsPart;
    } else {
        // Static payload type => no "a=rtpmap:" line
        rtpmapLine = strDup("");
    }
    unsigned rtpmapLineSize = strlen(rtpmapLine);
    char const* auxSDPLine = fRTPSink->auxSDPLine();
    if (auxSDPLine == NULL) auxSDPLine = "";
    unsigned auxSDPLineSize = strlen(auxSDPLine);

    char const* const sdpFmt =
            "m=%s 0 RTP/AVP %u\r\n"
            "%s" // "a=rtpmap:" line (if present)
            "%s" // auxilliary (e.g., "a=fmtp:") line (if present)
            "a=control:trackID=%u\r\n";
    unsigned sdpFmtSize = strlen(sdpFmt)
            + strlen(mediaType) + 3 /* max char len */
            + rtpmapLineSize
            + auxSDPLineSize
            + 20 /* max int len */;
    char* sdpLines = new char[sdpFmtSize];
    sprintf(sdpLines, sdpFmt,
            mediaType, // m= <media>
            rtpPayloadType, // m= <fmt list>
            rtpmapLine, // a=rtpmap:... (if present)
            auxSDPLine, // optional extra SDP line
            trackId); // a=control:<track-id>
    fSDPLines = strDup(sdpLines);
    delete[] sdpLines;
    delete[] rtpmapLine;
}

SubstreamDescriptor::~SubstreamDescriptor() {
    delete fSDPLines;
    delete fNext;
}
