#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
#include <pthread.h>
#include <execinfo.h>
#include <unistd.h>

extern void signal_routine(int signo);

void signal_clear()
{
    struct sigaction act;

    act.sa_handler = SIG_IGN;

    //忽略以下信号
    sigaction ( SIGUSR1, &act, NULL );
    sigaction ( SIGUSR2, &act, NULL );
    sigaction ( SIGINT,  &act, NULL );
    sigaction ( SIGSEGV, &act, NULL );
}

void signal_init()
{
    struct sigaction act;
    act.sa_handler = signal_routine;
    act.sa_flags   = 0;
    sigemptyset ( &act.sa_mask );
    sigaction ( SIGUSR1, &act, NULL );
    sigaction ( SIGUSR2, &act, NULL );
    sigaction ( SIGBUS, &act, NULL );
    sigaction ( SIGSEGV, &act, NULL );
    sigaction ( SIGINT, &act, NULL );
    //忽略以下信号
    act.sa_handler = SIG_IGN;
    sigaction ( SIGTERM, &act, NULL );
    sigaction ( SIGHUP,  &act, NULL );
    sigaction ( SIGQUIT, &act, NULL );
    sigaction ( SIGABRT, &act, NULL );
}



#endif // SIGNAL_HANDLER_H

