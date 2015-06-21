/*
 *  qDsp.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */

#ifndef LSTDSP_VER
#ifndef QAUDIO_VERSION
#define LSTDSP_VER "unknown"
#else
#define LSTDSP_VER QAUDIO_VERSION
#endif
#endif

#include <stdio.h>
#include <saf7741.h>
#include <qDsp.h>

struct _qdsp_dev{
    bool open;
    const qdsp_device_ops* ops;
};

static struct _qdsp_dev qdsp;


const char* qdsp_version()
{
    return LSTDSP_VER;
}


int qdsp_open(qdsp_device dev)
{
    switch(dev)
    {
        case QDSP_DEV_SAF7741: qdsp.ops = saf7741_deviceGetOps();   break;
        default:                                                    break;
    }

    if(qdsp.ops == NULL || qdsp.ops->open == NULL)
        return -1;

    if(qdsp_audio_init(dev) != 0)
    {
        printf("DSP : Init dsp audio error\n");
        return -1;
    }
    
    if(qdsp_radio_init(dev) != 0)
    {
        printf("DSP : Init dsp radio error\n");
        return -1;
    }

    if(qdsp.ops->open() < 0)
    {
        printf("DSP : Open device error\n");
        return -1;
    }

    qdsp.open = true;
    return 0;
}

int qdsp_reset()
{
    if(!qdsp.ops)
        return -1;
    
    if(qdsp.ops->startup && qdsp.ops->startup() < 0)
    {
        printf("DSP : Start up Fail!!!\n");
        return -1;
    }
    
    printf("DSP : Start up OK\n");
    return 0;
}


void qdsp_close()
{
    qdsp_audio_deinit();
    qdsp_radio_deinit();
    
    if(!qdsp.ops->close)
        return;

    qdsp.ops->close();
    qdsp.open = false;
}

bool qdsp_is_open(void)
{
    return qdsp.open;
}






