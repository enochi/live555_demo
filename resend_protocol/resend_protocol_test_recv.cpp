#include <cstdio>
#include <cstdlib>
using namespace std;

#include "resend_protocol_recv_handler.h"
#include "resend_protocol_send_handler.h"
#include "crc32.h"
#include "local_addr.h"

int recv_fd;

char *dst_ip = NULL;

void send_handler(unsigned char *fec_buf, unsigned int fec_len)
{

    struct        sockaddr_in  remoteaddr;

    bzero(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = inet_addr(dst_ip);
    remoteaddr.sin_port = htons(4602);


    sendto(recv_fd, fec_buf, fec_len, 0, (struct sockaddr*)&remoteaddr, sizeof(struct sockaddr));

}

void encap_resend_slice_header(int islice, unsigned char * & p_nalu , int & nalu_len)
{
    int header_size;
    int crc32_v = 0xffffffff;
    struct timeval tv;

    if(islice == 1)
    {
        ISlice_Buf *p_islice_header;

        header_size = sizeof(ISlice_Buf);
        crc32_v = mycrc32(crc32_v, p_nalu, nalu_len);

        p_nalu -= header_size;
        nalu_len += header_size;

        p_islice_header = (ISlice_Buf *)p_nalu;
        p_islice_header->slice_tag = 1;
        p_islice_header->slice_len = nalu_len;
        p_islice_header->crc32 = crc32_v;
        p_islice_header->src_addr = util::select_localaddr_for_remote(0);

        gettimeofday(&tv,NULL);
        p_islice_header->time_stamp = (tv.tv_sec << 16) | (tv.tv_usec & 0xffff);

    }
    else
    {
        PSlice_Buf *p_pslice_header;

        header_size = sizeof(PSlice_Buf);
        crc32_v = mycrc32(crc32_v, p_nalu, nalu_len);

        p_nalu -= header_size;
        nalu_len += header_size;

        p_pslice_header = (PSlice_Buf *)p_nalu;
        p_pslice_header->slice_tag = 0;
        p_pslice_header->slice_len = nalu_len;
        p_pslice_header->crc32 = crc32_v;

        gettimeofday(&tv,NULL);
        p_pslice_header->time_stamp = (tv.tv_sec << 16) | (tv.tv_usec & 0xffff);

    }
}




int init_sendsock ( unsigned int port )
{
    int nSendBuf = 1024 * 64;

    unsigned char	ttl = 64;
    unsigned int	loopback = 0;

    int sendfd = socket ( AF_INET, SOCK_DGRAM, 0 );
    int rt = setsockopt ( sendfd, SOL_SOCKET, SO_SNDBUF, ( const char* ) &nSendBuf, sizeof ( int ) );
    setsockopt(sendfd,IPPROTO_IP,IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt(sendfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopback, sizeof(loopback));

    return sendfd;
}

int init_recvsock ( unsigned int port )
{
    int nRecvBuf = 1024 * 1024;
    int videorecvfd = -1;
    struct sockaddr_in  servaddr;
    int rtn = 0;

    videorecvfd = socket ( AF_INET, SOCK_DGRAM, 0 );
    rtn = setsockopt ( videorecvfd, SOL_SOCKET, SO_RCVBUF, ( const char* ) & nRecvBuf, sizeof ( int ) );

    bzero ( &servaddr, sizeof ( servaddr ) );
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl ( INADDR_ANY );
    servaddr.sin_port        = ( port );
    //extern int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
    bind ( videorecvfd, ( struct sockaddr * ) &servaddr, sizeof ( servaddr ) );

    return videorecvfd;
}

typedef int CB(unsigned char*, unsigned int);

Resend_Protocol_Send_Handler resend_protocol_send_handler;



int nalu_handler(unsigned char *nalu, unsigned int nalu_len)
{
    //return 0;

    static unsigned char i_frame_tag[] = {0x00,0x00,0x00,0x01,0x67};
    int islice = 0;
    if ( memcmp(nalu,i_frame_tag,5) == 0 )  	islice = 1;

    if(0)
    {
        static int write_fd = -1;

        if(write_fd == -1)
        {
            write_fd = open("test.nalu", O_CREAT|O_TRUNC|O_RDWR);
        }

        write(write_fd, nalu, nalu_len);
    }

    unsigned char *pnalu = nalu;
    //fprintf(stderr, "had recv a nalu slice ---- \t\t(%d)\t\t\t(%d)\n", nalu_len, islice);
    encap_resend_slice_header(islice, (unsigned char * &)nalu, (int&)nalu_len);
    //fprintf(stderr, "slice hdr len %d  nalu_len %d \n", pnalu - nalu, nalu_len);
    resend_protocol_send_handler((unsigned char *)nalu,nalu_len, send_handler);

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        fprintf(stderr, "use as : test_recv recv_port qos_port dst_ip\n");
        return -1;
    }
    unsigned short  recv_port   = atoi(argv[1]);
    unsigned short  qos_port    = atoi(argv[2]);
    dst_ip = argv[3];

    //int
    recv_fd = init_recvsock(htons(recv_port));
    int qos_fd = init_sendsock(0);

    Resend_Protocol_Recv_Handler<CB> &h = *new Resend_Protocol_Recv_Handler<CB>;

    //int init ( CB *_cb, int recv_fd,  int qos_fd, short qos_port = 4603, int window_size = 64 );
    h.init( &nalu_handler, recv_fd,  qos_fd, htons(qos_port), 64 );

    h();
}
