/*
 *  qDspAudio.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <saf7741.h>
#include <qDspAudio.h>
#include <qDspDev.h>


#ifdef __GNUC__
#define LIKELY(x)       (__builtin_expect(!!(x),1))
#define UNLIKELY(x)     (__builtin_expect(!!(x),0))
#define PRETTY_FUNC     __PRETTY_FUNCTION__
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define PRETTY_FUNC     ""
#endif

extern bool qdsp_is_open(void);
const char* qdsp_audio_channel_name(qdsp_audio_channel ch);

struct _qdsp_audio{
    qdsp_device             dev;
    const qdsp_audio_ops*   ops;
};

static struct _qdsp_audio handler;


int qdsp_audio_init(qdsp_device dev)
{
    switch(dev)
    {
        case QDSP_DEV_SAF7741:
            handler.ops = saf7741_audioGetOps();
            break;

        default:
            printf("DSP : Init unknown audio device\n");
            break;
    }

    if(!handler.ops)
    {
        printf("DSP : Get audio ops error\n");
        return -1;
    }

    handler.dev = dev;
    
    return 0;
}

void qdsp_audio_deinit()
{
}


int qdsp_audio_switch_source(qdsp_audio_channel ch)
{
    int_check_status();
    int_check_func(handler.ops->source);

    if(handler.ops->source(ch) < 0)
    {
        printf("DSP : Set audio error!!!");
        return -1;
    }
    else
    {
        printf("DSP : Switch audio source to %s\n", qdsp_audio_channel_name(ch));
    }
    
    return 0;
}

int qdsp_audio_mute()
{
    int_check_status();
    int_check_func(handler.ops->mute);

    if(handler.ops->mute(true) == 0)
        printf("DSP : audio mute\n");
    return 0;
}

int qdsp_audio_unmute()
{
    int_check_status();
    int_check_func(handler.ops->mute);
    
    if(handler.ops->mute(false) == 0)
        printf("DSP : audio unmute\n");
    return 0;

}

int qdsp_audio_set_vol(int percent)
{
    int_check_status();
    int_check_func(handler.ops->volume);
    
    if (percent < 0)
        percent = 0;
    else if (percent > 100)
        percent = 100;

    if(handler.ops->volume(percent) < 0)
    {
        return -1;
    }
    printf("DSP : Set volume %d%%\n", percent);
    return 0;
}

int qdsp_audio_set_balance(short val)
{
    int_check_status();
    int_check_func(handler.ops->balance);
    
    return  handler.ops->balance(val);
}

int qdsp_audio_set_fade(short val)
{
    int_check_status();
    int_check_func(handler.ops->fade);
    
    return handler.ops->fade(val);
}

int qdsp_audio_set_bass(short val)
{
    int_check_status();
    int_check_func(handler.ops->bass);
    
    return handler.ops->bass(val);
}

int qdsp_audio_set_middle(short val)
{
    int_check_status();
    int_check_func(handler.ops->middle);
    
    return handler.ops->middle(val);
}

int qdsp_audio_set_treble(short val)
{
    int_check_status();
    int_check_func(handler.ops->treble);
    
    return handler.ops->treble(val);
}




// inner function

const char* qdsp_audio_channel_name(qdsp_audio_channel ch)
{
    switch(ch)
    {
        case QDSP_CH_USB:   return "USB";
        case QDSP_CH_NAVI:  return "Navi";
        case QDSP_CH_RADIO: return "Radio";
        case QDSP_CH_DVD:   return "DVD";
        case QDSP_CH_DTV:   return "DTV";
        case QDSP_CH_AUXIN: return "AuxIn";
        case QDSP_CH_NULL:  return "Null";
        default:            break;
    }

    return "unknown";
}



