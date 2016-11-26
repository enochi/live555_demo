#ifndef RING_BUF_HANDLER_H_INCLUDED
#define RING_BUF_HANDLER_H_INCLUDED

#include <cstring>
using namespace std;

#define VSYN_QUEUE_SIZE 8192
#define AMIX_QUEUE_SIZE 128
/*
保存所有数据包，提供保存、获取、初始化等接口

*/
#pragma pack(1)

class Ring_Buf_Handler{

#define PKGBUF_MAX_SIZE   1024
	struct pkg_struct_t
	{
		pkg_struct_t() :
				read_counts ( 0 ),
				pkg_size ( 0 ),
				seq_index ( 0 ),
				timestamp ( 0 )
		{
			memset ( pkg_reserve_buf, 0, sizeof ( pkg_reserve_buf ) );
			memset ( pkg_buf, 0, sizeof ( pkg_buf ) );
		}

		unsigned short       read_counts;
		unsigned short       pkg_size;
		unsigned short       seq_index;
		unsigned long        timestamp;
		char        pkg_reserve_buf[128];
		char        pkg_buf[PKGBUF_MAX_SIZE];
	};

public:
// vsyn
	int save_vsyn_pkg ( unsigned short slot_index, unsigned short seq_index
	                    , unsigned short pkg_size, char * pkg_buf );
	int get_vsyn_pkg ( unsigned short &has_get_counts, unsigned short slot_index,
	                   unsigned short &seq_index , unsigned short &pkg_buf_size, char * & pkg_buf );
//amix
	int save_amix_pkg ( unsigned long timestamp, unsigned short slot_index
	                    , unsigned short pkg_size, char * pkg_buf );
	int get_amix_pkg ( unsigned short &has_get_counts, unsigned short slot_index,
	                   unsigned long &timestamp , unsigned short &pkg_buf_size, char * & pkg_buf );

	static Ring_Buf_Handler & instance()
	{
		static Ring_Buf_Handler  ring_buf_handler;
		return ring_buf_handler;
	}

	~Ring_Buf_Handler()
	{
		if ( vsyn_ring_buf_queue != 0 )
		{
			delete [] vsyn_ring_buf_queue;
		}
		if ( amix_ring_buf_queue != 0 )
		{
			delete [] amix_ring_buf_queue;
		}
	}
private:
	Ring_Buf_Handler()
	{
		vsyn_ring_buf_queue = new pkg_struct_t[VSYN_QUEUE_SIZE];
		amix_ring_buf_queue = new pkg_struct_t[AMIX_QUEUE_SIZE];
	}
	pkg_struct_t    * vsyn_ring_buf_queue;
	pkg_struct_t    * amix_ring_buf_queue;
};

#pragma pack(0)

#endif // RING_BUF_HANDLER_H_INCLUDED
