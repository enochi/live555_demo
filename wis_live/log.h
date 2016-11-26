#ifndef COMMON_LOG_H
#define COMMON_LOG_H
#include <ace/Log_Msg.h>
#define DEBUG_PREFIX       ACE_TEXT ("[%T]%I")
#define INFO_PREFIX        ACE_TEXT ("INFO%I")
#define NOTICE_PREFIX      ACE_TEXT ("NOTICE%I")
#define WARNING_PREFIX     ACE_TEXT ("WARNING%I")
#define ERROR_PREFIX       ACE_TEXT ("ERROR%I")
#define CRITICAL_PREFIX    ACE_TEXT ("CRITICAL%I")
#define ALERT_PREFIX       ACE_TEXT ("ALERT%I")
#define EMERGENCY_PREFIX   ACE_TEXT ("EMERGENCY%I")

#define MY_DEBUG(FMT,...)     \
        ACE_DEBUG(( LM_DEBUG,  \
                    DEBUG_PREFIX FMT  \
        ,##__VA_ARGS__))

#endif // SIGNAL_HANDLER_H
