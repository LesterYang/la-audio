#ifndef _QDSP_AUDIO_H
#define _QDSP_AUDIO_H

#include "qDspDev.h"

typedef enum 
{
    QDSP_TT_UNKNOWN = 0x0,
    QDSP_TT_TREB    = 0x1,    
    QDSP_TT_MID     = 0x1 << 1,
    QDSP_TT_BASS    = 0x1 << 2              
}qdsp_audio_tone_type;

typedef enum 
{
    QDSP_JAZZ,       
    QDSP_CLASSIC,
    QDSP_VOCAL,
    QDSP_ROCK 
}qdsp_audio_effect;

typedef enum
{
    QDSP_ENV_UNKNOWN,
    QDSP_ENV_STEREO,
    QDSP_ENV_CENTERPOINT,
}qdsp_audio_env_effect;

typedef enum
{
  QDSP_BEEP_MUTE      = 0x00, 
  QDSP_BEEP_109_KS_01 = 0x01,
  QDSP_BEEP_109_KS_02 = 0x02,
  QDSP_BEEP_109_KS_03 = 0x03,   
  QDSP_BEEP_109_KS_04 = 0x04,
  QDSP_BEEP_109_KS_05 = 0x05,
  QDSP_BEEP_109_KS_06 = 0x06,
  QDSP_BEEP_109_KS_07 = 0x07,
  QDSP_BEEP_109_KS_08 = 0x08,
  QDSP_BEEP_0TONE     = 0x10, 
  QDSP_BEEP_1TONE     = 0x11,   
  QDSP_BEEP_2TONE     = 0x12,      
  QDSP_BEEP_3TONE     = 0x13,           
  QDSP_BEEP_4TONE     = 0x14,   
  QDSP_BEEP_5TONE     = 0x15,           
  QDSP_BEEP_6TONE     = 0x16,             
  QDSP_BEEP_7TONE     = 0x17,          
  QDSP_BEEP_8TONE     = 0x18,      
  QDSP_BEEP_9TONE     = 0x19,   
  QDSP_BEEP_10TONE    = 0x20, 
  QDSP_BEEP_11TONE    = 0x21
}qdsp_audio_beep_type;



int qdsp_audio_init(qdsp_device dev);
void qdsp_audio_deinit(void);

int qdsp_audio_switch_source(qdsp_audio_channel ch);
int qdsp_audio_set_vol(int percent);
int qdsp_audio_mute(void);
int qdsp_audio_unmute(void);

int qdsp_audio_set_balance(short val);
int qdsp_audio_set_fade(short val);
int qdsp_audio_set_bass(short val);
int qdsp_audio_set_middle(short val);
int qdsp_audio_set_treble(short val);
#endif /* _QDSP_AUDIO_H */

