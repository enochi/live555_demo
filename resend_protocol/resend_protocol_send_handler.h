#ifndef RESEND_PROTOCOL_SEND_HANDLER_H_INCLUDED
#define RESEND_PROTOCOL_SEND_HANDLER_H_INCLUDED

#ifndef LEIZHOU
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "local_addr.h"
#include "crc32.h"
#include "resend_protocol.h"
#include <aio.h>
#include <errno.h>

//#include "udpfromto.h"


inline int max(int a , int b)
{
    if(a>b)
        return a;
    else
        return b;
}

//真正的发送在CB完成, callback所用的发送fd和send_fd应该相同哦
template<typename CB>
class Resend_Protocol_Send_Handler
{
public:
	enum Error_Code
	{
		Success,
		SELECT_ERROR
	};

	Resend_Protocol_Send_Handler();

	//fd用来select
    int init ( CB *_cb, int send_fd,  int qos_fd, unsigned long _dst_addr_ip = 0, int qos_queue_size = 1024 );//, bool _multicast = false
	int operator () ( unsigned char *nalu, unsigned int nalu_len, bool f_slice );

private:
	void encap_resend_slice_header ( bool islice, unsigned char * & p_nalu , unsigned int & nalu_len );
	void qos_handler();

	CB				*cb;
	int				send_fd;
	int 			qos_fd;
	short			current_slot;
	int				qos_queue_size;

    //bool            multicast;//是不是组播
    unsigned long   dst_addr_ip;//需要过滤的ip

	Package			*qos_queue;
};

template<typename CB>
Resend_Protocol_Send_Handler<CB>::Resend_Protocol_Send_Handler() : cb ( NULL ), send_fd ( -1 ), qos_fd ( -1 ), current_slot ( 0 ), qos_queue ( NULL )
{}

template<typename CB>
int Resend_Protocol_Send_Handler<CB>::init ( CB *_cb, int _send_fd,  int _qos_fd, unsigned long _dst_addr_ip, int _qos_queue_size )
{
	cb = _cb;
	send_fd	= _send_fd;
	qos_fd	= _qos_fd;
	qos_queue_size = _qos_queue_size;

    //multicast = _multicast;
    dst_addr_ip = _dst_addr_ip;

	qos_queue = new Package [qos_queue_size];

	{
		unsigned char resend_tag[] = {0xff, 0xff, 0xff, 0xff};


		for ( int i = 0; i < qos_queue_size; ++i )
		{
			Package &pkg = qos_queue[i];

			unsigned char *buf = ( unsigned char * ) pkg.buf();

			buf -= sizeof ( resend_tag );

			memcpy ( buf, resend_tag, sizeof ( resend_tag ) );
		}

	}

	return 0;
}

template<typename CB>
void Resend_Protocol_Send_Handler<CB>::encap_resend_slice_header ( bool islice, unsigned char * & p_nalu , unsigned int & nalu_len )
{
	int header_size = 0;
	int crc32_v = 0xffffffff;
	struct timeval tv;
	memset ( &tv, 0, sizeof ( tv ) );

	if ( islice == 1 )
	{
		ISlice_Buf *p_islice_header;

		header_size = sizeof ( ISlice_Buf );
        //whg
        //crc32_v = mycrc32 ( crc32_v, p_nalu, nalu_len );

		p_nalu -= header_size;
		nalu_len += header_size;

		p_islice_header = ( ISlice_Buf * ) p_nalu;
		p_islice_header->slice_tag = 1;
		p_islice_header->slice_len = nalu_len;
		p_islice_header->crc32 = crc32_v;
		p_islice_header->src_addr = util::select_localaddr_for_remote ( 0 );

		gettimeofday ( &tv, NULL );
		p_islice_header->time_stamp = ( tv.tv_sec << 16 ) | ( tv.tv_usec & 0xffff );
        //printf("i slice timestamp:%d,nalu len:%d\n",p_islice_header->time_stamp,nalu_len);

	}
	else
	{
		PSlice_Buf *p_pslice_header;

		header_size = sizeof ( PSlice_Buf );
        //whg
        //crc32_v = mycrc32 ( crc32_v, p_nalu, nalu_len );

		p_nalu -= header_size;
		nalu_len += header_size;

		p_pslice_header = ( PSlice_Buf * ) p_nalu;
		p_pslice_header->slice_tag = 0;
		p_pslice_header->slice_len = nalu_len;
		p_pslice_header->crc32 = crc32_v;

		gettimeofday ( &tv, NULL );
		p_pslice_header->time_stamp = ( tv.tv_sec << 16 ) | ( tv.tv_usec & 0xffff );
       // printf("p slice timestamp:%d,nalu len:%d\n",p_pslice_header->time_stamp,nalu_len);

	}
}


