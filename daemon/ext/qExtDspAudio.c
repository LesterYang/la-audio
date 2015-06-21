/*
 *  qExtDspAudio.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "qAudioLog.h"
#include "qExtDsp.h"
#include "qExtDspAudio.h"
#include "qDsp.h"

#define AUDIO_BACKUP_SIZE (sizeof(struct qext_dsp_audio))

struct qext_dsp_audio{
    char currSource;
    char prevSource;
    char backupSource;
    char mute;
    char vol;
    char speedMode;
    char fade;
    char balance;
    char bass;
    char middle;
    char treble;
    char loudness;
    char eq;
    char voiceStage;
};

static struct qext_dsp_audio audio;


void qext_dsp_audio_ret_source();
void qext_dsp_audio_ret_mute();
void qext_dsp_audio_ret_volume();
void qext_dsp_audio_ret_speed();
void qext_dsp_audio_ret_fad_bal();
void qext_dsp_audio_ret_bas_mid_tre();
void qext_dsp_audio_ret_loudness();
void qext_dsp_audio_ret_eq();
void qext_dsp_audio_ret_voice_stage();


void qext_dsp_audio_save()
{
    int fd;
    
    if(access(DSP_BACKUP_PATH, F_OK)!= 0)
    {
        mkdir(DSP_BACKUP_PATH, 0777); 
    }

    fd = open(ADSP_BACKUP, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if(fd > 0)
    {
        write(fd, (char*)&audio, AUDIO_BACKUP_SIZE);
        sync();
    }
    else
        la_log_info("qext_dsp_audio_save() open error(%s)",strerror(errno));

    close(fd);
}

void qext_dsp_audio_load()
{
    int fd = open(ADSP_BACKUP, O_RDONLY);

    if(fd < 0)
    {
        la_log_info("qext_dsp_audio_load() open error(%s)",strerror(errno));
        goto default_setting;
    }
    
    if(read(fd, (char*)&audio, AUDIO_BACKUP_SIZE) == AUDIO_BACKUP_SIZE)
    {
        qdsp_audio_unmute();
        audio.mute = 0;
        
        qdsp_audio_set_vol((audio.vol * 100)/32);
        qdsp_audio_set_balance(audio.balance);
        qdsp_audio_set_fade(audio.fade);
        qdsp_audio_set_bass(audio.bass);
        qdsp_audio_set_middle(audio.middle);
        qdsp_audio_set_treble(audio.treble);
     
        qext_dsp_audio_status();
        close(fd);
        return;
    }
    else
        la_log_info("qext_dsp_audio_load() read error");

    close(fd);
    
default_setting:
    audio.mute = 0;
    audio.vol = QEXT_ADSP_DEFAULT_VOLUME;
    qdsp_audio_set_vol((audio.vol * 100)/32);
    qdsp_audio_unmute();
}

void qext_dsp_audio_status()
{
    unsigned char buf[AUDIO_BACKUP_SIZE + 1] = {0};
    
    buf[0] = QEXT_DSP_CMD_AUDIO_STATUS;
    memcpy(buf+1, (char*)&audio, AUDIO_BACKUP_SIZE);
    
    qext_dsp_ipc_send(buf, sizeof(buf));
}

void qext_dsp_audio_ret_source()
{
    unsigned char source[4];

    source[0] = QEXT_DSP_CMD_AUDIO_SOURCE;
    source[1] = audio.currSource;
    source[2] = audio.prevSource;
    source[3] = audio.backupSource;

    qext_dsp_ipc_send(source, sizeof(source));
}

void qext_dsp_audio_ret_mute()
{
    unsigned char mute[2];

    mute[0] = QEXT_DSP_CMD_AUDIO_MUTE;
    mute[1] = (unsigned char)audio.mute;
    
    qext_dsp_ipc_send(mute, sizeof(mute));
}

void qext_dsp_audio_ret_volume()
{
    unsigned char volume[2];

    volume[0] = QEXT_DSP_CMD_AUDIO_VOLUME;
    volume[1] = (unsigned char)audio.vol;
    
    qext_dsp_ipc_send(volume, sizeof(volume));
}

void qext_dsp_audio_ret_speed()
{
    unsigned char speed[2];

    speed[0] = QEXT_DSP_CMD_AUDIO_SPEED;
    speed[1] = (unsigned char)audio.mute;
    
    qext_dsp_ipc_send(speed, sizeof(speed));
}

void qext_dsp_audio_ret_fad_bal()
{
    unsigned char fad_bal[3];

    fad_bal[0] = QEXT_DSP_CMD_AUDIO_FAD_BAL;
    fad_bal[1] = audio.fade;
    fad_bal[2] = audio.balance;

    qext_dsp_ipc_send(fad_bal, sizeof(fad_bal));
}

void qext_dsp_audio_ret_bas_mid_tre()
{
    unsigned char bas_mid_tre[4];
    
    bas_mid_tre[0] = QEXT_DSP_CMD_AUDIO_BAS_MID_TRE;
    bas_mid_tre[1] = audio.bass;
    bas_mid_tre[2] = audio.middle;
    bas_mid_tre[3] = audio.treble;

    qext_dsp_ipc_send(bas_mid_tre, sizeof(bas_mid_tre));
}

void qext_dsp_audio_ret_loudness()
{
}

void qext_dsp_audio_ret_eq()
{
}

void qext_dsp_audio_ret_voice_stage()
{
}

void qext_dsp_audio_source(unsigned int len, unsigned char *msg)
{
    int ret = 0;
    short mute_off_time = 0, mute_on_time = 0; 
    qext_dsp_audio_source_t src;

    src = msg[0];

    if(len == 5)
    {
        mute_off_time = msg[1]*256 + msg[2];
        mute_on_time  = msg[3]*256 + msg[4];
    }
    
    
    if(mute_off_time)
    {
        // save current volume
        // decrease volume
        la_log_debug("not support fade out");
    }


    switch(src)
    {
        case QEXT_DSP_AS_RADIO:         
            ret = qdsp_audio_switch_source(QDSP_CH_RADIO);      
            break;
            
        case QEXT_DSP_AS_AUX:           
            ret = qdsp_audio_switch_source(QDSP_CH_AUXIN);      
            break;
            
        case QEXT_DSP_AS_DTV:           
            ret = qdsp_audio_switch_source(QDSP_CH_DTV);        
            break;
            
        case QEXT_DSP_AS_IPOD:
        case QEXT_DSP_AS_MHL:           
            break;
            
        case QEXT_DSP_AS_NAVI:          
            ret = qdsp_audio_switch_source(QDSP_CH_NAVI);       
            break;
            
        case QEXT_DSP_AS_BT_HFP:
        case QEXT_DSP_AS_BT_A2DP:       
            break;
            
        case QEXT_DSP_AS_DISC_CD:
        case QEXT_DSP_AS_DISC_USB:
        case QEXT_DSP_AS_DISC_SD:
        case QEXT_DSP_AS_DISC_CHANGER:  
            ret = qdsp_audio_switch_source(QDSP_CH_DVD);        
            break;
            
        case QEXT_DSP_AS_ARM_USB:
        case QEXT_DSP_AS_ARM_NAVI:
        case QEXT_DSP_AS_ARM_BT_HFP:
        case QEXT_DSP_AS_ARM_BT_A2DP:   
            ret = qdsp_audio_switch_source(QDSP_CH_USB);        
            break;
            
        default:                                                                            
            break;
    }

    if(ret == 0)
    {
        audio.prevSource = audio.currSource;
        audio.currSource = (char)src;
    }
    else
        la_log_notice("switch audio source error");

 
    if(mute_on_time)
    {
        // increase volume to saved volume
        la_log_debug("not support fade in");
    }  

    qext_dsp_audio_ret_source();
}

void qext_dsp_audio_mute(unsigned int len, unsigned char *msg)
{
    int ret = -1;
    short mutetime = 0;
    
    char processor  = (!!(audio.mute & 0x80));
    char app        = (!!(audio.mute & 0x02));
    char user       = (!!(audio.mute & 0x01));

    if(len == 3)
    {
        mutetime = msg[1]*256 + msg[2];

        // decrease/increase volume
        la_log_debug("not support fade in/out (%d)", mutetime);
    }

    switch(msg[0])
    {
        case QEXT_DSP_AUDIO_USER_MUTE:          user = 1;           break;
        case QEXT_DSP_AUDIO_USER_UNMUTE:        user = 0;           break;
        case QEXT_DSP_AUDIO_USER_MUTE_TOGGLE:   user = (user)?0:1;  break;
        case QEXT_DSP_AUDIO_APP_MUTE:           app = 1;            break;
        case QEXT_DSP_AUDIO_APP_UNMUTE:         app = 0;            break;
        case QEXT_DSP_AUDIO_APP_MUTE_TOGGLE:    app = (app)?0:1;    break;
        case QEXT_DSP_AUDIO_MUTE_DIRECT:        
            la_log_warn("not support mute on driectly");
            break;
        default:                                                    
            break;
    }

    if(app != (!!(audio.mute & 0x02)))
    {
        ret = (app) ? qdsp_audio_mute() : qdsp_audio_unmute();
    }

    if(user != (!!(audio.mute & 0x01)))
    {
        ret = (user) ? qdsp_audio_mute() : qdsp_audio_unmute();
    }

    if(ret == 0)
        audio.mute = user | app << 1 | processor << 7;
    else
        la_log_notice("set mute/unmute error");

    qext_dsp_audio_ret_mute();
}

void qext_dsp_audio_volume(unsigned int len, unsigned char *msg)
{
    int vol = audio.vol;

    if(vol == msg[0])
        return;

    switch(msg[0])
    {
        case QEXT_DSP_AUDIO_INC_VOL:    vol++;          break;
        case QEXT_DSP_AUDIO_DEC_VOL:    vol--;          break;
        default:                        vol = msg[0];   break;
    }

    if(vol > QEXT_DSP_AUDIO_MAX_VOL)
    {
        la_log_notice("invalid vaule of volume");
        return;
    }

    if(qdsp_audio_set_vol((vol * 100)/32) == 0)
        audio.vol = vol;
    else
        la_log_notice("set volume error");

    qext_dsp_audio_ret_volume();
}

void qext_dsp_audio_speed(unsigned int len, unsigned char *msg)
{
    qext_dsp_audio_ret_speed();
}

void qext_dsp_audio_fad_bal(unsigned int len, unsigned char *msg)
{
    short fade = audio.fade, balance = audio.balance;

    switch((signed char)msg[0])
    {
        case QEXT_DSP_AUDIO_IGN_FAD:                                break;
        case QEXT_DSP_AUDIO_INC_FAD:    fade++;                     break;
        case QEXT_DSP_AUDIO_DEC_FAD:    fade--;                     break;
        default:                        fade = (signed char)msg[0]; break;
    }

    if(fade < QEXT_DSP_AUDIO_MIN_FAD || fade > QEXT_DSP_AUDIO_MAX_FAD)
    {
        la_log_notice("invalid value of fade");
    }
    else if(fade != audio.fade)
    {
        if(qdsp_audio_set_fade(fade) == 0)
            audio.fade = fade;
        else
            la_log_notice("set fade error");
    }

    switch((signed char)msg[1])
    {
        case QEXT_DSP_AUDIO_IGN_BAL:                                    break;
        case QEXT_DSP_AUDIO_INC_BAL:    balance++;                      break;
        case QEXT_DSP_AUDIO_DEC_BAL:    balance--;                      break;
        default:                        balance = (signed char)msg[1];  break;
    }
    
    if(balance < QEXT_DSP_AUDIO_MIN_BAL || balance > QEXT_DSP_AUDIO_MAX_BAL)
    {
        la_log_info("invalid value of balance");
    }
    else if(balance != audio.balance)
    {

        if(qdsp_audio_set_balance(balance) == 0)
            audio.balance = balance;
        else
            la_log_notice("set balance error");
    }

    qext_dsp_audio_ret_fad_bal();
}

void qext_dsp_audio_bas_mid_tre(unsigned int len, unsigned char *msg)
{ 
    short bass = audio.bass, middle = audio.middle, treble = audio.treble;

    switch((signed char)msg[0])
    {
        case QEXT_DSP_AUDIO_IGN_BAS:                                break;
        case QEXT_DSP_AUDIO_INC_BAS:    bass++;                     break;
        case QEXT_DSP_AUDIO_DEC_BAS:    bass--;                     break;
        default:                        bass=(signed char)msg[0];   break;
    }

    if(bass < QEXT_DSP_AUDIO_MIN_BAS || bass > QEXT_DSP_AUDIO_MAX_BAS)
    {
        la_log_notice("invalid value of bass");
    }
    else if(bass != audio.bass)
    {
        if(qdsp_audio_set_bass(bass) == 0)
            audio.bass = bass;
        else
            la_log_notice("set bass error");
    }

    switch((signed char)msg[1])
    {
        case QEXT_DSP_AUDIO_IGN_MID:                                break;
        case QEXT_DSP_AUDIO_INC_MID:    middle++;                   break;
        case QEXT_DSP_AUDIO_DEC_MID:    middle--;                   break;
        default:                        middle=(signed char)msg[1]; break;
    }
    
    if(middle < QEXT_DSP_AUDIO_MIN_MID || middle > QEXT_DSP_AUDIO_MAX_MID)
    {
        la_log_notice("invalid value of middle");
    }
    else if( middle != audio.middle)
    {
        if(qdsp_audio_set_middle(middle) == 0)
            audio.middle = middle;
        else
            la_log_notice("set middle error");
    }

    switch((signed char)msg[2])
    {
        case QEXT_DSP_AUDIO_IGN_TRE:                                break;
        case QEXT_DSP_AUDIO_INC_TRE:    treble++;                   break;
        case QEXT_DSP_AUDIO_DEC_TRE:    treble--;                   break;
        default:                        treble=(signed char)msg[2]; break;
    }
    
    if(treble < QEXT_DSP_AUDIO_MIN_TRE || treble > QEXT_DSP_AUDIO_MAX_TRE)
    {
        la_log_notice("invalid value of treble");
    }
    else if(treble != audio.treble)
    {
        if(qdsp_audio_set_treble(treble) == 0)
            audio.treble = treble;
        else
           la_log_notice("set treble error"); 
    }

    qext_dsp_audio_ret_bas_mid_tre();
}

void qext_dsp_audio_loudness(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_AUDIO_LOUDNESS_OFF:       break;
        case QEXT_DSP_AUDIO_LOUDNESS_ON:        break;
        case QEXT_DSP_AUDIO_LOUDNESS_TOGGLE:    break;
        default:                                break;
    }

    qext_dsp_audio_ret_loudness();
}

void qext_dsp_audio_eq(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_AUDIO_NORMAL:     break;
        case QEXT_DSP_AUDIO_JAZZ:       break;
        case QEXT_DSP_AUDIO_VOLCAL:     break;
        case QEXT_DSP_AUDIO_POP:        break;
        case QEXT_DSP_AUDIO_CLASSIC:    break;
        case QEXT_DSP_AUDIO_ROCK:       break;
        case QEXT_DSP_AUDIO_USER:       break;
        case QEXT_DSP_AUDIO_INC_EQ:     break;
        case QEXT_DSP_AUDIO_DEC_EQ:     break;
        default:                        break;
    }

    qext_dsp_audio_ret_eq();
}

void qext_dsp_audio_voice_stage(unsigned int len, unsigned char *msg)
{
    qext_dsp_audio_ret_voice_stage();
}

#if 0
const char* qext_dsp_audio_str_source(char source)
{
    switch(source)
    {
        case QEXT_DSP_AS_RADIO:         return "Radio";
        case QEXT_DSP_AS_AUX:           return "Aux";
        case QEXT_DSP_AS_DTV:           return "DTV";
        case QEXT_DSP_AS_IPOD:          return "iPod";
        case QEXT_DSP_AS_MHL:           return "MHL";
        case QEXT_DSP_AS_NAVI:          return "Navi";
        case QEXT_DSP_AS_BT_HFP:        return "BT HFP";
        case QEXT_DSP_AS_BT_A2DP:       return "BT A2DP";
        case QEXT_DSP_AS_DISC_CD:       return "DISC CD";
        case QEXT_DSP_AS_DISC_USB:      return "DISC USB";
        case QEXT_DSP_AS_DISC_SD:       return "DISC SD";
        case QEXT_DSP_AS_DISC_CHANGER:  return "DISC CHANGER";
        case QEXT_DSP_AS_ARM_USB:       return "ARM USB";
        case QEXT_DSP_AS_ARM_NAVI:      return "ARM Navi";
        case QEXT_DSP_AS_ARM_BT_HFP:    return "ARM BT HFP";
        case QEXT_DSP_AS_ARM_BT_A2DP:   return "ARM BT A2DP";
        default:                        break;
    }
    return "unknown";
}


void qext_dsp_audio_show_status()
{
    struct qext_dsp_audio{
        char currSource;
        char prevSource;
        char backupSource;
        char mute;
        char vol;
        char speedMode;
        char fade;
        char balance;
        char bass;
        char middle;
        char treble;
        char loudness;
        char eq;
        char voiceStage;
    };
    
    static struct qext_dsp_audio audio;

    la_log_debug("audio channel     -> %s ",    (radio.stereo)?"Stereo":"Mono");
    la_log_debug("audio function    -> %s ",    qext_dsp_radio_str_run_func(radio.runFunc));
    la_log_debug("audio band type   -> %s ",    qext_dsp_radio_str_band_type(radio.bandType));
    la_log_debug("audio band num    -> 0x%0x ", radio.bandNum);
    la_log_debug("audio channel num -> %d ",    radio.chNum);
    la_log_debug("audio preset      -> %s ",    (radio.preset)?"Yes":"No");
    la_log_debug("audio frequency   -> %d ",    radio.freq);
}
#endif
