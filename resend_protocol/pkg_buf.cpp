#include "pkg_buf.h"
#include "boost/circular_buffer.hpp"
//scan until all pkg are valid or one pkg not valid

#define MY_DEBUG printf
bool is_before(int p1,int p2)
{
    if((p1-p2>0 && p1-p2<QUEUE_SIZE/2) )
        return true;
     if  (p1-p2<0 && p1-p2+QUEUE_SIZE <QUEUE_SIZE/2)
     {
         MY_DEBUG("is before strange p1-p2:%d-%d\n",p1,p2);
         return true;
     }
     return false;

}
int distance ( unsigned int pos1, unsigned int pos2 )
{
    int d = pos1 > pos2 ? pos1 - pos2 : pos2 - pos1;

    int reserve_d = QUEUE_SIZE - d;

    d = d < reserve_d ? d : reserve_d;

    return ++d;
}
int pre_index( int i)
{
    return (i + QUEUE_SIZE - 1) % QUEUE_SIZE;
}
int current_index( int i)
{
    return i % QUEUE_SIZE;
}

int next_index( int i)
{
    return (++i) % QUEUE_SIZE;
}
int next_index( int a,int i)
{
    return (a+i) % QUEUE_SIZE;
}
void  _Tbuf::reset()
{
    MY_DEBUG(" reset  buf from:%s\n",_name.c_str());
    invalid_all_package();
    head = tail = current = prev = -1;
    before_head_flow  = middle_flow =0;
    nalu_count.reset();
    heads.clear();
    return ;
}
void _Tbuf::qos_records_countdown()
{
    std::unordered_map<unsigned int,TQosRecored>::iterator iter = qos_records.begin();
    while(iter != qos_records.end())
    {
        TQosRecored &qos_record = iter->second;
        --qos_record.countdown;
        ++iter;
    }
}
int Tbuf::current_nalu_pkg_count(int &current_naul_recv)
{
    Tbufpkg *pkg_head = &vpkg[head];
    int count = 0;
    int slice_pkg_count = 0;
    do
    {
        if(!pkg_head->valid )
        {
            slice_pkg_count = 0;
            break;
        }
        else if(pkg_head->pkg.sequence_index() != 0)
        {
            MY_DEBUG(" wtf!,pkg head not valid \n");
            slice_pkg_count = -1;
            break;
        }

        unsigned char *body = pkg_head->pkg.body();

        int slice_length = 0;
        bool is_islice = false;
        if ( body[0] == 1 ) //islice
        {
            is_islice = true;
            ISlice_Buf *islice = ( ISlice_Buf * ) body;
            slice_length = islice->slice_len;

        }
        else if ( body[0] == 0 ) //pslice
        {
            PSlice_Buf *pslice = ( PSlice_Buf * ) body;
            slice_length = pslice->slice_len;
        }
        else
        {
            fprintf(stderr, "aaaaaaaaaaaaaaaaaaaaaa\n");
            slice_pkg_count = -1;
            break;
        }
        slice_pkg_count = slice_length % 1024 == 0 ? slice_length / 1024 : slice_length / 1024 + 1;

        for(int i=0,pos=head;i<slice_pkg_count ;++i)
        {
            Tbufpkg *pkg = &vpkg[pos];
            if(pkg->valid != 0)
            {
                ++count;
            }
            else
            {
                break;
            }
            pos = next_index(pos);
        }
    }while(0);
    current_naul_recv = count;
    return slice_pkg_count;
}
int _Tbuf::get_current_nalu(char *& nalu)
{
    char *slice_buf = &nalu_buf[128];
    char *p = slice_buf;
    int recv_num;
    int slice_pkg_count = current_nalu_pkg_count(recv_num);
    int len = 0;
    if(slice_pkg_count<=0 || slice_pkg_count>recv_num)
    {
        MY_DEBUG("get_current_nalu not ok ,slice count:%d,%d-%d\n",slice_pkg_count,head,tail);
        nalu = NULL;
        return -1;
    }
    Package *pkg_head = &vpkg[head].pkg;
    int pos = head;
    bool is_islice = pkg_head->pkg_type() ==1 ?true:false;

    for (int i = 0; i < slice_pkg_count; ++i)
    {
        Package *pkg = &vpkg[pos].pkg;
        memcpy(p, pkg->body(), 1024);
        pos = next_index(pos);
        p += 1024;
    }
    if (is_islice)
    {
        ISlice_Buf *i = (ISlice_Buf*)slice_buf;
        len = i->slice_len - sizeof(ISlice_Buf);
        nalu = i->buf;
    }
    else
    {
        PSlice_Buf *p = (PSlice_Buf*)slice_buf;
        len = p->slice_len - sizeof(PSlice_Buf);
        nalu = p->buf;
    }
    move_to_next_head();
    return len;
}
int _Tbuf::move_to_next_head()
{
    int recv_num;
    int slice_pkg_count = current_nalu_pkg_count(recv_num);
    int d_ht = distance(head,tail);
    if(slice_pkg_count<=0 || d_ht<slice_pkg_count)
    {
        MY_DEBUG("move_to_next_head return slice_pkg_count:%d %d-%d\n",slice_pkg_count,head,tail);
        return -1;
    }
    for (int i = 0,pos=head; i < slice_pkg_count; ++i)
    {
        reset_packet(pos);
        if(qos_records.find(pos) != qos_records.end())
        {
            MY_DEBUG("move to next head ,eraze qos record:%d\n",pos);
            qos_records.erase(pos);
        }
        pos = next_index(pos);

    }
    head = next_index(head,slice_pkg_count);
    //下一个包还未接受到头部，特殊处理一下，以防当恰好丢包为头部的时候，被判为收到了头之前的包
    int next = next_index(head);\
    Tbufpkg &tbufpkg = vpkg[next];
    if(tbufpkg.valid == 0)
    {
        //MY_DEBUG("move to next head ,next head+1 not valid %d-%d\n",head,tail);
        tail = head;
    }
    return 0;
}
int _Tbuf::get_current_valid_pkg()
{
    Tbufpkg *tpkg = NULL;
    int count=0;
    for(int pos=head;pos!=tail;)
    {
        tpkg = &vpkg[pos];
        if(tpkg->valid !=0)
        {
            ++count;
        }
        pos = next_index(pos);
    }
    return count;
}
int _Tbuf::scan_pkg()
{

    int ret = 0;
    int recv_num;
    int slice_pkg_count = current_nalu_pkg_count(recv_num);
    int valid_num = get_current_valid_pkg();
    if(slice_pkg_count<0)//包头无效
    {
        MY_DEBUG("current head not valid\n");
       reset();
       ret = -1;
    }
    else if(slice_pkg_count==0)//包头未收到

    {
        //MY_DEBUG("!!! wait the head resend pkg:%d-%d\n",head,tail);
        ret = 2;
    }
    else if  ( slice_pkg_count > valid_num )//包还没有收齐
    {
        ret = 1;
    }
    else
    {
        int pos = head;
        int i = 0;
        bool next_head_f = false;
        int next_head = next_index(head,slice_pkg_count);
        for (i = 0; i < slice_pkg_count; ++i)
        {
            if (vpkg[pos].valid == 0)
            {
                if(qos_records.end() ==qos_records.find(pos) )
                {
                    MY_DEBUG("invalid pkg can't find in the qos recored:%d\n",pos);
                    next_head_f = true;
                }
                break;
            }
            pos = (pos + 1) % QUEUE_SIZE;
        }
        if( i != slice_pkg_count)//有丢包
        {
            if(next_head_f)//丢包已无法恢复
            {
                MY_DEBUG(" lost pkg %d-%d can't reover,jump next head %d\n",head,tail,next_head);
                move_to_next_head();
            }
            else
            {
                ret = 1;
            }

        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}
int _Tbuf::move_ht_needed()
{

    //超时处理
    int d = distance ( head, tail );
    int next_head =-1;
    int move_count = 0;
    if ( d > window_size ) //超时
    {
        //先移动到一个不超时的位置
        next_head = ( head + d - window_size ) % QUEUE_SIZE;
        int pos = head;
        for ( pos = head; pos != next_head; pos = next_index(pos) )
        {
            ++move_count;
            reset_packet(pos);
        }
        head = next_head;
#if 1
        //再移动到一个I帧开始的位置
        int tail_plus_1 = next_index(tail);

        for ( pos = head; pos != tail_plus_1; pos = next_index(pos))
        {
            ++move_count;
            reset_packet(pos);
            Package &pkg = vpkg[pos].pkg;
            if(vpkg[pos].valid !=0)
            {
                //find I
                if(pkg.pkg_type() == 1) break;
                else if(pkg.pkg_type() == 0) break;
            }
        }

        if ( pos == tail_plus_1 ) //队列已空
        {
            MY_DEBUG("%s overflow ,can't find frame head,reset\n",_name.c_str());
            reset();
            move_count = -1;
        }
        else
        {
           MY_DEBUG("%s overflow window_size ,reset h-t:%d-%d to:%d\n",_name.c_str(),head,tail,pos);
            head = pos;
        }
#endif
    }
    return move_count;
}
int _Tbuf::handle_normal_pkg()
{
    Package &pkg_cur = vpkg[current].pkg;
    if(head == -1)
    {
        if(pkg_cur.pkg_type() != 1 && pkg_cur.pkg_type() != 0)
        {
            MY_DEBUG("discard pkg for not head seq:%d,slot:%d\n",pkg_cur.sequence_index(),pkg_cur.slot_index());
            return -1;
        }
        MY_DEBUG("%s recv first packet :%d\n ",_name.c_str(),current);
        head=tail=current;

    }
    else
    {
//        if(pkg_cur.sequence_index() == 0)
//        {
//            MY_DEBUG("recv next nalu pkg:%d\n",pkg_cur.slot_index());
//            heads.push_back(pkg_cur.slot_index());
//        }
        tail = current;
    }
    ++nalu_count._valid_count;
    return 0;
}
int _Tbuf::handle_lost()
{

    MY_DEBUG("%s lost pkg,cur :%d,h-t:%d-%d,prev:%d\n",_name.c_str(),current,head,tail,prev);

    int ret = -1;
    int d_h = distance ( head, current );
    int d_t = distance ( tail, current );
    int d_ht = distance(head,tail);
    if(current == tail || current == head)
    {
        ret = 0;
    }
    //收到后面的包
    else if(d_h +1== d_t+d_ht  && d_t<window_size)
    {
         tail = current;
        if(d_t<window_size)
        {
            int pos = head;
            for( ;pos != tail;pos = next_index(pos))
            {
                if(vpkg[pos].valid == 0)
                {
                    add_qos_request(pos);
                }
            }
            ret = 0;
        }
        else
        {
            MY_DEBUG("lost pkg overflow windows size\n");
        }
        nalu_count._lost_count+=d_t-1;

    }
    //之前的包
    else if(d_h +d_ht == d_t+1)
    {
        //是头之前的包丢弃掉
        reset_packet(current);
        //收到了一个头之前的包,计算连续包的数目，大于5，我们认为之前的状态已损坏
        if(pre_index(current) == prev)
        {
            ++before_head_flow;
        }
        else
        {
            before_head_flow = 0;
        }
        MY_DEBUG("%s recv data before head  %d-%d-%d,flowcount:%d\n",
                 _name.c_str(),head,tail,current,before_head_flow);
        if(before_head_flow>5)
        {
            reset();
            MY_DEBUG("%s reset %d-%d-%d,flowcount:%d\n",
                     _name.c_str(),head,tail,current,before_head_flow);
        }
    }
    return 0;

}
void _Tbuf::add_qos_request(unsigned int slot)
{

    if(qos_records.find(slot) == qos_records.end() )
    {
        TQosRecored qos_record;
        qos_records[slot] = qos_record;

    }

}
int _Tbuf::handle_resend_pkg(Tbufpkg &pkg)
{
    MY_DEBUG("pkg handler resend pkg from %s: slot:%d\n",_name.c_str(),pkg.pkg.slot_index());
    int ret= -1;
    std::unordered_map<unsigned int,TQosRecored>::iterator pos =qos_records.find(pkg.pkg.slot_index());
    if(  pos!= qos_records.end() )
    {
        qos_records.erase(pkg.pkg.slot_index());
        ret = 0;
    }
    else
    {
        MY_DEBUG("wtf!\n");
    }

    return ret;
}
