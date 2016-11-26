#include <cstdio>
#include <cstring>
using namespace std;

#include "resend_protocol_send_handler.h"
#include "resend_protocol.h"
#include "ring_buf_handler.h"
#include "xproc.h"
#include "mp_ini.h"
#ifdef LEIZHOU

Resend_Protocol_Send_Handler::Resend_Protocol_Send_Handler():_fps_time("fpstime",false)
{
	 memset(&_nalu_count,0,sizeof(_nalu_count));

}

int Resend_Protocol_Send_Handler::operator()(unsigned char *fnalu_buf, int fnalu_buf_len, SEND_FUNC send_func)
{
    ++_nalu_count._frames;
    _nalu_count._total_bytes += fnalu_buf_len;
    int elapsed = _fps_time.get_elapsed()/1000000;
    if(elapsed > 10)
    {
        _nalu_count._fps =_nalu_count. _frames/(elapsed);
        _nalu_count._bitrate = _nalu_count._total_bytes*8/(1024*elapsed);
        fprintf(stderr,"pid:%d,send fps:%d,bitrate:%dkbps in %dsec\n",getpid(),_nalu_count._fps,_nalu_count._bitrate,elapsed);
        _fps_time.update();
         _nalu_count._frames = 0;
         _nalu_count._total_bytes = 0;
    }
    Package_Buf *p_package_buf;
    unsigned char        *p_fanlu_buf = fnalu_buf;
    int reserve = fnalu_buf_len;
    int pkg_size = 1024;
    int pkg_head_size = sizeof(Package_Buf);

    static unsigned short current_slot_pos = 0;
    int sequence_index = 0;

    while(reserve)
    {
        pkg_size = reserve >= 1024 ? 1024 : reserve;
        reserve -= pkg_size;

        p_fanlu_buf -= pkg_head_size;
        p_package_buf = (Package_Buf *)p_fanlu_buf;

        if(pkg_size != 1024)
            p_package_buf->slot_index = 0x8000 | current_slot_pos;
        else
            p_package_buf->slot_index = current_slot_pos;
        p_package_buf->sequence_index = sequence_index++;

        Ring_Buf_Handler::instance().save_vsyn_pkg(current_slot_pos
                                                   , p_package_buf->sequence_index, pkg_size,p_package_buf->buf);
        ++current_slot_pos;
        current_slot_pos = (current_slot_pos) % VSYN_QUEUE_SIZE;

        {
#ifdef TEST_LOST
            if(current_slot_pos % 2 == 0)
            {
                // fprintf(stderr,"current lost pkg slot index %d \n", current_slot_pos);
            }
            else
#endif
            {
                //加密
                if(1)
                {
                    static int &xproc = MpIni_Handler::instance().xproc;
			//printf("xproc %d-----------------\n", xproc);
                    if(xproc == 1)
                        vproc(p_fanlu_buf, ( unsigned int )(pkg_head_size + pkg_size));
                }
                send_func(p_fanlu_buf,( unsigned int )(pkg_head_size + pkg_size));
            }
        }
        p_fanlu_buf += (pkg_head_size + pkg_size);
    }
    return 0;
}

#endif
