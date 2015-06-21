/*
 *  qDspRadio.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <stdio.h>
#include <saf7741.h>
#include <qDspRadio.h>
#include <qDspDev.h>

extern bool qdsp_is_open(void);
const char* qdsp_radio_band_name(qdsp_radio_band band);


struct _qdsp_radio{
    qdsp_device             dev;
    const qdsp_radio_ops*   ops;
};

static struct _qdsp_radio handler;

int qdsp_radio_init(qdsp_device dev)
{
    switch(dev)
    {
        case QDSP_DEV_SAF7741:
            handler.ops   = saf7741_radioGetOps();
            break;

        default:
            printf("DSP : Init unknown radio device\n");
            break;
    }

    if(!handler.ops)
    {
        printf("DSP : Get radio ops error\n");
        return -1;
    }

    handler.dev = dev;
    
    return 0;
}


void qdsp_radio_deinit()
{
    return;    
}


int qdsp_radio_set_band(qdsp_radio_band band)
{
    int_check_status();
    int_check_func(handler.ops->band);

    if(handler.ops->band(band) < 0)
    {
        printf("DSP : Set radio error!!!\n");
        return -1;
    }
    else
    {
        printf("DSP : Switch radio band to %s\n", qdsp_radio_band_name(band)); 
    }
    
    return 0;
}

int qdsp_radio_set_freq(short freq)
{
    int_check_status();
    int_check_func(handler.ops->freq);
    
    return handler.ops->freq(freq);
}

bool qdsp_radio_is_stereo(void)
{
    int_check_status();
    bool_check_func(handler.ops->band);
    
    return handler.ops->pilot();
}

void qdsp_radio_load_align()
{
    void_check_status();
    void_check_func(handler.ops->align);

    handler.ops->align();
}

const char* qdsp_radio_band_name(qdsp_radio_band band)
{
    switch(band)
    {
        case QDSP_BAND_AM:  return "AM";
        case QDSP_BAND_FM:  return "FM";
        default:            break;
    }

    return "unknown";
}

