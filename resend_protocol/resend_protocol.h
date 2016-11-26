#ifndef RESEND_PROTOCOL_H
#define RESEND_PROTOCOL_H

#include <string.h>

#define SLOT_COUNTS			8192
#define RESERVE_PKG_SIZE	128
#define PKG_SIZE			1024

#pragma pack(1)

typedef struct
{
    char magic[13];
    int type;//0正常包，1：重传包,2:重传请求
    int len;
    char uuid_from[37];//不管是数据包还是重传请求，这里的from,to都是转发表里的from,to
    char uuid_to[37];
    char data[0];
}TPackageHead;
class ISlice_Buf
{

public:
	char	slice_tag;  	// 1 =  I slice || 0 = P slice
	int		slice_len;
	int		crc32;
	int		src_addr;
	int		time_stamp;
    //short dsp_timestamp_sec;
    //short dsp_timestamp_100usec;
	char	buf[0];
};

class PSlice_Buf
{

public:
	char	slice_tag;  	// 1 =  I slice || 0 = P slice
	int		slice_len;
	int		crc32;
	int		time_stamp;
    //short dsp_timestamp_sec;
    //short dsp_timestamp_100usec;
	char	buf[0];
};

class Package_Buf
{

public:

	//char 	dev_type:4;
	//char	reserv:2;
	//char	qos_type:2;
	
	unsigned short	slot_index;
	unsigned short	sequence_index;
	char	buf[0];
};

class Resend_Package_Buf
{

public:

	//char 	dev_type:4;
	//char	reserve:2;
	//char	qos_type:2;
	
	char	resend_tag[4];
	unsigned short	slot_index;
	unsigned short	sequence_index;
	char	buf[0];
};

class Resend_Package_Audio_Buf
{
public:

	char	resend_tag[4];
	unsigned short	pkg_size;
	unsigned short	slot_index;
	unsigned long   timestamp;
	char	buf[0];
};

class Package
{
public:
	unsigned char* body()
	{
		return is_resend_pkg() ? b.pkg_buf + sizeof ( Resend_Package_Buf ) : b.pkg_buf + sizeof ( Package_Buf );
	}
	
	unsigned char* buf()
	{
		return b.pkg_buf;
	}
	
	int buf_size()
	{
		return sizeof ( b.pkg_buf );
	}
	
	int buf_len()
	{
		return len;
	}
	
	void buf_len(int _len)
	{
		len = _len > buf_size() ? buf_size() : _len;
	}
	
	unsigned short slot_index()
	{
		return is_resend_pkg() ?  b.resend_pkg.slot_index & 0x7FFF : b.pkg.slot_index & 0x7FFF;
	}
	
	unsigned short sequence_index()
	{
		return is_resend_pkg() ? b.resend_pkg.sequence_index : b.pkg.sequence_index;
	}
	
	bool is_resend_pkg()
	{
		unsigned char resend_tag[] = {0xff, 0xff, 0xff, 0xff};
		if ( memcmp ( b.pkg_buf, resend_tag, 4 ) == 0 )
			return true;
		else
			return false;
		//return (b.pkg_buf[0] == 0xFF) && (b.pkg_buf[1] == 0xFF) && (b.pkg_buf[2] == 0xFF) && (b.pkg_buf[3] == 0xFF);
	}

    int pkg_type()
    {
        return (sequence_index() ? -1 : *body() );
        //return (sequence_index() ? *body() : -1);
    }
	
private:
	union Buf
	{
		Package_Buf         pkg;
		Resend_Package_Buf  resend_pkg;
		unsigned char       pkg_buf[PKG_SIZE + sizeof (Resend_Package_Buf) ];
	};
	
	int				len;
	unsigned char	reserved_head[128];
	Buf				b;
};

inline bool valid_slice ( ISlice_Buf *b )
{
	return true;
}

inline bool valid_slice ( PSlice_Buf *b )
{
	return true;
}
typedef struct _T_nalu_count
{
    _T_nalu_count(){reset();}
    void reset()
    {
        _frames=_total_bytes=_bitrate=_fps=0;
        _not_recoverd = _recoverd=_valid_count=_lost_count=0;
    }
    int _frames;
    int _total_bytes;
    int _bitrate;
     int _fps;
     int _not_recoverd;
     int _recoverd;
     int _valid_count;
     int  _lost_count;
}T_nalu_count;
#pragma pack()

namespace resend_protocol
{
extern unsigned char init_bytes[8];
};
#endif // RESEND_PROTOCOL_H
