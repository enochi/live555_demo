#ifndef PKG_BUF_H
#define PKG_BUF_H
#include <functional>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <list>
#include "resend_protocol/resend_protocol.h"

#define QUEUE_SIZE 8192

int distance (unsigned int pos1,unsigned int pos2 );
int pre_index( int i);
int current_index( int i);
int next_index( int i);

typedef struct _Tbufpkg{
    _Tbufpkg(){ valid = 0;}
    Package pkg;
    int valid;
}Tbufpkg;

typedef struct _TQosRecored
{
    int request_count;//请求重传的次数
    int countdown;//send resend request every 5 pkg
    _TQosRecored():request_count(0),countdown(5){}
}TQosRecored;


typedef struct _Tbuf
{
    std::string _name;
    std::vector<Tbufpkg> vpkg;
    std::list<int> heads;
    int head;
    int tail;
    int current;
    int prev;
    int before_head_flow;//头之前连续的异常数据
    int middle_flow;//中间连续的异常数据
    int window_size;
    T_nalu_count nalu_count;
    std::vector<char> nalu_buf;
    std::unordered_map<unsigned int,TQosRecored> qos_records;//重传请求记录
    _Tbuf():head(-1),tail(-1),current(-1),prev(-1),before_head_flow(0),middle_flow(0),window_size(256)
    {
        nalu_buf.resize(1024*1024*2);
        nalu_count.reset();
    }
    void reset();
    //返回h移动的距离，
    int move_ht_needed();
    int scan_pkg();
    int get_current_nalu(char *& nalu_buf);
    void qos_records_countdown();
    int current_nalu_pkg_count(int &current_naul_recv);
    int handle_lost();
    int handle_normal_pkg();
    int handle_resend_pkg(Tbufpkg &pkg);
    int move_to_next_head();
    void add_qos_request(unsigned int slot);
    int get_current_valid_pkg();
    void set_name(const std::string & name){_name = name;}
    int invalid_all_package()
    {
        if(head == -1)
            return 0;
        for( int i=head;i<=tail;i++)
        {
            reset_packet(i);
        }
        return 0;
    }
    int reset_packet(int slot)
    {
        Tbufpkg &pkg = vpkg[slot];
        if(pkg.valid !=0)
        {
            pkg.valid = 0;
            --nalu_count._valid_count;
        }
        return 0;
    }
}Tbuf;

#endif // PKG_BUF_H
