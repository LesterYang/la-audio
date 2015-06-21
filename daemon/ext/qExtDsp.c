/*
 *  qExtDsp.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <string.h>

#include "qAudioIpc.h"
#include "qAudioLog.h"

#include "qExtDsp.h"
#include "qExtDspAudio.h"
#include "qExtDspRadio.h"

#include "qDsp.h"

static char target[7] = {0};

int qext_dsp_ipc_len_error(unsigned char hdr, unsigned int len);


int qext_dsp_init()
{
    if(qdsp_open(QDSP_DEV_SAF7741) != 0)
    {
        la_log_error("init dsp error !!");
        return -1;
    }

    if(qdsp_reset() != 0)
    {
        la_log_error("reset dsp fail !!");
        return -1;
    }     

    memcpy(target, "LSTQT0", 6);
    qext_dsp_load_data();

    la_log_notice("qext_dsp_init()... done"); 
    return 0;
}

void qext_dsp_deinit()
{
    qext_dsp_backup_data();
    qdsp_close();
}

void qext_dsp_load_data()
{
    qext_dsp_audio_load();
    qext_dsp_radio_load();
}

void qext_dsp_backup_data()
{
    la_log_warn("qext_dsp_backup_data");
    qext_dsp_audio_save();
    qext_dsp_radio_save();
}


int qext_dsp_ipc_len_error(unsigned char hdr, unsigned int len)
{
    unsigned int min;
    
    switch(hdr)
    {
        case QEXT_DSP_CMD_AUDIO_STATUS:         min = 1;    break;    
        case QEXT_DSP_CMD_AUDIO_SOURCE:         min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_MUTE:           min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_VOLUME:         min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_SPEED:          min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_FAD_BAL:        min = 3;    break;
        case QEXT_DSP_CMD_AUDIO_BAS_MID_TRE:    min = 4;    break;
        case QEXT_DSP_CMD_AUDIO_LOUDNESS:       min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_EQ:             min = 2;    break;
        case QEXT_DSP_CMD_AUDIO_VOICE_STAGE:    min = 1;    break;
        case QEXT_DSP_CMD_RADIO_STATUS:         min = 1;    break;
        case QEXT_DSP_CMD_RADIO_CHANGE:         min = 3;    break;
        case QEXT_DSP_CMD_RADIO_RUN_MODE:       min = 3;    break;
        case QEXT_DSP_CMD_RADIO_SET_FREQ:       min = 5;    break;
        case QEXT_DSP_CMD_RADIO_PRESET:         min = 3;    break;
        default:                                min = ~0;   break;                                       
    }

    return (len < min) ? 1 : 0;
}

void qext_dsp_ipc_audio(const char *from, unsigned int len, unsigned char *msg)
{
    la_log_debug("dsp audio cmd from %s, data 0x%02x 0x%02x", from, msg[0], msg[1]);

    if(qext_dsp_ipc_len_error(msg[0], len))
        return;

    memcpy(target, from, 6);

    switch(msg[0])
    {
        case QEXT_DSP_CMD_AUDIO_STATUS:         qext_dsp_audio_status();                    break;
        case QEXT_DSP_CMD_AUDIO_SOURCE:         qext_dsp_audio_source(len-1, &msg[1]);      break;
        case QEXT_DSP_CMD_AUDIO_MUTE:           qext_dsp_audio_mute(len-1, &msg[1]);        break;
        case QEXT_DSP_CMD_AUDIO_VOLUME:         qext_dsp_audio_volume(len-1, &msg[1]);      break;
        case QEXT_DSP_CMD_AUDIO_SPEED:          qext_dsp_audio_speed(len-1, &msg[1]);       break;
        case QEXT_DSP_CMD_AUDIO_FAD_BAL:        qext_dsp_audio_fad_bal(len-1, &msg[1]);     break;
        case QEXT_DSP_CMD_AUDIO_BAS_MID_TRE:    qext_dsp_audio_bas_mid_tre(len-1, &msg[1]); break;
        case QEXT_DSP_CMD_AUDIO_LOUDNESS:       qext_dsp_audio_loudness(len-1, &msg[1]);    break;
        case QEXT_DSP_CMD_AUDIO_EQ:             qext_dsp_audio_eq(len-1, &msg[1]);          break;
        case QEXT_DSP_CMD_AUDIO_VOICE_STAGE:    qext_dsp_audio_voice_stage(len-1, &msg[1]); break;
        default:                                break;    
    }
}

void qext_dsp_ipc_radio(const char *from, unsigned int len, unsigned char *msg)
{  
    la_log_debug("dsp radio cmd from %s, data 0x%02x 0x%02x", from, msg[0], msg[1]);

    if(qext_dsp_ipc_len_error(msg[0], len))
        return;

    memcpy(target, from, 6);
    
    switch(msg[0])
    {
        case QEXT_DSP_CMD_RADIO_STATUS:     qext_dsp_radio_status();                    break;
        case QEXT_DSP_CMD_RADIO_CHANGE:     qext_dsp_radio_change(len-1, &msg[1]);      break;
        case QEXT_DSP_CMD_RADIO_RUN_MODE:   qext_dsp_radio_mode(len-1, &msg[1]);        break;
        case QEXT_DSP_CMD_RADIO_SET_FREQ:   qext_dsp_radio_freq(len-1, &msg[1]);        break;
        case QEXT_DSP_CMD_RADIO_PRESET:     qext_dsp_radio_preset(len-1, &msg[1]);      break;
        default:                                                                        break;    
    }
    qext_dsp_radio_show_status();
}

void qext_dsp_ipc_send(unsigned char *msg, int len)
{
    la_ipc_send(target, msg, len);
}

