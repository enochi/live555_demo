#ifndef RESEND_PROTOCOL_IMPL_H
#define RESEND_PROTOCOL_IMPL_H


#ifdef __cplusplus
extern "C"
{
#endif


void resend_pro_send_nalu(char *file, char* local_ip, char* remote_ip, unsigned short port, unsigned short qos_port, bool multicast = false, bool lost = true);
void resend_pro_receive_nalu(char *file, char* local_ip, char* remote_ip, unsigned short port, unsigned short qos_port, bool multicast = false);
//void resend_pro_socket_send_close();
//void resend_pro_socket_recv_close();

#ifdef __cplusplus
};
#endif

#endif // RESEND_PROTOCOL_IMPL_H
