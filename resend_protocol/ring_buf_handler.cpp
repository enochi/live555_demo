
#include <string.h>
#include <stdio.h>
#include "ring_buf_handler.h"

int Ring_Buf_Handler::save_vsyn_pkg(unsigned short slot_index, unsigned short seq_index
                                            , unsigned short pkg_size, char * pkg_buf)
{
    if(vsyn_ring_buf_queue == NULL)          return -1;
    if(slot_index >= VSYN_QUEUE_SIZE)        return -2;
    if(seq_index >= VSYN_QUEUE_SIZE)         return -3;
    if(pkg_size > 1024)                 return -4;

    pkg_struct_t * p_pkg_struct;
    p_pkg_struct = &vsyn_ring_buf_queue[slot_index];
    p_pkg_struct->read_counts = 0;
    p_pkg_struct->pkg_size = pkg_size;
    p_pkg_struct->seq_index = seq_index;
    memcpy(p_pkg_struct->pkg_buf, pkg_buf, pkg_size);

    return 0;
}

int Ring_Buf_Handler::get_vsyn_pkg(unsigned short &has_get_counts, unsigned short slot_index,
                    unsigned short &seq_index , unsigned short &pkg_buf_size, char * &pkg_buf)
{
    if(vsyn_ring_buf_queue == NULL)          return -1;
    if(slot_index >= VSYN_QUEUE_SIZE)        return -2;

    has_get_counts = ++vsyn_ring_buf_queue[slot_index].read_counts;
    seq_index = vsyn_ring_buf_queue[slot_index].seq_index;
    pkg_buf_size = vsyn_ring_buf_queue[slot_index].pkg_size;
    pkg_buf = vsyn_ring_buf_queue[slot_index].pkg_buf;

    return 0;

}



int Ring_Buf_Handler::save_amix_pkg(unsigned long timestamp, unsigned short slot_index
                                    , unsigned short pkg_size, char * pkg_buf)
{
    if(amix_ring_buf_queue == NULL)     return -1;
    if(slot_index >= AMIX_QUEUE_SIZE)   return -2;
    if(pkg_size > 1024)                 return -4;

    pkg_struct_t * p_pkg_struct;
    p_pkg_struct = &amix_ring_buf_queue[slot_index];
    p_pkg_struct->read_counts = 0;
    p_pkg_struct->pkg_size = pkg_size;
    p_pkg_struct->timestamp = timestamp;
    memcpy(p_pkg_struct->pkg_buf, pkg_buf, pkg_size);

    return 0;
}

int Ring_Buf_Handler::get_amix_pkg(unsigned short &has_get_counts, unsigned short slot_index,
                    unsigned long &timestamp , unsigned short &pkg_buf_size, char * & pkg_buf)
{
    if(amix_ring_buf_queue == NULL)          return -1;
    if(slot_index >= AMIX_QUEUE_SIZE)        return -2;

    has_get_counts = ++amix_ring_buf_queue[slot_index].read_counts;
    timestamp = amix_ring_buf_queue[slot_index].timestamp;
    pkg_buf_size = amix_ring_buf_queue[slot_index].pkg_size;
    pkg_buf = amix_ring_buf_queue[slot_index].pkg_buf;


    return 0;

}
