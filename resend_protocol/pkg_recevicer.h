#ifndef PKG_RECEVICER_H
#define PKG_RECEVICER_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "pkg_buf.h"
#include "Poco/Runnable.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

using namespace std;
using namespace Poco;
typedef std::function<int (const char* nalu,int len,const std::string &remote_addr)> FUNC_ON_NALU_RECV;

class Pkg_Receiver:public Poco::Runnable
{
public:

    typedef  std::unordered_map<string,Tbuf >  Tmapbuf;

    typedef  std::unordered_map<string,FUNC_ON_NALU_RECV >  TMapCallback;
    static Pkg_Receiver& instance()
    {
        static Pkg_Receiver instance;
        return instance;
    }
    int init(int port);
    int start(char *end_notify);
    void stop();
    int get_nalu_count(const std::string &from,T_nalu_count &nalu_count);
    int set_callback(const std::string &from,FUNC_ON_NALU_RECV *func)
    {
        Poco::ScopedLock<Mutex> guard(_mutex);
        if(func == NULL)
        {
            _callbacks.erase(from);
        }
        else
        {
            _callbacks[from] = *func;
        }
        return 0;
    }
    int pkg_handler(const string &from,  const char *buf, int len);
    void  run();
private:
    Pkg_Receiver();
    ~Pkg_Receiver();

    int handle_resend_pkg(const string &from,Tbufpkg &pkg);
    //callback and move head tail
    int combine_callback(const string &from);

    int change_from_addr(Package *pkg);


    int send_qos_request(const string &from,int slot_index);

    Tmapbuf buffers;
    TMapCallback _callbacks;
    int _recv_fd;
    int _qos_send_interval;
    int _xproc;
    int _listen_port;
    bool _running;
    Poco::Mutex _mutex;
    Poco::Timestamp _qos_ts;
    Poco::Thread _thread;
    char *_end_notify;
    struct sockaddr_in _servaddr;

};



#endif // PKG_RECEVICER_H
