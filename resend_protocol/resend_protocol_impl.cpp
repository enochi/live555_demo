#include "resend_protocol_impl.h"
#include "resend_protocol_recv_handler.h"
#include"resend_protocol_send_handler.h"
#include "socket_func.h"
#include"nalu_file_handler.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

//#include "udpfromto.h"



/*
void resend_pro_socket_send_close()
{


}

void resend_pro_socket_recv_close()
{

}
*/
struct timeval tv_start;

class My_Nalu_Writer
{
public:
    int operator () (unsigned char *nalu, unsigned int len)
    {
        return writen(fd, nalu, len);
    }

    int fd;
};


class My_Send
{
public:
    int operator ()(unsigned char *buf, unsigned int len)
    {
        static int i = 0;

        if(i++ % 3 == 1)
                return 0;
        int rc;


        rc = sendto(send_fd, buf, len, 0, (const sockaddr*)&dst_addr, dst_addr_len);


        usleep(10000);
        return rc;
    }

    int send_fd;
    struct sockaddr_in dst_addr;
    socklen_t dst_addr_len;
};


void resend_pro_send_nalu(char *file, char* local_ip, char* remote_ip, unsigned short port, unsigned short qos_port, bool multicast, bool lost)
{
    int fd = open(file, O_RDWR);
    if(fd == -1)
    {
            perror("nalu_send_test");
            return;
    }

    SocketFunc socket_func;

    int send_fd = socket_func.init_send_sock_udp();
    int qos_fd	= socket_func.init_recv_sock_udp(inet_addr(local_ip), qos_port, multicast);

//    if ( multicast )
//    {
//        udpfromto_init(send_fd);
//    }

    My_Send my_send;

    my_send.send_fd = send_fd;

    struct sockaddr_in &dst_addr	= my_send.dst_addr;
    socklen_t dst_addr_len 			= sizeof(struct sockaddr_in);

    memset(&dst_addr, 0, dst_addr_len);

    dst_addr.sin_family 		= AF_INET;

    if ( multicast )
    {
        //组播情况，目标是组播地址
        dst_addr.sin_addr.s_addr 	= ( inet_addr (local_ip) );
    }
    else
    {
        dst_addr.sin_addr.s_addr 	= ( inet_addr (remote_ip) );
    }

    dst_addr.sin_port           = htons(port);
    my_send.dst_addr_len		= dst_addr_len;


    typedef Resend_Protocol_Send_Handler<My_Send> SEND_HANDLER;
    SEND_HANDLER send_handler;

    send_handler.init(&my_send, send_fd, qos_fd, inet_addr (remote_ip));

    Nalu_File_Read_Hander<SEND_HANDLER> nalu_file_reader(&send_handler);



    gettimeofday(&tv_start, NULL);

    nalu_file_reader(fd);

    close(fd);

}



void resend_pro_receive_nalu(char *file, char* local_ip, char* remote_ip, unsigned short port, unsigned short qos_port, bool multicast)
{
    int fd = open(file, O_CREAT|O_RDWR|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if(fd == -1)
    {
            perror("create file failed");
            return;
    }

    My_Nalu_Writer writer;
    writer.fd = fd;

    SocketFunc socket_func;

    int recv_fd = socket_func.init_recv_sock_udp(ntohl ( inet_addr (local_ip) ), port, multicast);
    Resend_Protocol_Recv_Handler<My_Nalu_Writer> recv_handler;

    recv_handler.init(&writer, recv_fd, recv_fd, inet_addr (remote_ip), qos_port, 256);

    recv_handler();
}


