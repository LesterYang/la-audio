#ifndef _QDSP_DEV_H
#define _QDSP_DEV_H

#include <stdbool.h>

#ifdef __GNUC__
#define QDSP_LIKELY(x) (__builtin_expect(!!(x),1))
#define QDSP_UNLIKELY(x) (__builtin_expect(!!(x),0))
#else
#define QDSP_LIKELY(x) (x)
#define QDSP_UNLIKELY(x) (x)
#endif

#define  int_check_status() do{ if(!qdsp_is_open()) return -1; }while(0)
#define  void_check_status() do{ if(!qdsp_is_open()) return; }while(0)

#define void_check_func(func)               \
    do{                                     \
        if(QDSP_UNLIKELY((func) == NULL))   \
            return;                         \
    }while(0)

#define int_check_func(func)                \
    do{                                     \
        if(QDSP_UNLIKELY((func) == NULL))   \
            return -1;                      \
    }while(0)
#define bool_check_func(func)                   \
        do{                                     \
            if(QDSP_UNLIKELY((func) == NULL))   \
                return false;                   \
        }while(0)


typedef struct _qdsp_device_ops qdsp_device_ops;
typedef struct _qdsp_radio_ops qdsp_radio_ops;
typedef struct _qdsp_audio_ops qdsp_audio_ops;

typedef enum
{
    QDSP_DEV_NONE,
    QDSP_DEV_SAF7741,

    QDSP_DEV_UNKNOWN
}qdsp_device;


typedef enum
{
    QDSP_CH_USB,
    QDSP_CH_NAVI,
    QDSP_CH_RADIO,
    QDSP_CH_DVD,
    QDSP_CH_DTV,
    QDSP_CH_AUXIN,

    QDSP_CH_MAX,
    QDSP_CH_NULL = -1
}qdsp_audio_channel;


typedef enum
{
    QDSP_BAND_AM,
    QDSP_BAND_FM,
    
    QDSP_BAND_NULL = -1
}qdsp_radio_band;


/*--------------------------------
Function :basic options of DSP
Return    : int
                 succeed :  0 
                 error      :  < 0 
---------------------------------*/
struct _qdsp_device_ops{
    int  (*open)();
    void (*close)();
    int  (*startup)();
};


/*--------------------------------
Function :a set of audio functions
Return    : int
                 succeed :  0 
                 error      :  < 0 
---------------------------------*/
struct _qdsp_audio_ops{
    int  (*mute)(bool mute);
    int  (*volume)(int vol);
    int  (*source)(qdsp_audio_channel ch);

    int  (*balance)(short val);
    int  (*fade)(short val);
    int  (*bass)(short val);
    int  (*middle)(short val);
    int  (*treble)(short val);
};


/*--------------------------------
Function :a set of radio functions
Return    : int
                 succeed :  0 
                 error      :  < 0 
---------------------------------*/
struct _qdsp_radio_ops{
    int (*band)(qdsp_radio_band band);
    int (*freq)(short freq);
    bool (*search)(qdsp_radio_band band, short freq);
    void (*align)(void);
    bool (*pilot)(void);
};
#endif /* _QDSP_DEV_H */

