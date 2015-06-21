/*
 *  saf7741_audio.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include <stdbool.h>
#include <qDspDev.h>

#include "saf7741_audio.h"
#include "saf7741_audio_regs.h"
#include "saf7741_util.h"
#include "saf7741_i2c.h"
#include "saf7741_vol_table.h"

unsigned char gcWeight[SAF7741_A_MAX]={
                        100,        //SAF7741_A_IIS1
                        100,        //SAF7741_A_IIS2
                        100,        //SAF7741_A_IIS3
                        100,        //SAF7741_A_IIS4
                        100,        //SAF7741_A_SPDIF1
                        100,        //SAF7741_A_SPDIF2
                        100,        //SAF7741_A_TUNER1
                        100,        //SAF7741_A_TUNER2
                        100,        //SAF7741_A_PHONE
                        100,        //SAF7741_A_NAVI
                        100,        //SAF7741_A_AIN0
                        100,        //SAF7741_A_AIN1
                        100,        //SAF7741_A_AIN2
                        100,        //SAF7741_A_AIN3
                        100         //SAF7741_A_AIN4       
};

struct saf7741_audio_status{
    bool mute;
    saf7741_audio_source source;
    int percentVol;
};

static struct saf7741_audio_status gsAudioState;

int saf7741_audioSetSourceOnPrimary(saf7741_audio_source source)
{
    int ret = 0;
    bool skip = false;
    bool switch_mute = (gsAudioState.mute == false);
    unsigned char data[3]={0};
   
    switch(source)
    {
        case SAF7741_A_IIS1:    i_to_3uc(data, ADSP_EASYP_SrcSw_I2S1onA);   break;
        case SAF7741_A_IIS2:    i_to_3uc(data, ADSP_EASYP_SrcSw_I2S2onA);   break;
        case SAF7741_A_IIS3:    i_to_3uc(data, ADSP_EASYP_SrcSw_I2S3onA);   break;
        case SAF7741_A_IIS4:    i_to_3uc(data, ADSP_EASYP_SrcSw_I2S4onA);   break;
        case SAF7741_A_SPDIF1:  i_to_3uc(data, ADSP_EASYP_SrcSw_SPDIF1onA); break;
        case SAF7741_A_SPDIF2:  i_to_3uc(data, ADSP_EASYP_SrcSw_SPDIF2onA); break;
        case SAF7741_A_TUNER1:  i_to_3uc(data, ADSP_EASYP_SrcSw_Tuner1onA); break;
        case SAF7741_A_TUNER2:  i_to_3uc(data, ADSP_EASYP_SrcSw_Tuner2onA); break;
        case SAF7741_A_PHONE:   skip = true;                                break;
        case SAF7741_A_NAVI:    skip = true;                                break;
        case SAF7741_A_AIN0:    i_to_3uc(data, ADSP_EASYP_SrcSw_AIN0onA);   break;
        case SAF7741_A_AIN1:    i_to_3uc(data, ADSP_EASYP_SrcSw_AIN1onA);   break;
        case SAF7741_A_AIN2:    i_to_3uc(data, ADSP_EASYP_SrcSw_AIN2onA);   break;
        case SAF7741_A_AIN3:    skip = true;                                break;
        case SAF7741_A_AIN4:    i_to_3uc(data, ADSP_EASYP_SrcSw_AIN4onA);   break;
        default:                skip = true;                                break;
    }

     if(skip)
        return -1;

    if(switch_mute)
        saf7741_audioSetPrimaryMute(1);

    ret = saf7741_i2c_write(ADSP_X_EasyP_Index, data, sizeof(data));

    if(ret == 0)
    {
        gsAudioState.source = source;
        saf7741_audioSetPrimaryVol(gsAudioState.percentVol);
    }

    if(switch_mute)
        saf7741_audioSetPrimaryMute(0);
    
    return ret;
}

int saf7741_audioSetSource(qdsp_audio_channel ch)
{
    saf7741_audio_source source = SAF7741_A_UNKNOWN;

    switch(ch)
    {
        case QDSP_CH_USB:       source = SAF7741_A_IIS3;        break;
        case QDSP_CH_NAVI:      source = SAF7741_A_IIS4;        break;
        case QDSP_CH_RADIO:     source = SAF7741_A_TUNER1;      break;
        case QDSP_CH_DVD:       source = SAF7741_A_IIS1;        break;
        case QDSP_CH_DTV:       source = SAF7741_A_AIN2;        break;
        case QDSP_CH_AUXIN:     source = SAF7741_A_AIN0;        break;
        case QDSP_CH_NULL:      source = SAF7741_A_IIS2;        break;
        default:                source = SAF7741_A_UNKNOWN;     break;

    }

    if(source != SAF7741_A_UNKNOWN)
        saf7741_audioSetSourceOnPrimary(source);
    
    return 0;
}



int saf7741_audioTransVol(int vol)
{  
    return (vol * gcWeight[gsAudioState.source]) / 100;
}

int saf7741_audioSetPrimaryVol(int vol)
{
    int ret = 0;
    int weightVol;
    unsigned char data1[2];
    unsigned char data2[2];
   
    weightVol = saf7741_audioTransVol((vol*61)/100);

    if(weightVol > SAF7741_VOL_MAX) weightVol = SAF7741_VOL_MAX;
    if(weightVol < SAF7741_VOL_MIN) weightVol = SAF7741_VOL_MIN;

    data1[0] = (unsigned char)(vol_table[weightVol] >> 24);
    data1[1] = (unsigned char)(vol_table[weightVol] >> 16);
    data2[0] = (unsigned char)(vol_table[weightVol] >> 8);
    data2[1] = (unsigned char)(vol_table[weightVol]);

    ret = saf7741_i2c_write(ADSP_Y_Vol_Main1P, data1, sizeof(data1));
    ret = saf7741_i2c_write(ADSP_Y_Vol_Main2P, data2, sizeof(data2));

    if(ret == 0)
        gsAudioState.percentVol = vol;
    
    return ret;
}

int saf7741_audioSetSecondVol(int vol)
{
    int ret = 0;
    unsigned char data1[2];
    unsigned char data2[2];
   
    vol = (vol*61)/100;

    if(vol > SAF7741_VOL_MAX) vol = SAF7741_VOL_MAX;
    if(vol < SAF7741_VOL_MIN) vol = SAF7741_VOL_MIN;

    data1[0] = (unsigned char)(vol_table[vol] >> 24);
    data1[1] = (unsigned char)(vol_table[vol] >> 16);
    data2[0] = (unsigned char)(vol_table[vol] >> 8);
    data2[1] = (unsigned char)(vol_table[vol]);

    ret = saf7741_i2c_write(ADSP_Y_Vol_Main1S, data1, sizeof(data1));
    ret |= saf7741_i2c_write(ADSP_Y_Vol_Main2S, data2, sizeof(data2));

    return ret;
}

int saf7741_audioSetPhoneVol(int vol)
{
    unsigned char data[2];
   
    vol = (vol*61)/100;
   
    if(vol > SAF7741_BTPHONE_VOL_MAX) vol = SAF7741_BTPHONE_VOL_MAX;
    if(vol < SAF7741_BTPHONE_VOL_MIN) vol = SAF7741_BTPHONE_VOL_MIN;

    data[0] = (unsigned char)(btphone_vol_table[vol] >> 8);
    data[1] = (unsigned char)(btphone_vol_table[vol]);
    
    return saf7741_i2c_write(ADSP_Y_Vol_Phon, data, sizeof(data));
}

int saf7741_audioSetNaviVol(int vol)
{
    unsigned char data[2];
   
    vol = (vol*61)/100;
   
    if(vol > SAF7741_NAVI_VOL_MAX) vol = SAF7741_NAVI_VOL_MAX;
    if(vol < SAF7741_NAVI_VOL_MIN) vol = SAF7741_NAVI_VOL_MIN;

    data[0] = (unsigned char)(navi_vol_table[vol] >> 8);
    data[1] = (unsigned char)(navi_vol_table[vol]);
    
    return saf7741_i2c_write(ADSP_Y_Vol_Nav, data, sizeof(data));
}


int saf7741_audioSetPrimaryMute(bool mute)
{
    unsigned char data[2];

    if(mute)
        i_to_2uc(data, 0x0000);
    else
        i_to_2uc(data, 0x0800);
    
    if(saf7741_i2c_write(ADSP_Y_Mute_P, data, sizeof(data)) < 0)
        return -1;

    gsAudioState.mute = mute;
    return 0;
}

int saf7741_audioSetSecondMute(bool mute)
{
    unsigned char data[2];

    if(mute)
        i_to_2uc(data, 0x0000);
    else
        i_to_2uc(data, 0x0800);
    
    if(saf7741_i2c_write(ADSP_Y_Mute_S, data, sizeof(data)) < 0)
        return -1;

    return 0;
}

int saf7741_audioSetPhoneMute(bool mute)
{
    unsigned char data[2];

    if(mute)
        i_to_2uc(data, 0x0000);
    else
        i_to_2uc(data, 0x0800);
    
    if(saf7741_i2c_write(ADSP_Y_Mute_T, data, sizeof(data)) < 0)
        return -1;

    return 0;
}

int saf7741_audioSetNaviMute(bool mute)
{
    unsigned char data[2];

    if(mute)
        i_to_2uc(data, 0x0000);
    else
        i_to_2uc(data, 0x0800);
    
    if(saf7741_i2c_write(ADSP_Y_Mute_N, data, sizeof(data)) < 0)
        return -1;

    return 0;
}



int saf7741_audioSetPrimaryBal(short bal)
{
    int ret = 0;
    unsigned char dataPL[2];
    unsigned char dataPR[2];

    bal += SAF7741_BALANCE_SIDE;

    if(bal > SAF7741_BALANCE_MAX) bal = SAF7741_BALANCE_MAX;
    if(bal < SAF7741_BALANCE_MIN) bal = SAF7741_BALANCE_MIN;
    

    dataPL[0] = (unsigned char)(balance_table[bal] >> 24);
    dataPL[1] = (unsigned char)(balance_table[bal] >> 16);
    dataPR[0] = (unsigned char)(balance_table[bal] >> 8);
    dataPR[1] = (unsigned char)(balance_table[bal]);

    ret = saf7741_i2c_write(ADSP_Y_Vol_BalPL, dataPL, sizeof(dataPL));
    ret = saf7741_i2c_write(ADSP_Y_Vol_BalPR, dataPR, sizeof(dataPR));

    return ret;
}

int saf7741_audioSetFade(short fade)
{
    int ret = 0; 
    unsigned char dataF[2];
    unsigned char dataR[2];
    
    fade += SAF7741_FADE_SIDE;

    if(fade > SAF7741_FADE_MAX) fade = SAF7741_FADE_MAX;
    if(fade < SAF7741_FADE_MIN) fade = SAF7741_FADE_MIN;
    
    dataF[0] = (unsigned char)(fade_table[fade] >> 24);
    dataF[1] = (unsigned char)(fade_table[fade] >> 16);
    dataR[0] = (unsigned char)(fade_table[fade] >> 8);
    dataR[1] = (unsigned char)(fade_table[fade]);

    ret = saf7741_i2c_write(ADSP_Y_Vol_FadF, dataF, sizeof(dataF));
    ret = saf7741_i2c_write(ADSP_Y_Vol_FadR, dataR, sizeof(dataR));

    return ret;
}

int saf7741_audioSetBassTone(short bass)
{
    int ret = 0, i;
    unsigned char buf[SAF7741_BASS_SIZE*sizeof(int)];
    unsigned char data[3]={0};

    bass += SAF7741_BASS_SIDE;

    if(bass > SAF7741_BASS_MAX) bass = SAF7741_BASS_MAX;
    if(bass < SAF7741_BASS_MIN) bass = SAF7741_BASS_MIN;

    for(i=0; i<SAF7741_BASS_SIZE; i++)
        i_to_4uc(&buf[i*4], bass_table[bass][i]);

    ret = saf7741_i2c_write(ADSP_Y_OFFSET, buf+2, sizeof(buf)-2);
    
    i_to_3uc(data, ADSP_Y_BMT_a1bHP);
    ret = saf7741_i2c_write(ADSP_X_SrcSw_OrDigSrcSelMask, data, sizeof(data));
    
    i_to_3uc(data, ADSP_EASYP_UpdateCoeff_Coeff11);
    ret = saf7741_i2c_write(ADSP_X_EasyP_Index, data, sizeof(data));
    
    
    return ret;
}

int saf7741_audioSetTrebleTone(short treble)
{
    int ret = 0, i;
    unsigned char buf[SAF7741_TREBLE_SIZE*sizeof(int)];
    unsigned char data[3]={0};
    
    treble += SAF7741_TREBLE_SIDE;

    if(treble > SAF7741_TREBLE_MAX) treble = SAF7741_TREBLE_MAX;
    if(treble < SAF7741_TREBLE_MIN) treble = SAF7741_TREBLE_MIN;
    
    for(i=0; i<SAF7741_TREBLE_SIZE; i++)
        i_to_4uc(&buf[i*4], treble_table[treble][i]);

    ret = saf7741_i2c_write(ADSP_Y_OFFSET, buf, sizeof(buf));
    
    i_to_3uc(data, ADSP_Y_BMT_a1tP);
    ret = saf7741_i2c_write(ADSP_X_SrcSw_OrDigSrcSelMask, data, sizeof(data));
    
    i_to_3uc(data, ADSP_EASYP_UpdateCoeff_Coeff6);
    ret = saf7741_i2c_write(ADSP_X_EasyP_Index, data, sizeof(data));
    
    return ret;
}

int saf7741_audioSetMidTone(short middle)
{
    int ret = 0, i;
    unsigned char buf[SAF7741_MID_SIZE*sizeof(int)];
    unsigned char data[3]={0};
    
    middle += SAF7741_MID_SIDE;

    if(middle > SAF7741_MID_MAX) middle = SAF7741_MID_MAX;
    if(middle < SAF7741_MID_MIN) middle = SAF7741_MID_MIN;
    
    for(i=0; i<SAF7741_MID_SIZE; i++)
        i_to_4uc(&buf[i*4], mid_table[middle][i]);

    ret = saf7741_i2c_write(ADSP_Y_OFFSET, buf+2, sizeof(buf)-2);
    
    i_to_3uc(data, ADSP_Y_BMT_a1mHP);
    ret = saf7741_i2c_write(ADSP_X_SrcSw_OrDigSrcSelMask, data, sizeof(data));
    
    i_to_3uc(data, ADSP_EASYP_UpdateCoeff_Coeff7);
    ret = saf7741_i2c_write(ADSP_X_EasyP_Index, data, sizeof(data));
    
    return ret;
}

int saf7741_audioSetLoudness(char enable)
{
    int ret = 0;



    return ret;
}

int saf7741_audioSetEQPattern(saf7741_eq_pattern pattern)
{
    int ret = 0;
//    unsigned char eq[2]={0}


    switch(pattern)
    {
        case SAF7741_EQ_JAZZ:
#if 0
            i_to_2uc(eq, 0x200);
            ret = saf7741_i2c_write(ADSP_Y_Vol_DesScalGEq, eq, sizeof(eq));
            




            
            i_to_2uc(eq, 0x5F6);
            ret |= saf7741_i2c_write(ADSP_Y_GEq_Gc1, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_GEq_Gc2, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_GEq_Gc3, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_GEq_Gc4, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_GEq_Gc5, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_EQ4_Gb1, eq, sizeof(eq));
            i_to_2uc(eq, 0x0);
            ret |= saf7741_i2c_write(ADSP_Y_EQ4_Gb2, eq, sizeof(eq));
            i_to_2uc(eq, 0x306);
            ret |= saf7741_i2c_write(ADSP_Y_EQ4_Gb3, eq, sizeof(eq));
#endif
            break;
        case SAF7741_EQ_POP:        break;
        case SAF7741_EQ_CLASSIC:    break;
        case SAF7741_EQ_ROCK:       break;
        default:                    break;
    }

    return ret;
}


int saf7741_audioSetChannelMode(saf7741_channel_mode mode)
{
    unsigned char data[2]={0};
    
    switch(mode)
    {
        case SAF7741_CH_MODE_STEREO:    
            i_to_2uc(data, ADSP_EASYP_PrimChannel_2ChannelMode);    
            break;
            
        case SAF7741_CH_MODE_6CHANNELS: 
            i_to_2uc(data, ADSP_EASYP_PrimChannel_6ChannelMode);    
            break;
            
        case SAF7741_CH_MODE_DEDEKIND:
        case SAF7741_CH_MODE_STANDARD:
        default:                         
            return 0;
    }
    
    return saf7741_i2c_write(ADSP_X_EasyP_Index,data,sizeof(data));
}



static qdsp_audio_ops ops;

const qdsp_audio_ops* saf7741_audioGetOps(void)
{    
    ops.volume  = saf7741_audioSetPrimaryVol;
    ops.source  = saf7741_audioSetSource;
    ops.mute    = saf7741_audioSetPrimaryMute;
    ops.balance = saf7741_audioSetPrimaryBal;
    ops.fade    = saf7741_audioSetFade;
    ops.bass    = saf7741_audioSetBassTone;
    ops.middle  = saf7741_audioSetMidTone;
    ops.treble  = saf7741_audioSetTrebleTone;

    gsAudioState.source = SAF7741_A_IIS3;

    return &ops;
}
