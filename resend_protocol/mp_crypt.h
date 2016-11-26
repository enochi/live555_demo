#ifndef MP_CRYPT_H
#define MP_CRYPT_H

#include <cstdlib>

#include <arpa/inet.h>

#include <unordered_map>

using namespace std;

//#include "destination_key.h"
#include "resend_protocol.h"
#include "mp_ini.h"
#include "object/device.h"


enum MP_EnCrypt_Type
{
    MP_EnCrypt_Type_Video,
    MP_EnCrypt_Type_Audio
};

class Mp_EnCrypt
{
public:
    void clear()
    {
        destination_ip_crypt_key_table.clear();
    }

    int operator () (char *send_buf, int send_len, char *crypt_buf, int*crypt_len, Device const& dest, MP_EnCrypt_Type type)
    {
//#ifdef USE_ARQ_CRYPT
        if(MpIni_Handler::instance().use_fec == 0)
            return mp_arq_encrypt((unsigned char *)send_buf, send_len, (unsigned char *)crypt_buf, crypt_len, dest, type);
//#else
        else
            return mp_encrypt((unsigned char *)send_buf, send_len, (unsigned char *)crypt_buf, crypt_len, dest, type);
//#endif
    }


private:
    std::unordered_map<int, int> destination_ip_crypt_key_table;

    bool need_encrypt(Device const& dest)
    {
        int m = dest.to_ip & 0xff;
        return (!(m < 224 || m > 239));
    }

    int generate_key(Device const& dest)
    {
        int key = 0;

        char str[128];
        //sprintf(str, "%d%d", ntohs(dest.port), (dest.to_ip & 0xff000000) >> 24 );
        key = strtoul(str, 0, 16);
        destination_ip_crypt_key_table[dest.to_ip] = key;

        return key;
    }

    int mp_encrypt(unsigned char *send_buf, int send_len, unsigned char *crypt_buf, int *crypt_len, Device const& dest, MP_EnCrypt_Type type)
    {
        if (*crypt_len <  send_len) return -1;

        int enclen = 0;
        if (type == MP_EnCrypt_Type_Video)
            enclen = 128;
        else if (type == MP_EnCrypt_Type_Audio)
            enclen = 160;
        else
            return -1;

        if (need_encrypt(dest))
        {
            int key = 0;

            if (!destination_ip_crypt_key_table.empty())
            {
                typedef std::unordered_map<int, int>::iterator iterator;
                iterator iter = destination_ip_crypt_key_table.find(dest.to_ip);
                if (iter == destination_ip_crypt_key_table.end())
                {
                    key = generate_key(dest);
                    destination_ip_crypt_key_table[dest.to_ip] = key;
                }
                else
                {
                    key = destination_ip_crypt_key_table[dest.to_ip];
                }
            }
            else
            {
                key = generate_key(dest);
                destination_ip_crypt_key_table[dest.to_ip] = key;
            }
            //char mp_enc_buf[128 + 1500];
            //memcpy(mp_enc_buf, send_buf, send_len);

            memcpy( crypt_buf, send_buf, send_len );
            unsigned char *mp_enc_buf = crypt_buf;
            *crypt_len = send_len;

            for (int i = 0; i < send_len && i < enclen; i += 4)
                *(int*)(mp_enc_buf + i) = *(int*)(mp_enc_buf + i) ^ key;

            return 1;
        }

        *crypt_len = 0;
        return 0;
    }

    int mp_arq_encrypt(unsigned char *send_buf, int send_len, unsigned char *crypt_buf, int *crypt_len, Device const& dest, MP_EnCrypt_Type type)
    {
        if (*crypt_len <  send_len) return -1;

        int enclen = 128;

        if (need_encrypt(dest))
        {
            int key = 0;

            if (!destination_ip_crypt_key_table.empty())
            {
                typedef std::unordered_map<int, int>::iterator iterator;
                iterator iter = destination_ip_crypt_key_table.find(dest.to_ip);
                if (iter == destination_ip_crypt_key_table.end())
                {
                    key = generate_key(dest);
                    destination_ip_crypt_key_table[dest.to_ip] = key;
                }
                else
                {
                    key = destination_ip_crypt_key_table[dest.to_ip];
                }
            }
            else
            {
                key = generate_key(dest);
                destination_ip_crypt_key_table[dest.to_ip] = key;
            }

            unsigned char mp_enc_buf[128 + 1500];
            memcpy(mp_enc_buf, send_buf, send_len);
            if (type == MP_EnCrypt_Type_Video)
            {
                int nalu_offset = 0;
                unsigned char *p_nalu_buf = send_buf;
                //int seq_index = 0;      //unuse
                Package_Buf * pkg_buf;

                //是否是重传包
                unsigned char resend_tag[] = {0xff,0xff,0xff,0xff};
                if(memcmp(send_buf,resend_tag,4) == 0)
                {
                    nalu_offset = 4;
                    p_nalu_buf = send_buf + nalu_offset;
                }

                pkg_buf = (Package_Buf *)p_nalu_buf;

                if (pkg_buf->sequence_index == 0)
                {
                    //pkg的header
                    nalu_offset += 4;

                    //收到帧流的第一个数据包
                    memcpy( crypt_buf, send_buf, send_len );
                    if (pkg_buf->buf[0] == 1)
                    {
                        nalu_offset += sizeof(ISlice_Buf);
                    }
                    else
                        nalu_offset += sizeof(PSlice_Buf);

                    unsigned char *mp_enc_buf = crypt_buf;
                    mp_enc_buf += nalu_offset;

                    int arq_crypt_len = ((send_len - nalu_offset) >= enclen ?  enclen : (send_len - nalu_offset));

                    for (int i = 0; i <  arq_crypt_len; i += 4)
                        *(int*)(mp_enc_buf + i) = *(int*)(mp_enc_buf + i) ^ key;

                    *crypt_len = send_len;
                    return 1;
                }
                else
                {
                    *crypt_len = 0;
                    return 0;
                }

            }
            else if (type == MP_EnCrypt_Type_Audio)
            {
                int audio_offset = 8;

                //如果是init包，不做加密，直接发送
                if ((send_len == 8 ) && memcmp(send_buf,resend_protocol::init_bytes, 8)  == 0)
                {
                    *crypt_len = 0;
                    return 0;
                }

                //是否是重传包
                unsigned char resend_tag[] = {0xff,0xff,0xff,0xff};
                if(memcmp(send_buf,resend_tag,4) == 0)
                {
                    audio_offset += 4;
                }


                memcpy( crypt_buf, send_buf, send_len );

                unsigned char *mp_enc_buf = crypt_buf;
                mp_enc_buf += audio_offset;

                int arq_crypt_len =
                (send_len - audio_offset) >= enclen ?  enclen : (send_len - audio_offset);

                for (int i = 0; i <  arq_crypt_len; i += 4)
                    *(int*)(mp_enc_buf + i) = *(int*)(mp_enc_buf + i) ^ key;

                *crypt_len = send_len;
                return 1;
            }

        }

        *crypt_len = 0;
        return 0;
    }
};


#endif
