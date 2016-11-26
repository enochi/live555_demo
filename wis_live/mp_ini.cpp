
#include "mp_ini.h"
#include "dotini_parser.h"
#include <cstdio>

bool MpIni_Handler::init()
{
    int rc = dotini_open("mp.ini");

    if (rc != 0)
        return false;


    cv_enable = dotini_get_integer("yuv_pre_process", "cv_enable");
    cv_color = dotini_get_integer("yuv_pre_process", "cv_color");
    canny = dotini_get_integer("yuv_pre_process", "canny");
    smooth = dotini_get_integer("yuv_pre_process", "smooth");
    laplace = dotini_get_integer("yuv_pre_process", "laplace");

    use_fec = dotini_get_integer("fec", "use_fec");

    window_size = dotini_get_integer("resend", "window_size");
    start_port = dotini_get_integer("port_manager", "start_port");
    port_range = dotini_get_integer("port_manager", "port_range");


    codec_type = dotini_get_integer("audio", "codec_type");
    pre_pcm = dotini_get_integer("audio", "pre_pcm");

    resend = dotini_get_integer("qos", "resend");
    xproc = dotini_get_integer("crypt", "xproc");
    
    fps = dotini_get_integer("vsyn", "fps");
    min_fps = dotini_get_integer("vsyn", "min_fps");
    max_fps = dotini_get_integer("vsyn", "max_fps");
    bitrate = dotini_get_integer("vsyn", "bitrate");

    low_bandwith_gop = dotini_get_integer("low_bandwidth", "gop");
    low_bandwith_qop_min = dotini_get_integer("low_bandwidth", "qop_min");
    low_bandwith_qop_max = dotini_get_integer("low_bandwidth", "qop_max");
    low_bandwith_qop_step = dotini_get_integer("low_bandwidth", "qop_step");
    low_bandwith_fps = dotini_get_integer("low_bandwidth", "fps");

    force = dotini_get_integer("force", "valid");
    force_gop = dotini_get_integer("force", "gop");
    force_qop_min = dotini_get_integer("force", "qop_min");
    force_qop_max =dotini_get_integer("force", "qop_max");
    force_qop_step = dotini_get_integer("force", "qop_step");
    //modified by Leon on 20130819

    max_decode_count = dotini_get_integer("capacity", "max_decode_count");
    max_encode_count = dotini_get_integer("capacity","max_encode_count");
    max_video_forward_count = dotini_get_integer("capacity","max_video_forward_count");
    max_audio_forward_count = dotini_get_integer("capacity","max_audio_forward_count");


    if(min_fps <= 0 || min_fps > 40)
    {
        min_fps = 20;
    }

    if(max_fps <= min_fps || max_fps > 160)
    {
        max_fps = 120;
    }

    if(fps < min_fps || fps > max_fps)
    {
        fps = (min_fps + max_fps) / 2;
    }
    sse41 = dotini_get_integer("cpu", "sse41");
    licence_type = dotini_get_integer("extra", "licence_type");
    if(licence_type == 1)
    {
        const char *host = dotini_get_string("extra", "host");
        if(NULL != host)
        {
            host_ip = host;
        }
        fprintf(stderr,"get host ip:%s\n",host_ip.c_str());
    }
    send_init_pkg = dotini_get_integer("extra", "send_init_pkg");
    qos_send_interval = dotini_get_integer("extra", "qos_send_interval");
    qos_wait_method = dotini_get_integer("extra", "qos_wait_method");
    const char *ip = dotini_get_string("extra", "mp_ip");
    if( NULL != ip)
    {
        mp_ip =  ip;
    }
    dotini_close();
    return true;
}
