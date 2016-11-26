#ifndef MP_INI_H_INCLUDED
#define MP_INI_H_INCLUDED

#include <string>
class MpIni_Handler{

public:
    bool init();
    static MpIni_Handler & instance()
    {
        static MpIni_Handler _instance;
        return _instance;
    }

public:
    int  cv_enable;
    int  cv_color;
    int  canny;
    int  smooth;
    int  laplace;
    int  use_fec;
    int  window_size;
    int  start_port;
    int  port_range;
    int  codec_type;
    int  resend;
    int  xproc;
    int  fps;
    int  min_fps;
    int  max_fps;
    int  bitrate;
    int	 sse41;
//低带宽下特定参数
    int low_bandwith_gop;
    int low_bandwith_qop_min;
    int low_bandwith_qop_max;
    int low_bandwith_qop_step;
    int low_bandwith_fps;
//强制参数 调试使用
    int force;
    int force_gop;
    int force_qop_min;
    int force_qop_max;
    int force_qop_step;

    //modified by Leon on 20130819
    int max_decode_count;
    int max_encode_count;

    int max_video_forward_count;
    int max_audio_forward_count;
    int pre_pcm;
    int licence_type;
    std::string host_ip;
    std::string mp_ip;

    int qos_send_interval;//QOS发送的时间间隔
    int send_init_pkg;//是否发送init包
    int qos_wait_method;//0：等待下一个I帧，1，等待下一个I帧或者P帧
private:
    MpIni_Handler()
    {

    }
};

#endif // MP_INI_H_INCLUDED
