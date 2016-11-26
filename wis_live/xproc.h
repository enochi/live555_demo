#ifndef XPROC_H
#define XPROC_H


/** fxxk encrypt */

inline void __x_p_r_o_c__( void * key, int klen, void * buf, int len, int off ) {
  unsigned char * p, * r;

  if ( !buf || len <= off )
    return;

  r = (unsigned char *) key;
  p = (unsigned char *) ((unsigned char*)buf + off);
  len -= off;
  if ( klen < len )
    len = klen;

  while ( len ) {
    *p ^= *r;
    p++, r++, len--;
  }
}

inline void optimize( void * buf, int len ) {
  void * pktdata = (unsigned char*)buf + 4;
  int pktlen = len - 4;
  unsigned char * ptr = (unsigned char *) buf;
  unsigned short A,B,K[4],loc;
  unsigned char KK[16];
  unsigned int V;

  if ( pktlen <= 0 )
    return;

  A = (ptr[0] << 8) | ptr[1];
  B = (~ptr[2] << 8) | ptr[2];
  K[0] = A * B; K[1] = A + B; K[2] = A ^ B; K[3] = A - B;

  ((unsigned short *)KK)[0] = K[0];
  ((unsigned short *)KK)[1] = K[1];
  ((unsigned short *)KK)[2] = K[2];
  ((unsigned short *)KK)[3] = K[3];
  ((unsigned short *)KK)[4] = K[2];
  ((unsigned short *)KK)[5] = K[0];
  ((unsigned short *)KK)[6] = K[1];
  ((unsigned short *)KK)[7] = K[3];

  V = ( (unsigned int) K[2] << 16 ) | K[0];

  loc = V % 197;
  __x_p_r_o_c__( KK, 16, pktdata, pktlen, loc );

  loc = V % (K[3] | 0x43);
  __x_p_r_o_c__( KK, 16, pktdata, pktlen, loc );
}

inline void deblock( void * buf, int len ) {
  void * pktdata = (unsigned char*)buf + 4;
  int pktlen = len - 4;
  unsigned char K[] = "\xC1\xBD\xB8\xF6\xBB\xC6\xF0\xBF"
                      "\xC3\xF9\xB4\xE4\xC1\xF8\xA3\xAC"
                      "\xD2\xBB\xD0\xD0\xB0\xD7\xF0\xD8"
                      "\xC9\xCF\xCE\xF7\xCC\xEC\xA1\xA3";
  if ( pktlen <= 0 )
    return;
  __x_p_r_o_c__( K, 32, pktdata, pktlen, 5 );
}

inline void vproc( void * buf, int len ) {
  optimize( buf, len );
  deblock( buf, len );
}

#endif // XPROC_H
