TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../live555/testProgs/MPEG2TransportStreamIndexer.cpp \
    ../live555/testProgs/openRTSP.cpp \
    ../live555/testProgs/playCommon.cpp \
    ../live555/testProgs/playSIP.cpp \
    ../live555/testProgs/registerRTSPStream.cpp \
    ../live555/testProgs/sapWatch.cpp \
    ../live555/testProgs/testAMRAudioStreamer.cpp \
    ../live555/testProgs/testDVVideoStreamer.cpp \
    ../live555/testProgs/testGSMStreamer.cpp \
    ../live555/testProgs/testH264VideoStreamer.cpp \
    ../live555/testProgs/testH264VideoToTransportStream.cpp \
    ../live555/testProgs/testH265VideoStreamer.cpp \
    ../live555/testProgs/testH265VideoToTransportStream.cpp \
    ../live555/testProgs/testMKVStreamer.cpp \
    ../live555/testProgs/testMP3Receiver.cpp \
    ../live555/testProgs/testMP3Streamer.cpp \
    ../live555/testProgs/testMPEG1or2AudioVideoStreamer.cpp \
    ../live555/testProgs/testMPEG1or2ProgramToTransportStream.cpp \
    ../live555/testProgs/testMPEG1or2Splitter.cpp \
    ../live555/testProgs/testMPEG1or2VideoReceiver.cpp \
    ../live555/testProgs/testMPEG1or2VideoStreamer.cpp \
    ../live555/testProgs/testMPEG2TransportReceiver.cpp \
    ../live555/testProgs/testMPEG2TransportStreamer.cpp \
    ../live555/testProgs/testMPEG2TransportStreamTrickPlay.cpp \
    ../live555/testProgs/testMPEG4VideoStreamer.cpp \
    ../live555/testProgs/testOggStreamer.cpp \
    ../live555/testProgs/testOnDemandRTSPServer.cpp \
    ../live555/testProgs/testRelay.cpp \
    ../live555/testProgs/testReplicator.cpp \
    ../live555/testProgs/testRTSPClient.cpp \
    ../live555/testProgs/testWAVAudioStreamer.cpp \
    ../live555/testProgs/vobStreamer.cpp

DISTFILES += \
    ../live555/testProgs/testMP3-using-ADUs.sdp \
    ../live555/testProgs/testMP3.sdp \
    ../live555/testProgs/testMPEG1or2AudioVideo.sdp \
    ../live555/testProgs/testMPEG1or2Video.sdp \
    ../live555/testProgs/testMPEG2Transport.sdp \
    ../live555/testProgs/testProgs.mak \
    ../live555/testProgs/COPYING \
    ../live555/testProgs/Makefile.head \
    ../live555/testProgs/Makefile.tail

HEADERS += \
    ../live555/testProgs/playCommon.hh

