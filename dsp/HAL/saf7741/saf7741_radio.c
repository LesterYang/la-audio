/*
 *  saf7741_radio.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <qDspDev.h>

#include "saf7741_radio.h"
#include "saf7741_radio_regs.h"
#include "saf7741_util.h"
#include "saf7741_i2c.h"

#include "../qTuner.h"

unsigned char gpDefaultAlign[E2P_RADIO_ALIGN_LEN] =
#if 1
{  
  0x06,             //IFCF FM 
  0x06,             //IFCF AM         
  0xE2, 0xD9, 0x65, //offset FMIX0 FM   
  0xE2, 0x12, 0xEA, //offset FMIX1 FM 
  0xFA, 0x70, 0xF2, //offset FMIX0 AM  
  0xFA, 0x6E, 0xC2, //offset FMIX1 AM  
  0x0F, 0xC3,       //level FM 
  0x0F, 0xA6        //level AM
};
#endif
#if 0
{
    0x08, 0x08,          
    0xE2, 0x76, 0x28, 0xE2, 0x76, 0x28, 0xFA, 0x6F, 0xDA, 0xFA, 0x6F, 0xDA, 
    0x0F, 0xC3, 0x0F, 0xA6
};
#endif
#if 0
{
    0x00, 0x00,          
    0xE2, 0x76, 0x47, 0xE2, 0x76, 0x47, 0xFA, 0x6F, 0xDA, 0xFA, 0x6F, 0xDA, 
    0x0F, 0xC2, 0x0F, 0xE0
};
#endif



struct saf7741_radio_status{
    bool              defaultAlign;
    qdsp_radio_band   band;
    short             freqFM;
    short             freqAM;
    unsigned char     align[E2P_RADIO_ALIGN_LEN];
    const qtuner_ops* tuner;
};

static struct saf7741_radio_status gsRadioState;

int saf7741_radioSetAMBand()
{
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->set_freq);
    
    if(gsRadioState.tuner->set_freq(QTUNER_MODE_LOAD, QTUNER_BAND_AM_MW_EU, DEFAULT_AM_FREQ) < 0)
        return -1;

    if(saf7741_radioSetAMAdjacent() < 0)
        return -1;

    if(saf7741_radioSetRadioMode(RADIO_MODE_AM) < 0)
        return -1;

    if(gsRadioState.tuner->set_freq(QTUNER_MODE_PRESET, QTUNER_BAND_AM_MW_EU, DEFAULT_AM_FREQ) < 0)
        return -1;
    else if(gsRadioState.freqAM != DEFAULT_AM_FREQ)
        saf7741_radioSetFreq(gsRadioState.freqAM);


    if(saf7741_radioSetAMLvAlign(DEFAULT_ALIGN_AM_LV) < 0)
        return -1;

    if(saf7741_radioSetPDC2FMIX(DEFAULT_AM_PDC2_FMIX0, DEFAULT_AM_PDC2_FMIX1) < 0)
        return -1;

#if 0
        // set -4.7db, 0x000(no audio) - 0x7FF(full audio)
        if(saf7741_radioSetVolScaler(0x0530) < 0)
            return -1;
#endif

    if(saf7741_radioAMAlign() < 0)
        return -1;

    saf7741_log(DbgRadio, "Switch to AM");

    return 0;
}

int saf7741_radioSetFMBand()
{
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->set_freq);

    if(gsRadioState.tuner->set_freq(QTUNER_MODE_LOAD, QTUNER_BAND_FM, DEFAULT_FM_FREQ) < 0)
        return -1;

    if(saf7741_radioSelecAnt(ANTENNA_A) < 0)
        return -1;

    if(saf7741_radioSetFMAdjacent() < 0)
        return -1;

    if(saf7741_radioRBSDefault() < 0)
        return -1;
    
    if(saf7741_radioFMDeEmphasis() < 0)
        return -1;
    
    if(saf7741_radioSetRadioMode(RADIO_MODE_FM) < 0)
        return -1;

    if(gsRadioState.tuner->set_freq(QTUNER_MODE_PRESET, QTUNER_BAND_FM, DEFAULT_FM_FREQ) < 0)
        return -1;
    else if(gsRadioState.freqFM != DEFAULT_FM_FREQ)
        saf7741_radioSetFreq(gsRadioState.freqFM);

    if(saf7741_radioSetFMLvAlign(DEFAULT_ALIGN_FM_LV) < 0)
        return -1;

    if(saf7741_radioSetFMLRDecoderCor(DEFAULT_STEREO_DECODER_COR) < 0)
        return -1;

    if(saf7741_radioSetPDC2FMIX(DEFAULT_FM_PDC2_FMIX0, DEFAULT_FM_PDC2_FMIX1) < 0)
        return -1;

#if 0
    // set -9.3db, 0x000(no audio) - 0x7FF(full audio)
    if(saf7741_radioSetVolScaler(0x7FF) < 0)
        return -1;
#endif

    if(saf7741_radioFMAlign() < 0)
        return -1;

    saf7741_log(DbgRadio, "Switch to FM");

    return 0;
}


int saf7741_radioOpenTuner()
{
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->open);

    return gsRadioState.tuner->open();
}

int saf7741_radioResetTuner()
{
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->reset);

    return gsRadioState.tuner->reset();
}

int saf7741_radioStartUpTuner()
{
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->clear);
    int_saf7741_check_func(gsRadioState.tuner->set_freq);

    // 30. Level alignment for FM radio mode
    // 31. Stereo alignment for FM radio mode
    // 32. Frequency offset alignment value FMIX0 for FM
    // 33. Frequency offset alignment value FMIX1 for FM
    // 34. Set radio mode to FM
    
    
    // 35. Need to be sent always after radio mode selection (command to FM,92.7MHz)
    gsRadioState.tuner->clear();
    
    if(gsRadioState.tuner->set_freq(QTUNER_MODE_PRESET, QTUNER_BAND_FM, 9270) < 0)
        saf7741_log(DbgStartUpRadio, "set tuner FM error");

    return 0;
}


int saf7741_radioSetBand(qdsp_radio_band band)
{
    int ret = -1;
    switch(band)
    {
        case QDSP_BAND_AM:  ret = saf7741_radioSetAMBand();             break;
        case QDSP_BAND_FM:  ret = saf7741_radioSetFMBand();             break;
        default:            saf7741_log(DbgRadio, "uset nknown band");  break;
    }
    
    if(ret == 0)
        gsRadioState.band = band;
    return ret;
}

int saf7741_radioSetFreq(short freq)
{
    int ret = -1;
    
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->set_freq);
    
    switch(gsRadioState.band)
    {
        case QDSP_BAND_AM:
            if(gsRadioState.freqAM != freq)
            {
                ret = gsRadioState.tuner->set_freq(QTUNER_MODE_PRESET, QTUNER_BAND_AM_MW_EU, freq);
                if(ret == 0) 
                    gsRadioState.freqAM = freq; 
            }
            break;
            
        case QDSP_BAND_FM:  
            if(gsRadioState.freqFM != freq)
            {
                ret = gsRadioState.tuner->set_freq(QTUNER_MODE_PRESET, QTUNER_BAND_FM, freq);
                if(ret == 0) 
                    gsRadioState.freqFM = freq; 
            }
            break;
            
        default:                                           
            break;;
    }
    
    return ret;
}

bool saf7741_radioSearchFreq(qdsp_radio_band band, short freq)
{
#if 0
    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->reset);

    if()
        gsRadioState.tuner->set_mode

    
    switch(gsRadioState.band)
    {
        case QDSP_BAND_AM:
            ret = gsRadioState.tuner->set_freq(QTUNER_MODE_SEARCH, QTUNER_BAND_AM_MW_EU, freq);
            if(ret == 0) 
                gsRadioState.freqAM = freq; 
            break;
            
        case QDSP_BAND_FM:  
            ret = gsRadioState.tuner->set_freq(QTUNER_MODE_SEARCH, QTUNER_BAND_FM, freq);
             if(ret == 0) 
                gsRadioState.freqFM = freq; 
            break;
            
        default:                                           
            break;;
    }

    gsRadioState.tuner->set_freq
 #endif
    return true;
}

bool saf7741_radioPilot()
{
    unsigned char pilot[2]={0};
    
    if(saf7741_i2c_read(TDSP1E_Y_wFIPL_1_Pil_out, pilot, sizeof(pilot)) < 0)
        return false;

    return (pilot[0] == 0x7 && pilot[1] == 0xFF) ? true : false;
}

void saf7741_radioTunerStatus()
{
    unsigned char status[3]={0};
    int val;
    
    if(saf7741_i2c_read(TDSP1_X_FW_RadStat, status, sizeof(status)) < 0)
        return;

    val = status[0]*256*256 + status[1]*256 + status[2];

    if(val == 0x7FFFFF)
         saf7741_log(DbgRadio, "Tuner Status : ON");
    else if(val == 0)
         saf7741_log(DbgRadio, "Tuner Status : INIT");
    else
         saf7741_log(DbgRadio, "Tuner Status : OFF");
}


static qdsp_radio_ops ops;

const qdsp_radio_ops* saf7741_radioGetOps(void)
{
    ops.band   = saf7741_radioSetBand;
    ops.freq   = saf7741_radioSetFreq;
    ops.search = saf7741_radioSearchFreq;
    ops.pilot  = saf7741_radioPilot;

    gsRadioState.freqAM       = DEFAULT_AM_FREQ;
    gsRadioState.freqFM       = DEFAULT_FM_FREQ;
    gsRadioState.defaultAlign = true;
    gsRadioState.tuner        = qturner_getOps(QTUNER_DEV_TEF7000);

    return (gsRadioState.tuner == NULL) ? NULL : &ops;
}


int saf7741_radioSetRadioMode(int mode)
{
    unsigned char VAL[3] = {0};
    i_to_3uc(VAL, mode);
    return (saf7741_i2c_write(TDSP1_X_FW_RadMod, VAL, sizeof(VAL)) < 0) ? -1 : 0;
}

int saf7741_radioSelecAnt(int ant)
{
    //Refer to Radio User Manual p.122
    unsigned char VAL[3] = {0};
    
    i_to_3uc(VAL, ant);
    return (saf7741_i2c_write(TDSP2_X_FW_AntSelFix, VAL, sizeof(VAL)) < 0) ? -1 : 0;
}

int saf7741_radioRBSDefault()
{
    // Refer to Radio User Manual p.119
    unsigned char VAL[3] = {0x00, 0x00, 0x08};
    return (saf7741_i2c_write(TDSP2_X_FW_CIBW_2_FixSet_in, VAL, sizeof(VAL)) < 0) ? -1 : 0;
}

int saf7741_radioFMDeEmphasis()
{
    // Refer to Radio User Manual p.70
    unsigned char VAL[2] = {0};
    
    i_to_2uc(VAL, 0x02C0);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Demb10, VAL, sizeof(VAL)) < 0)
        return -1;
    i_to_2uc(VAL, 0x04E4);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Dema11, VAL, sizeof(VAL)) < 0)
        return -1;
    i_to_2uc(VAL, 0x0085);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Demb20, VAL, sizeof(VAL)) < 0)
        return -1;
    
    return 0;
}


int saf7741_radioSetAMAdjacent()
{
    unsigned char VAL_WORD[2] = {0};
    unsigned char VAL[3] = {0};
  
    i_to_2uc(VAL_WORD, 0x07FF);         // dynamic adjacent channel
    if(saf7741_i2c_write(TDSP1_Y_ACIB_1_BWSetMan, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    
    i_to_3uc(VAL, 0x00000A);            
    if(saf7741_i2c_write(TDSP1_X_ACIB_1_FirCtlFix, VAL, sizeof(VAL)) < 0)
        return -1;

    #if 0 // ??
    i_to_2uc(VAL_WORD, 0x0000);         
    if(saf7741_i2c_write(0x03008, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    #endif
    
    return 0;
}

int saf7741_radioSetFMAdjacent()
{
    // Refer to Radio User Manual 4.3.1

    unsigned char VAL_WORD[2] = {0};
    unsigned char VAL[3] = {0};
  
    i_to_2uc(VAL_WORD, 0x0000);         // dynamic adjacent channel
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_BWSetMan, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_2uc(VAL_WORD, 0x0002);         // release time for low devation signals
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMinDec, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_2uc(VAL_WORD, 0x0005);         // release time for large devation signals
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMaxDec, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    //i_to_2uc(VAL_WORD, 0x0003);
    i_to_2uc(VAL_WORD, 0x0004);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanSlpDec, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanOfsDec, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_3uc(VAL, 0x0FC0FC);            // attack time
    if(saf7741_i2c_write(TDSP1_X_FCIB_1_AttLim, VAL, sizeof(VAL)) < 0)
        return -1;
    //i_to_3uc(VAL, 0x000000);
    i_to_3uc(VAL, 0x200000);            // minimun bandwidth
    if(saf7741_i2c_write(TDSP1_X_FCIB_1_DefaultMinBW, VAL, sizeof(VAL)) < 0)
        return -1;
    //i_to_2uc(VAL_WORD, 0x011E);
    i_to_2uc(VAL_WORD, 0x01AE);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanSlpBst, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    //i_to_2uc(VAL_WORD, 0x0FDD);
    i_to_2uc(VAL_WORD, 0x0FE0);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanOfsBst, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1; 
    //i_to_2uc(VAL_WORD, 0x01EC);
    i_to_2uc(VAL_WORD, 0x0164);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanSlpTE, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1; 
    //i_to_2uc(VAL_WORD, 0x0FF2);
    i_to_2uc(VAL_WORD, 0x0FE7);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanOfsTE, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1; 
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMinTE, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1; 
    //i_to_2uc(VAL_WORD, 0x0400);
    i_to_2uc(VAL_WORD, 0x0300);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMaxTE, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1; 
    
    return 0;
}


int saf7741_radioSetVolScaler(short gain)
{
    unsigned char factor[2] = {0};
        
    i_to_2uc(factor, gain);
    return (saf7741_i2c_write(TDSP1E_Y_wFW_VolRed_in, factor, sizeof(factor)) < 0) ? -1 : 0;
}

int saf7741_radioSetFMLRDecoderCor(short val)
{
    unsigned char factor[2] = {0};
    
    i_to_2uc(factor, val);
    return (saf7741_i2c_write(TDSP1E_Y_FMLR_1_GanCor, factor, sizeof(factor)) < 0) ? -1 : 0;
}

int saf7741_radioSetFMLvAlign(short val)
{
    unsigned char offset[2] = {0};
    
    i_to_2uc(offset, val);
    return (saf7741_i2c_write(TDSP1E_Y_FW_FDLE_1_RanOfs, offset, sizeof(offset)) < 0) ? -1 : 0;
}

int saf7741_radioSetAMLvAlign(short val)
{
    unsigned char offset[2] = {0};
    
    i_to_2uc(offset, val);
    return (saf7741_i2c_write(TDSP1E_Y_FW_ADLE_1_RanOfs, offset, sizeof(offset)) < 0) ? -1 : 0;
}


int saf7741_radioSetPDC2FMIX(int fmix0, int fmix1)
{
    int ret = 0;
    
    unsigned char freq0[3] = {0};
    unsigned char freq1[3] = {0};
    
    i_to_3uc(freq0, fmix0);
    i_to_3uc(freq1, fmix1);

    ret |= saf7741_i2c_write(PDC2_FMIX_0, freq0, sizeof(freq0));
    ret |= saf7741_i2c_write(PDC2_FMIX_1, freq1, sizeof(freq1));
    
    return (ret != 0) ? -1 : 0;
}

void saf7741_radioStoreAlign()
{
    int fd;

    if(access(SAF7741_DIRT_BACKUP, F_OK)!= 0)
    {
        mkdir(SAF7741_DIRT_BACKUP, 0777); 
    }

    fd = open(SAF7741_ALIGN_BACKUP, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if(fd > 0)
    {
        write(fd, gsRadioState.align, E2P_RADIO_ALIGN_LEN);
        sync();
    }
    else
        saf7741_log(DbgRadio,"_radioStoreAlign() open error");

    close(fd);
}

void saf7741_radioLoadAlign()
{
    return;
    
    int fd = open(SAF7741_ALIGN_BACKUP, O_RDONLY);
    
    if(fd < 0)
    {
        if(errno == ENOENT)
            saf7741_log(DbgRadio,"_radioLoadAlign() no backup data");
        else
            saf7741_log(DbgRadio,"_radioLoadAlign() open error");
        goto default_setting;
    }
    
    if(read(fd, gsRadioState.align, E2P_RADIO_ALIGN_LEN) == E2P_RADIO_ALIGN_LEN)
    {
        gsRadioState.defaultAlign = false;
        close(fd);
        return;
    }

    close(fd);

default_setting:
    gsRadioState.defaultAlign = true;
}

int saf7741_radioFMAlign()
{
    unsigned char *align;
    unsigned char freq[3] = {0};
    unsigned char level[2] = {0};
    int ret = 0;

    if(gsRadioState.defaultAlign)
        align = gpDefaultAlign;
    else
        align = gsRadioState.align;

    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->set_ifcf);

    //IFCF
    ret = gsRadioState.tuner->set_ifcf(QTUNER_MODE_PRESET, (char)align[0]);

    //OFFSET
    freq[0] = align[2];
    freq[1] = align[3];
    freq[2] = align[4];
    ret = saf7741_i2c_write(PDC2_FMIX_0, freq, sizeof(freq));

    freq[0] = align[5];
    freq[1] = align[6];
    freq[2] = align[7];
    ret = saf7741_i2c_write(PDC2_FMIX_1, freq, sizeof(freq));

    //LEVEL
    level[0] = align[14];
    level[1] = align[15];

    saf7741_log(DbgRadio, "FM IFCF  : 0x%02X",align[0]);
    saf7741_log(DbgRadio, "FM FMXI0 : 0x%06X",align[2]*256*256+align[3]*256+align[4]);
    saf7741_log(DbgRadio, "FM FMIX1 : 0x%06X",align[5]*256*256+align[6]*256+align[7]);
    saf7741_log(DbgRadio, "FM LEVEL : 0x%04X",align[14]*256+align[15]);

    ret = saf7741_i2c_write(TDSP1E_Y_FW_ADLE_1_RanOfs, level, sizeof(level));

    return (ret != 0) ? -1 : 0;
}

int saf7741_radioAMAlign()
{
    unsigned char *align;
    unsigned char freq[3] = {0};
    unsigned char level[2] = {0};
    int ret = 0;

    if(gsRadioState.defaultAlign)
        align = gpDefaultAlign;
    else
        align = gsRadioState.align;

    int_saf7741_check_func(gsRadioState.tuner);
    int_saf7741_check_func(gsRadioState.tuner->set_ifcf);

    //IFCF
    ret = gsRadioState.tuner->set_ifcf(QTUNER_MODE_PRESET, (char)align[1]);

    //OFFSET
    freq[0] = align[8];
    freq[1] = align[9];
    freq[2] = align[10];
    ret = saf7741_i2c_write(PDC2_FMIX_0, freq, sizeof(freq));

    freq[0] = align[11];
    freq[1] = align[12];
    freq[2] = align[13];
    ret = saf7741_i2c_write(PDC2_FMIX_1, freq, sizeof(freq));

    //LEVEL
    level[0] = align[16];
    level[1] = align[17];

    saf7741_log(DbgRadio, "AM IFCF  : 0x%02X",align[1]);
    saf7741_log(DbgRadio, "AM FMXI0 : 0x%06X",align[8]*256*256+align[9]*256+align[10]);
    saf7741_log(DbgRadio, "AM FMIX1 : 0x%06X",align[11]*256*256+align[12]*256+align[13]);
    saf7741_log(DbgRadio, "AM LEVEL : 0x%04X",align[16]*256+align[17]);
    
    ret = saf7741_i2c_write(TDSP1E_Y_FW_ADLE_1_RanOfs, level, sizeof(level));

    return (ret != 0) ? -1 : 0;

}



