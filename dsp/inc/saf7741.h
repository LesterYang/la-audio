#ifndef _SAF7741_H
#define _SAF7741_H

#include <qDspDev.h>

#define SAF7741_DIRT_BACKUP     "/tmp/dsp/"

typedef enum
{
    SAF7741_A_IIS1,
    SAF7741_A_IIS2,
    SAF7741_A_IIS3,
    SAF7741_A_IIS4,
    SAF7741_A_SPDIF1,
    SAF7741_A_SPDIF2,
    SAF7741_A_TUNER1,
    SAF7741_A_TUNER2,
    SAF7741_A_PHONE,
    SAF7741_A_NAVI,
    SAF7741_A_AIN0,
    SAF7741_A_AIN1,
    SAF7741_A_AIN2,
    SAF7741_A_AIN3,
    SAF7741_A_AIN4,

    SAF7741_A_MAX,
    SAF7741_A_UNKNOWN = -1
}saf7741_audio_source;


typedef enum
{
    SAF7741_CH_MODE_STEREO,
    SAF7741_CH_MODE_6CHANNELS,
    SAF7741_CH_MODE_DEDEKIND,
    SAF7741_CH_MODE_STANDARD,

    SAF7741_CH_MODE_MAX,
    SAF7741_CH_MODE_UNKNOWN = -1
}saf7741_channel_mode;

typedef enum
{
    SAF7741_EQ_JAZZ,
    SAF7741_EQ_POP,
    SAF7741_EQ_CLASSIC,
    SAF7741_EQ_ROCK,
    
    SAF7741_EQ_UNKNOWN = -1
}saf7741_eq_pattern;

typedef enum
{
    SAF7741_R_BAND_AM_LW,
    SAF7741_R_BAND_AM_MW_EU,
    SAF7741_R_BAND_AM_MW_USA,
    SAF7741_R_BAND_AM_SW,
    SAF7741_R_BAND_FM,
    SAF7741_R_BAND_FM_OIRT,
    SAF7741_R_BAND_WB,
    SAF7741_R_BAND_ORBCOMM
}saf7741_radio_band_type;


int saf7741_OpenDev(void);
void saf7741_CloseDev(void);
int saf7741_StartUp(void);

const qdsp_device_ops* saf7741_deviceGetOps(void);
const qdsp_audio_ops* saf7741_audioGetOps(void);
const qdsp_radio_ops* saf7741_radioGetOps(void);



#endif /* _SAF7741_H */

