#ifndef DECODETHREAD_H
#define DECODETHREAD_H
extern "C" {
#include "interf_dec.h"
#include "interf_rom.h"
#include "rom_dec.h"
#include "sp_dec.h"
#include "typedef.h"

#include "dec.h"
#include "dec_acelp.h"
#include "dec_dtx.h"
#include "dec_gain.h"
#include "dec_if.h"
#include "dec_lpc.h"
#include "dec_main.h"
#include "dec_util.h"
#include "if_rom.h"
#include "typedef.h"
}


class decodeThread
{
public:
    decodeThread();
};

#endif // DECODETHREAD_H
