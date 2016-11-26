#include "pkg_recevicer.h"
#include "Poco/Timestamp.h"
#include "xproc.h"
#include <set>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>
using namespace Poco;
#define MY_DEBUG printf
static int init_recvsock ( unsigned int port )
{
    int nRecvBuf = 8 * 1024 * 1024;
    int videorecvfd = -1;
    struct sockaddr_in  servaddr;
    int rtn = 0;

    videorecvfd = socket ( AF_INET, SOCK_DGRAM, 0 );
    rtn = setsockopt ( videorecvfd, SOL_SOCKET, SO_RCVBUF, ( const char* ) & nRecvBuf, sizeof ( int ) );

    bzero ( &servaddr, sizeof ( servaddr ) );
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl ( INADDR_ANY );
    servaddr.sin_port        = htons( port );

    ::bind ( videorecvfd, ( struct sockaddr * ) &servaddr, sizeof ( servaddr ) );

    return videorecvfd;
}

void  Pkg_Receiver::run()
{
    MY_DEBUG("Pkg_Receiver begin \n");
    _running = true;
    while(_running)
    {
        char recv_buf[1500+128] ={0};
        sockaddr_in source_addr;
        socklen_t source_addr_len = sizeof ( struct sockaddr_in );
        int pkg_size = recvfrom ( _recv_fd, recv_buf, sizeof(recv_buf), 0, ( struct sockaddr* ) & source_addr, &source_addr_len );
        string from = inet_ntoa(source_addr.sin_addr);
        if ( pkg_size < 0)
        {
            perror("Pkg_Receiver:");
            _running = false;
            continue;
        }
        if(memcmp(recv_buf,"ctrlexit",8)==0)
        {
            MY_DEBUG("recv ctrlexit\n");
            break;
        }
        pkg_handler(from,recv_buf,pkg_size);
        for(std::unordered_map<string,Tbuf >::iterator iter=buffers.begin();iter!=buffers.end();++iter)
        {
            Tbuf &tbuf = iter->second;
            const string &from = iter->first;
            std::unordered_map<unsigned int,TQosRecored> &qos_records = tbuf.qos_records;
            std::unordered_map<unsigned int,TQosRecored>::iterator iter_qos=qos_records.begin();
            while(iter_qos != qos_records.end())
            {

                unsigned int slot= iter_qos->first;
                TQosRecored &qos_record = iter_qos->second;
                if(qos_record.request_count<=4)
                {
                    if(qos_record.countdown == 0)
                    {
                        send_qos_request(from,slot);
                        ++qos_record.request_count;
                        qos_record.countdown = 5;
                        MY_DEBUG("qos request to :%s,slot:%d,count:%d\n",from.c_str(),slot,qos_record.request_count);
                    }
                    ++iter_qos;
                }
                else
                {
					++tbuf.nalu_count._not_recoverd;
                    MY_DEBUG("send %s %d qos more than 4,now erase it\n",from.c_str(),slot);
                    iter_qos = qos_records.erase(iter_qos);
                    //tbuf.move_to_next_head();
                }

            }
        }

    }
    MY_DEBUG("Pkg_Receiver end \n");
    if(_end_notify)
        *_end_notify =~0;
}
Pkg_Receiver::Pkg_Receiver():_recv_fd(-1),_qos_send_interval(20000),_xproc(0),_end_notify(NULL)
{

}
int Pkg_Receiver::start(char *end_notify)
{
    _running = true;
    _thread.start(*this);
    _end_notify = end_notify;
}
void Pkg_Receiver::stop()
{
  MY_DEBUG("Pkg_Receiver::stop() inn");
    if(_recv_fd>0)
    {
       sendto(_recv_fd,"ctrlexit",8,0,(sockaddr*)&_servaddr,sizeof(_servaddr));
    }
    _running = false;
    _thread.join();
    MY_DEBUG("Pkg_Receiver::stop() out\n");
}
Pkg_Receiver::~Pkg_Receiver()
{
    MY_DEBUG("~Pkg_Receiver()\n");
}
int Pkg_Receiver::init(int port)
{
    if(_recv_fd>0)
        close((_recv_fd));
    int nSendBuf = 1024 * 1024*10;
    unsigned char	ttl = 64;
    unsigned int	loopback = 0;
    _listen_port = port;

    int nRecvBuf = 8 * 1024 * 1024;
    int videorecvfd = -1;

    int rtn = 0;
    _recv_fd = socket ( AF_INET, SOCK_DGRAM, 0 );
    if(_recv_fd<0)
    {
        perror("fatal error:");
        MY_DEBUG("fatal error,pkg buffer create socket error\n");
        return -1;
    }
    rtn = setsockopt ( videorecvfd, SOL_SOCKET, SO_RCVBUF, ( const char* ) & nRecvBuf, sizeof ( int ) );
    bzero ( &_servaddr, sizeof ( _servaddr ) );
    _servaddr.sin_family      = AF_INET;
    _servaddr.sin_addr.s_addr = htonl ( INADDR_ANY );
    _servaddr.sin_port        = htons( port );
    ::bind ( _recv_fd, ( struct sockaddr * ) &_servaddr, sizeof ( _servaddr ) );

    return 0;
}

