
#include"resend_protocol_impl.h"
#include "local_addr.h"

int main()
{
    util::get_local_ipaddr();
    resend_pro_receive_nalu("211.nalu","224.0.0.25","168.168.1.195",4602 ,4603,true);
    //resend_pro_send_nalu("210.nalu","224.0.0.25","0",4602,4603,true);

    //正常情况调用方式有-------------------------//
//    resend_pro_receive_nalu("211.nalu", "192.168.1.195", "192.168.1.200", 4602, 4603);//1
//    resend_pro_receive_nalu("211.nalu", "192.168.1.195", "0", 4602, 4603);//2
//    resend_pro_receive_nalu("211.nalu", "0", "192.168.1.200", 4602, 4603);//3

//    resend_pro_send_nalu("210.nalu", "192.168.1.195", "192.168.1.200", 4602, 4603);//1
//    resend_pro_send_nalu("210.nalu", "0", "192.168.1.200", 4602, 4603);//2
    //----------------------------------------//

    //组播情况调用方式有-------------------------//
//    resend_pro_receive_nalu("211.nalu", "192.168.1.195", "192.168.1.200", 4602, 4603, true);//1
//    resend_pro_receive_nalu("211.nalu", "192.168.1.195", "0", 4602, 4603, true);//2
//    resend_pro_receive_nalu("211.nalu", "0", "192.168.1.200", 4602, 4603, true);//3

//    resend_pro_send_nalu("210.nalu", "192.168.1.200", "192.168.1.201", 4602, 4603, true);//1
//    resend_pro_send_nalu("210.nalu", "192.168.1.200", "0", 4602, 4603, true);//2
    //-----------------------------------------//
    return 0;

 }
