/*
 *  tef7000.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <stdio.h>

#include "tef7000.h"
#include "tef7000_regs.h"
#include "tef7000_i2c.h"

typedef struct {
    union{
      unsigned char data;  
      struct{  
          unsigned char    sub_addr:3;
          unsigned char    reserved:1;
          unsigned char    regc:1;
          unsigned char    mode:3;
      };
    };
}tef7000_msa;

typedef struct{
    char    band;
    short   freq;
    union{
        unsigned char ctrl;
        struct{  
            unsigned char    INJ_ctrl:1;
            unsigned char    INJ_mode:1;
            unsigned char    IF_bandwidth_ctrl:1;
            unsigned char    IF_bandwidth_mode:1;
            unsigned char    enable_LNA_output:1;
            unsigned char    antenna_mode:1;
            unsigned char    RFAGC:2;
        };
    }; 
}tef7000_turner_data;

typedef struct {
    union{
      unsigned char data;  
      struct{  
          unsigned char    IFCF:4;
          unsigned char    IFGAIN:2;
          unsigned char    MFAGC:2;
      };
    };
}tef7000_cfreq_align;

typedef struct {
    union{
      unsigned char data;  
      struct{  
          unsigned char    RFGAIN:1;
          unsigned char    RGATT:3;
          unsigned char    RIATT:3;
          unsigned char    RLATTM:1;
      };
    };
}tef7000_rfagc;

typedef struct {
    union{
      unsigned char data;  
      struct{  
          unsigned char    SWPB:1;
          unsigned char    SWPA:1;
          unsigned char    AGCEXT:1;
          unsigned char    IFATT:2;
          unsigned char    IFATTM:1;
          unsigned char    MFATT:1;
          unsigned char    MFATTM:1;
      };
    };
}tef7000_ifagc;

typedef struct {
    union{
      unsigned char data;  
      struct{  
          unsigned char    reserved:6;
          unsigned char    SWPC:1;
          unsigned char    STBY:1;
      };
    };
}tef7000_test;



typedef struct {
    union{
      unsigned char data;  
      struct{
          unsigned char    ID:3;
          unsigned char    reserved0:1;
          unsigned char    action:2;
          unsigned char    reserved1:1;
          unsigned char    power:1;
      };
    };
}tef7000_status;


typedef struct {
    union{
      unsigned char data;  
      struct{
          unsigned char    INJR:1;
          unsigned char    IFBWR:1;
          unsigned char    MFDET:2;
          unsigned char    RCDET:2;
          unsigned char    RLDET:2;
      };
    };
}tef7000_det;

typedef struct {
    union{
      unsigned char data;  
      struct{
          unsigned char    MFATR:1;
          unsigned char    reserved:1;
          unsigned char    RGATR:3;
          unsigned char    RIATR:3;
      };
    };
}tef7000_agc;

typedef struct {
    union{
      unsigned char data;  
      struct{
          unsigned char    reserved:6;
          unsigned char    IFATR:2;
      };
    };
}tef7000_vid;


typedef struct{
    tef7000_msa         msa;
    tef7000_turner_data turner;
    tef7000_cfreq_align cfalign;
    tef7000_rfagc       rfagc;
    tef7000_ifagc       ifagc;
    tef7000_test        test;
    char                xtest;

    tef7000_status      status;
    tef7000_det         det;
    tef7000_agc         agc;
    tef7000_vid         vid;
}tef7000_data;

static tef7000_data tef7000;
static qtuner_ops ops;

int tef7000_reset(void);
int tef7000_commit(void);
int tef7000_pull_status(void);
int tef7000_get_status(void);
void tef7000_show_status(void);

int  tef7000_set_param(qtuner_mode mode, qtuner_band band, short freq);
void tef7000_clear_param(void);

int  tef7000_set_ifcf(qtuner_mode mode, char ifcf);

void tef7000_fill_mode(qtuner_mode mode);
void tef7000_fill_band(qtuner_band band);
void tef7000_fill_freq(short freq);

int tef7000_reset()
{   
    tef7000.msa.mode     = MSA_MODE_SEARCH;
    tef7000.msa.regc     = MSA_REGC_CONTROL;
    tef7000.msa.sub_addr = TEF7000_REG_IFAGC;

    tef7000.ifagc.data   = 0;
    tef7000.ifagc.IFATTM = 1;
    tef7000.ifagc.IFATT  = 3;

    // 0xC05538
    // printf("SEC_I2C_EN : 0x%02x%02x%02x\n", TEF7000_ADDR_GROUND, tef7000.msa.data, tef7000.ifagc.data);  
    
    return tef7000_i2c_write(tef7000.msa.data, &tef7000.ifagc.data, sizeof(tef7000.ifagc.data));
}

int tef7000_commit()
{
    unsigned char buf[TEF7000_I2C_MAX_W_DATA_LEN] = {0};

    buf[0] = (tef7000.turner.band << 5) | ((tef7000.turner.freq >> 8) & 0x1F);
    buf[1] = tef7000.turner.freq & 0xFF;
    buf[2] = tef7000.turner.ctrl;
    buf[3] = tef7000.cfalign.data;
    buf[4] = tef7000.rfagc.data;
    buf[5] = tef7000.ifagc.data;
    buf[6] = tef7000.test.data;
    buf[7] = tef7000.xtest;

    return tef7000_i2c_write(tef7000.msa.data, buf, sizeof(buf));
}

int tef7000_pull_status()
{
    unsigned char buf[TEF7000_I2C_MAX_R_DATA_LEN] = {0};

    if(tef7000_i2c_read(buf, sizeof(buf)) < 0)
        return -1;

    tef7000.status.data = buf[0];
    tef7000.det.data    = buf[1];
    tef7000.agc.data    = buf[2];
    tef7000.vid.data    = buf[3];

#if 1
    tef7000_show_status();
#endif
   
    return 0;
}

int tef7000_get_status()
{
    int status = 0;

    status |= tef7000.status.data << 24;
    status |= tef7000.det.data    << 16;
    status |= tef7000.agc.data    << 8;
    status |= tef7000.vid.data;
    
    return status;
}

int  tef7000_set_param(qtuner_mode mode, qtuner_band band, short freq)
{
    tef7000_fill_mode(mode);
    tef7000_fill_band(band);
    tef7000_fill_freq(freq);

    if(tef7000_commit() < 0)
        return -1;

    if(mode == QTUNER_MODE_PRESET)
    {
        if(band >= QTUNER_BAND_AM_LW && band <= QTUNER_BAND_AM_SW)
            printf("tef7000 : set AM %d kHz\n",freq);
        else if (band == QTUNER_BAND_FM || band == QTUNER_BAND_FM_OIRT)
            printf("tef7000 : set FM %d.%d MHz\n",freq/100,freq%100);
    }
    
    return 0;
}


int tef7000_search(qtuner_band band, short freq)
{
    tef7000_fill_mode(QTUNER_MODE_SEARCH);
    tef7000_fill_band(band);
    tef7000_fill_freq(freq);
 
    return tef7000_commit();
}

void tef7000_clear_data()
{
    tef7000.cfalign.data = 0;
    tef7000.rfagc.data   = 0;
    tef7000.ifagc.data   = 0;
    tef7000.test.data    = 0;
}

int tef7000_set_ifcf(qtuner_mode mode, char ifcf)
{
    tef7000_fill_mode(mode);
    tef7000.cfalign.IFCF = ifcf;
    return tef7000_commit();
}

void tef7000_fill_mode(qtuner_mode mode)
{
    switch(mode)
    {
        case QTUNER_MODE_BUFFER:       tef7000.msa.mode = MSA_MODE_BUFFER;     break;
        case QTUNER_MODE_PRESET:       tef7000.msa.mode = MSA_MODE_PRESET;     break;
        case QTUNER_MODE_SEARCH:       tef7000.msa.mode = MSA_MODE_SEARCH;     break;
        case QTUNER_MODE_AF_UPDATE:    tef7000.msa.mode = MSA_MODE_AF_UPDATE;  break;
        case QTUNER_MODE_AF_JUMP:      tef7000.msa.mode = MSA_MODE_AF_JUMP;    break;
        case QTUNER_MODE_AF_CHECK:     tef7000.msa.mode = MSA_MODE_AF_CHECK;   break;
        case QTUNER_MODE_LOAD:         tef7000.msa.mode = MSA_MODE_LOAD;       break;
        case QTUNER_MODE_END:          tef7000.msa.mode = MSA_MODE_END;        break;
        default:                                                                break;
    }

    if(mode == QTUNER_MODE_SEARCH)
    {
        tef7000.msa.regc = MSA_REGC_BUFFER;
    }
    else
    {
        tef7000.msa.regc = MSA_REGC_CONTROL;
    }
    tef7000.msa.sub_addr = TEF7000_REG_TUNER;
}

void tef7000_fill_band(qtuner_band band)
{
    switch(band)
    {
        case QTUNER_BAND_AM_LW:        tef7000.turner.band = BAND_TYPE_AM_LW;      break;
        case QTUNER_BAND_AM_MW_EU:     tef7000.turner.band = BAND_TYPE_AM_MW_EU;   break;
        case QTUNER_BAND_AM_MW_USA:    tef7000.turner.band = BAND_TYPE_AM_MW_USA;  break;
        case QTUNER_BAND_AM_SW:        tef7000.turner.band = BAND_TYPE_AM_SW;      break;
        case QTUNER_BAND_FM:           tef7000.turner.band = BAND_TYPE_FM;         break;
        case QTUNER_BAND_FM_OIRT:      tef7000.turner.band = BAND_TYPE_FM_OIRT;    break;
        case QTUNER_BAND_WB:           tef7000.turner.band = BAND_TYPE_WB;         break;
        case QTUNER_BAND_ORBCOMM:      tef7000.turner.band = BAND_TYPE_ORBCOMM;    break;
        default:                                                                    break;
    }

    if(band == QTUNER_BAND_FM || band == QTUNER_BAND_FM_OIRT)
    {
        tef7000.turner.INJ_ctrl          = 0x0;
        tef7000.turner.INJ_mode          = 0x0;
        tef7000.turner.IF_bandwidth_ctrl = 0x0;
        tef7000.turner.IF_bandwidth_mode = 0x0;
        tef7000.turner.enable_LNA_output = 0x0;
        tef7000.turner.antenna_mode      = 0x0;
        tef7000.turner.RFAGC             = 0x2;
        tef7000.cfalign.IFCF             = 0x0;
        tef7000.cfalign.IFGAIN           = 0x0;
        tef7000.cfalign.MFAGC            = 0x3;
        tef7000.rfagc.data               = 0x0;
        tef7000.ifagc.data               = 0x0;
        tef7000.test.data                = 0x0;
    }
    else if(band != QTUNER_BAND_WB && band != QTUNER_BAND_ORBCOMM)
    {
        tef7000.turner.ctrl              = 0x0;
        tef7000.cfalign.data             = 0x0;
        tef7000.rfagc.data               = 0x0;
        tef7000.ifagc.data               = 0x0;
        tef7000.test.data                = 0x0;
    }   
}

void tef7000_fill_freq(short freq)
{    
    if(tef7000.turner.band == QTUNER_BAND_FM || tef7000.turner.band == QTUNER_BAND_FM_OIRT)
    {
        tef7000.turner.freq     = freq / 5;     //  freq [10kHz] / 5
    }
    else if(tef7000.turner.band != QTUNER_BAND_WB && tef7000.turner.band != QTUNER_BAND_ORBCOMM)
    {
        tef7000.turner.freq     = freq;         // freq [kHz] / 1
    }
}

const qtuner_ops* tef7000_getOps()
{
    ops.init      = NULL;
    ops.reset     = tef7000_reset;
    ops.open      = tef7000_i2c_open;
    ops.close     = tef7000_i2c_close;
    ops.search    = tef7000_search;
    ops.set_freq  = tef7000_set_param;
    ops.set_ifcf  = tef7000_set_ifcf;
    ops.clear     = tef7000_clear_data;
    return &ops;
}


// =============================
// print information
// =============================

const char* _tef7000_strdet(char val)
{
    switch(val)
    {
        case 0: return "no AGC step request";
        case 1: return "AGC gain step request (low)";
        case 2: return "AGC attenuation step request (high)";
        default:
            break;
    }
    return "unknown";
}

const char* _tef7000_stract(char val)
{
    switch(val)
    {
        case 0: return "no tuning action";
        case 1: return "action active, wait 1 ms to process muting";
        case 2: return "action active, tuning in progress";
        case 3: return "tuning ready and muted";
        default:
            break;
    }
    return "unknown";
}


void tef7000_show_status()
{
    printf("status : 0x%02x%02x%02x%02x\n",
                tef7000.status.data,
                tef7000.det.data,
                tef7000.agc.data,
                tef7000.vid.data);
    printf("Device ID             : %s\n", (tef7000.status.ID == 0) ? "TEF7000" : "unknown device");
    printf("Power-on reset        : %s\n", (tef7000.status.power) ? "power dip detected" : "normal");
    printf("Tuning action         : %s\n", _tef7000_stract(tef7000.status.action));
    printf("IF bandwith           : %s\n", (tef7000.det.IFBWR) ? "wide" : "narrow");
    printf("LO injection          : %s side \n", (tef7000.det.INJR) ? "low" : "high");

    if(tef7000.turner.band == QTUNER_BAND_FM || tef7000.turner.band == QTUNER_BAND_FM_OIRT)
    {
        printf("FM IF filter          : %s\n", _tef7000_strdet(tef7000.det.MFDET));
        printf("FM RF LNA voltage     : %s\n", _tef7000_strdet(tef7000.det.RLDET));
        printf("FM IF filter AGC step : %s\n", (tef7000.agc.MFATR) ? "attenuation" : "no attenuation");
        printf("FM RF AGC step        : %d (0~7)\n", tef7000.agc.RIATR);

    }
    else if(tef7000.turner.band != QTUNER_BAND_WB && tef7000.turner.band != QTUNER_BAND_ORBCOMM)
    {
        printf("AM RF mixer/LNA       : %s\n", _tef7000_strdet(tef7000.det.MFDET));
        printf("AM RF LNA current     : %s\n", _tef7000_strdet(tef7000.det.RCDET));
        printf("AM RF LNA voltage     : %s\n", _tef7000_strdet(tef7000.det.RLDET));
        printf("AM RF mixer AGC step  : %s\n", (tef7000.agc.MFATR) ? "attenuation" : "no attenuation");
        printf("AM RF LNA gain        : %d (0~4)\n", tef7000.agc.RGATR);
        printf("AM RF AGC step        : %d (0~7)\n", tef7000.agc.RIATR);
    }
    printf("IF AGC step           : %d (0~3)\n", tef7000.vid.IFATR);
}