template<typename CB>
int Resend_Protocol_Send_Handler<CB>::operator () ( unsigned char *nalu, unsigned int nalu_len, bool f_slice )
{


   //Spend_Timer_Logger time_log("Resend_Protocol_Send_Handler");
	encap_resend_slice_header ( f_slice,  nalu,  nalu_len );

	unsigned char 		*pkg_buf = nalu;
	int                 pkg_size = PKG_SIZE;
	int                 pkg_head_size = sizeof ( Package_Buf );

	int 				sequence_index = 0;
	int                 reserve = nalu_len;

	while ( reserve > 0 )
	{
		pkg_size = reserve >= PKG_SIZE ? PKG_SIZE : reserve;
		reserve -= pkg_size;

		Package &pkg = qos_queue[current_slot % qos_queue_size];

		unsigned char *buf = ( unsigned char* ) pkg.buf();

		memcpy ( buf + pkg_head_size, pkg_buf, pkg_size );

		pkg.buf_len ( pkg_size + pkg_head_size );

		unsigned int len = pkg_head_size + pkg_size;

		Package_Buf *package = ( Package_Buf * ) ( buf );

		if ( pkg_size != PKG_SIZE )
			package->slot_index = 0x8000 | current_slot;
		else
			package->slot_index = current_slot;

		package->sequence_index = sequence_index++;

        {
            fd_set read_fds;
            fd_set write_fds;
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO ( &read_fds );
            FD_ZERO ( &write_fds );

            FD_SET ( send_fd, &write_fds );
            FD_SET ( qos_fd, &read_fds );

            int nfds = max ( send_fd, qos_fd ) + 1;

            int rc = select ( nfds, &read_fds, &write_fds, NULL, &tv );
            //int rc = select ( qos_fd + 1, &read_fds, NULL, NULL, NULL );

            if ( rc == -1 )
            {
                perror ( __PRETTY_FUNCTION__ );
                throw ( SELECT_ERROR );
            }

            if ( FD_ISSET ( send_fd, &write_fds ) )
            {
                //fprintf ( stderr, "@" );
                ( *cb ) ( buf, len );
            }

            if ( FD_ISSET ( qos_fd, &read_fds ) )
            {
                //fprintf ( stderr, "#" );
                qos_handler();
            }

/*
            tv.tv_sec = 0;
            tv.tv_usec = 500;
            FD_ZERO ( &read_fds );
            FD_SET ( qos_fd, &read_fds );
            nfds = qos_fd + 1;
			//防止qos没有处理
            rc = select ( nfds, &read_fds, NULL, NULL, &tv );
            //int rc2 = select ( nfds, &read_fds, NULL, NULL, NULL );

            if ( rc == -1 )
			{
				perror ( __PRETTY_FUNCTION__ );
				throw ( SELECT_ERROR );
			}

			if ( FD_ISSET ( qos_fd, &read_fds ) )
			{
				//fprintf ( stderr, "#" );
                qos_handler();
			}
        }
*/
        ++current_slot;
        current_slot  %= SLOT_COUNTS;
		pkg_buf += pkg_size;
	}

	return 0;
}


template<typename CB>
void Resend_Protocol_Send_Handler<CB>::qos_handler()
{
    struct sockaddr_in  source_addr, to_addr;
	socklen_t           source_addr_len = sizeof ( struct sockaddr_in );
    socklen_t           to_addr_len = sizeof ( struct sockaddr_in );

	memset ( &source_addr, 0, source_addr_len );
    memset ( &to_addr, 0, to_addr_len );
	int                 pkg_size = 0;

	unsigned short		slot = 0;

    //组播情况和正常情况判断 。。
//    if (multicast)
//    {
//        pkg_size = recvfromto( qos_fd, &slot, sizeof ( slot ), 0, ( struct sockaddr* ) & source_addr, &source_addr_len, ( struct sockaddr* ) & to_addr, &to_addr_len );
//    }
//    else
    {
        pkg_size = recvfrom ( qos_fd, &slot, sizeof ( slot ), 0, ( struct sockaddr* ) & source_addr, &source_addr_len );

    }

    //ip过滤 。。
    if ( dst_addr_ip != 0 )
    {
        if ( dst_addr_ip != inet_addr( inet_ntoa( source_addr.sin_addr) ) )
        {
            //---------------需不需要继续等着接收呢 ？？-------------
            fprintf ( stderr, "qos_fd received dst_addr is not th need, recvfrom return \n" );
            return;
        }
    }

	if ( pkg_size != 2 )
	{
		fprintf ( stderr, "qos_fd recv error, recvfrom return %d\n", pkg_size );
		return;
	}

	//unsigned short count = slot >> 14;
	slot = slot & 0x3FFF;

	Package &pkg = qos_queue[slot % qos_queue_size];

	if ( pkg.slot_index() != slot )
	{
		fprintf ( stderr, "qos request slot is %d, but qos_queue[%d mod %d]'s slot is %d\n", slot, slot, qos_queue_size, pkg.slot_index() );
		return;
	}

	unsigned char *buf = ( unsigned char * ) pkg.buf();

	unsigned char resend_tag[] = {0xff, 0xff, 0xff, 0xff};

	buf -= sizeof ( resend_tag );
	//memcpy ( buf, resend_tag, sizeof ( resend_tag ) );

	int len = pkg.buf_len() + sizeof ( resend_tag );

    //组播情况和正常情况判断 。。这儿有没有必要判断啊？？ 。。
    int send_len;
//    if ( multicast )
//    {
//        send_len = sendfromto( send_fd, buf, len, 0, ( struct sockaddr* ) & to_addr, to_addr_len, ( struct sockaddr* ) & source_addr, source_addr_len );
//    }
//    else
    {
        send_len = sendto ( send_fd, buf, len, 0, ( const sockaddr* ) & source_addr, source_addr_len );
    }

	if ( 1 )
	{
		fprintf ( stderr, "qos send slot : %d send len %d send to ip : %s port %d\n", slot, send_len,  inet_ntoa ( source_addr.sin_addr ), ntohs ( source_addr.sin_port ) );
	}
}
#else
#include <sys/types.h>
#include <sys/socket.h>

#include "resend_protocol.h"
#include "../suyun_sdk/spend_timer_logger.h"

typedef void SEND_FUNC(unsigned char *, unsigned int);


/*
只做分片保存，然后调用回调函数处理数据包等发送
*/

class Resend_Protocol_Send_Handler
{
public:
    Resend_Protocol_Send_Handler();
    int operator()(unsigned char *fnalu_buf, int fnalu_buf_len, SEND_FUNC send_func);
    int get_nalu_count(T_nalu_count &nalu_count){ nalu_count = _nalu_count;return 0;}
    T_nalu_count _nalu_count;
    Spend_Timer_Logger _fps_time;
};
#endif
#endif // RESEND_PROTOCOL_SEND_HANDLER_H_INCLUDED




