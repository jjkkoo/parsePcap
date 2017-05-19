#include "decodethread.h"

decodeThread::decodeThread()
{
    void * destate;
    destate = Decoder_Interface_init();
    Decoder_Interface_exit(destate);

    void *st;
    st = D_IF_init();
    D_IF_exit(st);

}
