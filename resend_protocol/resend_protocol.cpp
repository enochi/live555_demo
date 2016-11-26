#include <cstring>
using namespace std;

#include "resend_protocol.h"

unsigned char resend_protocol::init_bytes[8] = {0xFF,0xFE,0xFE,0xFE,0xFF,0xFE,0xFE,0xFE};