int Pkg_Receiver::combine_callback(const string &from)
{

    Tbuf &tbuf =buffers[from];
    tbuf.qos_records_countdown();
    std::vector<Tbufpkg> &vpkg = tbuf.vpkg;
    int nalu_num  = 0;
    if( -1 == tbuf.move_ht_needed())
        return -1;
    int scan_ret = tbuf.scan_pkg();
    while(scan_ret ==0)
    {

        char *p_nalu = NULL;
        int nalu_len = tbuf.get_current_nalu(p_nalu);
        if(nalu_len<=0)
            break;
#if 0
        if(!had_idr)
        {
            static unsigned char idr[] = {0x00, 0x00, 0x00, 0x01, 0x67};
            //had_idr = (memcmp(idr, i->buf, sizeof(idr)) == 0);
            had_idr = get_nal_slice_type( (unsigned char*)(i->buf), (unsigned int)i->slice_len - sizeof(ISlice_Buf) );

            if(had_idr)
            {
                fprintf ( stderr, "had recv idr\n" );
            }
        }

#endif
        ++nalu_num;
        if(_xproc)
        {
            vproc(p_nalu,nalu_len);
        }

        Poco::ScopedLock<Mutex> guard(_mutex);
        std::string all("all");
        if(_callbacks[all] !=0)
        {
            _callbacks[all](p_nalu,nalu_len,from);
        }
        else
        {
            TMapCallback::iterator cb_iter = _callbacks.find(from);
            if(cb_iter != _callbacks.end() )
            {
                FUNC_ON_NALU_RECV &cb = cb_iter->second;
                cb(p_nalu,nalu_len,from);
            }
        }

        scan_ret = tbuf.scan_pkg();

    }
    return nalu_num;
}

int Pkg_Receiver::pkg_handler(const string &from, const  char *buf, int len)
{

    int ret = -1;
    Tmapbuf::iterator pos = buffers.find(from);
    if(pos == buffers.end())
    {
        MY_DEBUG("add new from :%s\n",from.c_str());
        Tbufpkg pkg;
        std::vector<Tbufpkg> vpkg(QUEUE_SIZE,pkg);
        Tbuf tmp;
        tmp.set_name(from);
        tmp.vpkg.swap(vpkg);
        buffers[from] = tmp;
    }
    do
    {
        Tbuf & tbuf = buffers[from];
        std::vector<Tbufpkg> & vpkg = tbuf.vpkg;
        Tbufpkg pkg;
        memcpy(pkg.pkg.buf(),buf,len);
        int slot = pkg.pkg.slot_index();
        //似乎板卡发送这种包的时候代表开始重新计数
        if(slot == 32511)
        {
            tbuf.reset();
            break;
        }
        unsigned char init[] = {0xFF, 0xFE, 0xFE, 0xFE, 0xFF, 0xFE, 0xFE, 0xFE};
        if(len ==sizeof(init) && memcmp(init,pkg.pkg.buf(),sizeof(init))==0)
        {
            fprintf(stderr,"recv init pkg,reinit\n");
            tbuf.reset();
            break;
        }
        try
        {
            vpkg.at(slot);
        }
        catch(...)
        {
            MY_DEBUG("%s pkg hander recv wrong pkg slot:%d\n",from.c_str(),slot);
            break;
        }
        if(vpkg[slot].valid !=0)
        {
            MY_DEBUG("recv dup pkg?:%d\n",slot);
        }
        change_from_addr(&pkg.pkg);
        if(pkg.pkg.is_resend_pkg() )
        {
            pkg.valid = 2;
            pkg.pkg.buf_len(len-sizeof(Resend_Package_Buf));
            vpkg[slot] = pkg;
            if(0 !=tbuf.handle_resend_pkg(pkg) )
                break;
        }
        else
        {
            tbuf.prev = tbuf.current;
            pkg.valid = 1;
            pkg.pkg.buf_len(len-sizeof(Package_Buf));
            vpkg[slot] = pkg;
            tbuf.current = slot;
            if(tbuf.prev!= pre_index(slot) && tbuf.head !=-1)
            {
                if( 0 !=tbuf.handle_lost())
                    break;
            }
            else
            {
                if( 0 !=tbuf.handle_normal_pkg())
                    break;
            }
            int  n = combine_callback(from);
        }
        ret = 0;
        break;
    }while(0);
    return ret;
}



int Pkg_Receiver::send_qos_request(const string &from,int slot_index)
{


    struct sockaddr_in  qos_dest_addr;
    bzero ( &qos_dest_addr, sizeof ( qos_dest_addr ) );
    qos_dest_addr.sin_family      = AF_INET;
    qos_dest_addr.sin_port        = htons(4603);

    int len = -1;

    qos_dest_addr.sin_addr.s_addr = inet_addr(from.c_str());
    len = sendto ( _recv_fd, &slot_index, 2, 0, (const sockaddr*)&qos_dest_addr, sizeof(qos_dest_addr) );


    return 0;
}




int Pkg_Receiver::change_from_addr(Package *pkg)
{
    return 0;
    //    if(pkg->sequence_index() != 0 || pkg->pkg_type() != 1)
    //        return 1;

    //    if(_xproc == 1)
    //        vproc(pkg->body(),pkg->buf_len());
    //    ISlice_Buf   *p_islicebuf = reinterpret_cast<ISlice_Buf *> ( pkg->body() );
    //    p_islicebuf->src_addr = util::select_localaddr_for_remote ( 0 );
    //    //加密
    //    if(xproc == 1)
    //        vproc(pkg->body(),pkg->buf_len());
    //    return 0;

}
int Pkg_Receiver::get_nalu_count(const std::string &from,T_nalu_count &nalu_count)
{
    Tmapbuf::iterator pos = buffers.find(from);
    if(pos != buffers.end())
    {
        Tbuf & tbuf = buffers[from];
        nalu_count = tbuf.nalu_count;
        return 0;
    }
    else
    {
        MY_DEBUG("can't get nalucount %s\n",from.c_str());
        return -1;
    }

}
