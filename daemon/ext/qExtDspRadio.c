/*
 *  qExtDspRadio.c
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
#include "qAudioUtil.h"
#include "qExtDsp.h"
#include "qExtDspRadio.h"
#include "qDsp.h"

#define RADIO_BACKUP_SIZE   (sizeof(struct qext_dsp_radio))
#define RADIO_STATUS_LEN    (6)
#define RADIO_AST_LEN       (5)

// Band = TYPE(3 bits) + NUM(5 bits)
#define NUM_TO_IDX(num)             ((num) - 1)
#define IDX_TO_NUM(idx)             ((idx) + 1)
#define GET_BAND_IDX(band)          (((band)&0x1F) - 1)
#define GET_BAND_NUM(band)          ((band)&0x1F)
#define GET_BAND_TYPE(band)         (((band)&0xE0)>>5)

#define SET_BAND(num, type)         ((((num)&0x1F) | ((type)<<5)) & 0xFF)
#define SET_FUNC_STATE(run, stereo) ((((run)&0x7F) | ((stereo)<<7)) & 0xFF)
#define SET_CH_STATE(num, preset)   ((((num)&0x7F) | ((preset)<<7)) & 0xFF)

#define TOGGLE_TYPE(type) ((type == QEXT_DSP_RADIO_BAND_TYPE_FM) ? QEXT_DSP_RADIO_BAND_TYPE_AM : QEXT_DSP_RADIO_BAND_TYPE_FM)



typedef enum 
{
    QEXTDSP_RADIO_ACK_AST,       
    QEXTDSP_RADIO_ACK_PRESET
}qext_dsp_radio_ack_freq;


typedef struct _qext_dsp_radio_band_data{
    int curCh;
    short ch[QEXT_DSP_MAX_CHANNEL_NUM];
}qext_dsp_radio_band_data;

typedef struct _qext_dsp_radio_data{
    int curBand;
    qext_dsp_radio_band_data band[QEXT_DSP_MAX_BAND_NUM];
}qext_dsp_radio_data;

typedef struct _qext_dsp_radio_param{
    qext_dsp_radio_data* data;
    char band;
    char step;
    char  max_band_num;
    char  max_ch_num;
    short max_freq;
    short min_freq;
}qext_dsp_radio_param;


struct qext_dsp_radio{    
    char  stereo;
    char  runFunc;
    char  type;
    char  preset;
    short freq;
    
    qext_dsp_radio_data FM;
    qext_dsp_radio_data AM;

    qext_dsp_radio_param *param;
};

#define PARAM_NUM   (2)
#define FM_IDX      (0)      // an index of following variable (radio_param)
#define AM_IDX      (1)      // an index of following variable (radio_param)
static qext_dsp_radio_param radio_param[PARAM_NUM] = {
    {
        .band         = QEXT_DSP_RADIO_BAND_TYPE_FM,
        .max_band_num = QEXT_DSP_MAX_FM_BAND_NUM,
        .max_ch_num   = QEXT_DSP_MAX_FM_CH_NUM,
        .max_freq     = QEXT_DSP_FM_FREQ_MAX,
        .min_freq     = QEXT_DSP_FM_FREQ_MIN,
        .step         = QEXT_DSP_FM_FREQ_STEP
    },
    {
        .band         = QEXT_DSP_RADIO_BAND_TYPE_AM,
        .max_band_num = QEXT_DSP_MAX_FM_BAND_NUM,
        .max_ch_num   = QEXT_DSP_MAX_FM_CH_NUM,
        .max_freq     = QEXT_DSP_AM_FREQ_MAX,
        .min_freq     = QEXT_DSP_AM_FREQ_MIN,
        .step         = QEXT_DSP_AM_FREQ_STEP
    }
};



static struct qext_dsp_radio radio;

//Parse IPC sub-message
void qext_dsp_radio_change_band(unsigned int len, unsigned char *msg);
void qext_dsp_radio_change_channel(unsigned int len, unsigned char *msg);
void qext_dsp_radio_change_freq(unsigned int len, unsigned char *msg);
void qext_dsp_radio_preset_band(unsigned int len, unsigned char *msg);
void qext_dsp_radio_preset_channel(unsigned int len, unsigned char *msg);
void qext_dsp_radio_preset_read(unsigned int len, unsigned char *msg);

// Return IPC message
void qext_dsp_radio_ret_ast();
void qext_dsp_radio_ret_preset(qext_dsp_radio_ack_freq ack, char band_type, char band_num);



// Internal functions
void qext_dsp_radio_init_channel(void);
int  qext_dsp_radio_set_type(char type);

int  qext_dsp_radio_toggle_band();
int  qext_dsp_radio_set_band(unsigned char band);
void qext_dsp_radio_next_band(void);
void qext_dsp_radio_prev_band(void);
void qext_dsp_radio_switch_fm_band(void);
void qext_dsp_radio_switch_am_band(void);

int  qext_dsp_radio_set_channel(qext_dsp_radio_band_data* band_data, unsigned char idx);
void qext_dsp_radio_switch_channel(unsigned char idx);
void qext_dsp_radio_next_channel(void);
void qext_dsp_radio_prev_channel(void);

int  qext_dsp_radio_set_freq(short freq);
void qext_dsp_radio_next_freq(void);
void qext_dsp_radio_prev_freq(void);

int  qext_dsp_radio_check_freq(short val);
void qext_dsp_radio_check_preset(short freq);

int  qext_dsp_radio_write_preset(qext_dsp_radio_band_data* band_data, qext_dsp_radio_param* param, unsigned char *data, int count);

void qext_dsp_radio_get_stereo(void);
char qext_dsp_radio_get_cur_bandIdx(void);
char qext_dsp_radio_get_cur_chIdx(void);
char qext_dsp_radio_get_cur_bandNum(void);
char qext_dsp_radio_get_cur_chNum(void);
qext_dsp_radio_param* qext_dsp_radio_get_param(char band_type);
qext_dsp_radio_data* qext_dsp_radio_get_rdata(char band_type);
qext_dsp_radio_data* qext_dsp_radio_get_cur_rdata();
qext_dsp_radio_band_data* qext_dsp_radio_get_band_data(qext_dsp_radio_data* radio_data, int band_idx);

//Information functions
const char* qext_dsp_radio_str_band_type(char band);
const char* qext_dsp_radio_str_run_func(char run_func);

// Inline functions
inline static void qext_dsp_radio_enable_preset()
{
    radio.preset = 1;
}

inline static void qext_dsp_radio_disable_preset()
{
    radio.preset = 0;
}

inline static void _word_to_2uc(unsigned char* buf, short word)
{
    buf[0] = (unsigned char)((word>>8)  & 0xFF);
    buf[1] = (unsigned char)((word)     & 0xFF);
}

inline static short _2uc_to_word(unsigned char* buf)
{
    return (short)(buf[0]*256 + buf[1]);
}


/*********************************
  Extern DSP API for radio
**********************************/
void qext_dsp_radio_save()
{
    int fd;
    
    if(access(DSP_BACKUP_PATH, F_OK)!= 0)
    {
        mkdir(DSP_BACKUP_PATH, 0777); 
    }

    fd = open(RDSP_BACKUP, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if(fd > 0)
    {
        write(fd, (char*)&radio, RADIO_BACKUP_SIZE);
        sync();
    }
    else
        la_log_info("qext_dsp_radio_save() open error(%s)",strerror(errno));

    close(fd);
}

void qext_dsp_radio_load()
{
    int fd;
    
    qdsp_radio_load_align();
    fd = open(RDSP_BACKUP, O_RDONLY);

    if(fd < 0)
    {
        la_log_info("qext_dsp_radio_load() open error(%s)",strerror(errno));
        goto default_setting;
    }
    
    if(read(fd, (char*)&radio, RADIO_BACKUP_SIZE) == RADIO_BACKUP_SIZE)
    {
        qext_dsp_radio_set_type(radio.type);
        qext_dsp_radio_set_freq(radio.freq);
        qext_dsp_radio_status();
        close(fd);
        return;
    }
    else
        la_log_info("qext_dsp_audio_load() read error");

    close(fd);
    
default_setting:
    qext_dsp_radio_init_channel();
    qext_dsp_radio_set_type(QEXT_DSP_RADIO_BAND_TYPE_FM);
    qext_dsp_radio_set_freq(QEXT_DSP_RADIO_DEFAULT_FM_FREQ);
}

void qext_dsp_radio_show_status()
{
    qext_dsp_radio_get_stereo();
    la_log_debug("radio channel     -> %s ", (radio.stereo)?"Stereo":"Mono");
    la_log_debug("radio function    -> %s ", qext_dsp_radio_str_run_func(radio.runFunc));
    la_log_debug("radio band type   -> %s ", qext_dsp_radio_str_band_type(radio.type));
    la_log_debug("radio band num    -> %d ", qext_dsp_radio_get_cur_bandNum());
    la_log_debug("radio channel num -> %d ", qext_dsp_radio_get_cur_chNum());
    la_log_debug("radio preset      -> %s ", (radio.preset)?"Yes":"No");
    la_log_debug("radio frequency   -> %d ", radio.freq);
}

/*********************************
  Parse IPC message
**********************************/
void qext_dsp_radio_status()
{
    unsigned char buf[RADIO_STATUS_LEN] = {0};

    qext_dsp_radio_get_stereo();
    
    buf[0] = QEXT_DSP_CMD_RADIO_STATUS;
    buf[1] = SET_FUNC_STATE(radio.runFunc, radio.stereo);
    buf[2] = SET_BAND(qext_dsp_radio_get_cur_bandNum(), radio.type);
    buf[3] = SET_CH_STATE(qext_dsp_radio_get_cur_chNum(), radio.preset);
    _word_to_2uc(&buf[4], radio.freq);
      
    qext_dsp_ipc_send(buf, sizeof(buf));
}

void qext_dsp_radio_change(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_RADIO_CHANGE_BAND: qext_dsp_radio_change_band(len-1, &msg[1]);    break;       
        case QEXT_DSP_RADIO_CHANGE_CH:   qext_dsp_radio_change_channel(len-1, &msg[1]); break;     
        case QEXT_DSP_RADIO_CHANGE_FREQ: qext_dsp_radio_change_freq(len-1, &msg[1]);    break;   
        default:                         la_log_info("x_radio_change invalid massage"); break;
    }
    qext_dsp_radio_status();
}

void qext_dsp_radio_mode(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_RADIO_NEXT:                                                       break;
        case QEXT_DSP_RADIO_PREV:                                                       break;
        case QEXT_DSP_RADIO_FUNC_IDLE:  break;       
        case QEXT_DSP_RADIO_FUNC_SEEK:  break;     
        case QEXT_DSP_RADIO_FUNC_SCAN:  break;
        case QEXT_DSP_RADIO_FUNC_AST:   break;  
        case QEXT_DSP_RADIO_FUNC_FREQ:  break;  
        default:                        la_log_info("x_radio_mode invalid massage");    break;
    }
    qext_dsp_radio_status();
}

void qext_dsp_radio_freq(unsigned int len, unsigned char *msg)
{
    char type, band_num, ch_num;
    short freq;
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_param* param;

    type     = GET_BAND_TYPE(msg[0]);
    band_num = GET_BAND_NUM(msg[0]);
    ch_num   = msg[1];
    freq     = _2uc_to_word(&msg[2]);

    // 1. save band type
    qext_dsp_radio_set_type(type);

    // 2. save band number   
    rdata = qext_dsp_radio_get_rdata(type);
    param = qext_dsp_radio_get_param(type);

    if(rdata == NULL || param == NULL)
    {
        return;
    }

    if(band_num == 0 || band_num == 31)
    {
        band_num = qext_dsp_radio_get_cur_bandNum();
    }
    else if (band_num > param->max_band_num)
    {
        la_log_info("invalid band num %d", band_num);
        qext_dsp_radio_disable_preset();       
        //return;
    }    

    qext_dsp_radio_set_band(SET_BAND(band_num, type));

    // 3. save channel number  

    if(ch_num == 0)
    {
        ch_num = qext_dsp_radio_get_cur_chNum();
    }

    if (ch_num != 0xFF)
    {
        if(ch_num > param->max_ch_num)
        {
            la_log_info("invalid channel num %d", ch_num);
            qext_dsp_radio_disable_preset();       
        }
        else
        {
            qext_dsp_radio_band_data* band_data;
            band_data = qext_dsp_radio_get_band_data(rdata, NUM_TO_IDX(band_num));
            
            if(band_data == NULL)
                la_log_info("get band data error (band num : %d)", band_num);
            else
                band_data->curCh = NUM_TO_IDX(ch_num);
        }
    }

    // 4. save frequency
    if(qext_dsp_radio_set_freq(freq) == 0)
        qext_dsp_radio_check_preset(freq);

    // 6. ack
    qext_dsp_radio_ret_ast();
    qext_dsp_radio_status();
}

void qext_dsp_radio_preset(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_RADIO_PRESET_BAND_SHT: qext_dsp_radio_preset_band(len-1, &msg[1]);    break;       
        case QEXT_DSP_RADIO_PRESET_CH_SHT:   qext_dsp_radio_preset_channel(len-1, &msg[1]); break;     
        case QEXT_DSP_RADIO_PRESET_READ_SHT: qext_dsp_radio_preset_read(len-1, &msg[1]);    break;   
        default:                             la_log_info("x_radio_preset invalid massage"); break;
    }
}

/*********************************
  Parse IPC sub-message
**********************************/
void qext_dsp_radio_change_band(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_RADIO_NEXT:           qext_dsp_radio_next_band();         break;
        case QEXT_DSP_RADIO_PREV:           qext_dsp_radio_prev_band();         break;
        case QEXT_DSP_RADIO_CHANGE_BAND_FM: qext_dsp_radio_switch_fm_band();    break;
        case QEXT_DSP_RADIO_CHANGE_BAND_AM: qext_dsp_radio_switch_am_band();    break;
        default:                    
            qext_dsp_radio_set_band(SET_BAND(msg[0]&0xF, (msg[0]>>4)&0x1));
            break;
    }
}

void qext_dsp_radio_change_channel(unsigned int len, unsigned char *msg)
{
    switch(msg[0])
    {
        case QEXT_DSP_RADIO_NEXT:           qext_dsp_radio_next_channel();         break;
        case QEXT_DSP_RADIO_PREV:           qext_dsp_radio_prev_channel();         break;
        default:                            qext_dsp_radio_switch_channel(msg[0]); break;
    }
}

void qext_dsp_radio_change_freq(unsigned int len, unsigned char *msg)
{
    if(len == 2)
    {
        la_log_info("0x%x 0x%x",msg[1],msg[2]);
        qext_dsp_radio_set_freq(_2uc_to_word(&msg[1]));
        return;
    }

    switch(msg[0])
    {
        case QEXT_DSP_RADIO_NEXT:   qext_dsp_radio_next_freq();                         break;
        case QEXT_DSP_RADIO_PREV:   qext_dsp_radio_prev_freq();                         break;
        default:                    la_log_info("x_radio_change_freq invalid massage"); break;
    }
}

void qext_dsp_radio_preset_band(unsigned int len, unsigned char *msg)
{
    char type, band_num, count;
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_param* param;
    qext_dsp_radio_band_data* band_data;
    
    int pos = 2;

    type     = GET_BAND_TYPE(msg[0]);
    band_num = GET_BAND_NUM(msg[0]);
    count    = msg[1];

    rdata = qext_dsp_radio_get_rdata(type);
    param = qext_dsp_radio_get_param(type);
    
    if(len == 0 || rdata == NULL || param == NULL)
        return;
 
    if (band_num < 0 || band_num > 30)
        band_num = IDX_TO_NUM(rdata->curBand);

    if(band_num == 0)
    {
        int idx;
        
        for(idx=0; idx<param->max_band_num; idx++)
        {
            int done = 0;
            band_data = qext_dsp_radio_get_band_data(rdata, idx);

            if(band_data == NULL)
                continue;

            done = qext_dsp_radio_write_preset(band_data, param, &msg[pos], count);

            pos   += done*2;
            count -= done;

            if(count == 0)
                break;
        }
    }
    else
    {
        band_data = qext_dsp_radio_get_band_data(rdata, NUM_TO_IDX(band_num));

        if(band_data == NULL)
            return;

        qext_dsp_radio_write_preset(band_data, param, &msg[pos], count);
    }

    qext_dsp_radio_ret_preset(QEXTDSP_RADIO_ACK_PRESET, type, 0);
}

void qext_dsp_radio_preset_channel(unsigned int len, unsigned char *msg)
{
    char type, band_num, ch_num;
    short freq = -1;
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_param* param;
    qext_dsp_radio_band_data* band_data;

    type     = GET_BAND_TYPE(msg[0]);
    band_num = GET_BAND_NUM(msg[0]);
    ch_num   = msg[1];

    rdata = qext_dsp_radio_get_rdata(type);
    param = qext_dsp_radio_get_param(type);
    
    if(rdata == NULL || param == NULL)
        return;
 
    if (band_num < 1 || band_num > 30)
        band_num = IDX_TO_NUM(rdata->curBand);

    band_data = qext_dsp_radio_get_band_data(rdata, NUM_TO_IDX(band_num));

    if(band_data == NULL)
        return;

    if(len == 4)
        freq = _2uc_to_word(&msg[2]);


    if (ch_num < 1 || ch_num > param->max_ch_num)
    {
        if(ch_num == 0xFF)
            freq  = radio.freq;
            
        ch_num = IDX_TO_NUM(band_data->curCh);
    }

    if(freq < 0)
        return;

    band_data->ch[(int)ch_num] = freq;

    qext_dsp_radio_ret_preset(QEXTDSP_RADIO_ACK_PRESET, type, 0);
}

void qext_dsp_radio_preset_read(unsigned int len, unsigned char *msg)
{
    char type, band_num;

    type     = GET_BAND_TYPE(msg[0]);
    band_num = GET_BAND_NUM(msg[0]);

    if (band_num < 0 || band_num > 30)
        band_num = IDX_TO_NUM(qext_dsp_radio_get_cur_rdata()->curBand);

    qext_dsp_radio_ret_preset(QEXTDSP_RADIO_ACK_PRESET, type, band_num);
}


/*********************************
  Return IPC message
**********************************/
void qext_dsp_radio_ret_ast()
{
    unsigned char buf[RADIO_AST_LEN] = {0};
    
    buf[0] = QEXT_DSP_CMD_RADIO_AST_LOCK;
    buf[1] = SET_BAND(qext_dsp_radio_get_cur_bandNum(), radio.type);
    buf[2] = SET_CH_STATE(qext_dsp_radio_get_cur_chNum(), radio.preset);
    _word_to_2uc(&buf[3], radio.freq);

    qext_dsp_ipc_send(buf, sizeof(buf));
}

void qext_dsp_radio_ret_preset(qext_dsp_radio_ack_freq ack, char band_type, char band_num)
{
    unsigned char buf[QEXT_DSP_MAX_BUF_SIZE] = {0};
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_param* param;
    qext_dsp_radio_band_data* band_data;
    int count=0, pos=3;

    rdata = qext_dsp_radio_get_rdata(band_type);
    param = qext_dsp_radio_get_param(band_type);
        
    if(rdata == NULL || param == NULL)
    {
        la_log_warn("ack preset, wrong band type");
        return;
    }
    
    if((unsigned char)band_num == 0xFF)
    {
        band_num = IDX_TO_NUM(rdata->curBand);
    }
    else if(band_num < 0 || band_num > param->max_band_num)
    {
        return;
    }

    if(band_num == 0)
    {
        int num;

        count = 0;
        
        for(num=0; num<param->max_band_num; num++)
        {
            int ch_idx;
            band_data = qext_dsp_radio_get_band_data(rdata, NUM_TO_IDX(num));

            if(band_data == NULL)
                continue;

            for(ch_idx=0 ;ch_idx<param->max_ch_num; ch_idx++)
            {
                _word_to_2uc(&buf[pos], band_data->ch[ch_idx]);
                pos += 2;
                count++;
            } 
        }
    }
    else
    {
        int ch_idx;
        band_data = qext_dsp_radio_get_band_data(rdata, NUM_TO_IDX(band_num));

        if(band_data == NULL)
            return;

        for(ch_idx=0; ch_idx<param->max_ch_num; ch_idx++)
        {
            _word_to_2uc(&buf[pos], band_data->ch[ch_idx]);
            pos += 2;
            count++;
        }
    }

    buf[0] = QEXT_DSP_CMD_RADIO_PRESET;
    buf[1] = SET_BAND(band_num, band_type);
    buf[2] = count;

    qext_dsp_ipc_send(buf, count*2+3);
}


/*********************************
  Internal functions
**********************************/
void _qext_dsp_radio_init_channel(qext_dsp_radio_data* data, short freq)
{
    int band, ch;

    la_assert(data);
    
    for(band=0; band<QEXT_DSP_MAX_BAND_NUM; band++)
    {
        for(ch=0; ch<QEXT_DSP_MAX_CHANNEL_NUM; ch++)
        {
            data->band[band].ch[ch] = freq;
        }
    }
    radio.preset = (radio.freq == freq) ? 1 : 0;
}

void qext_dsp_radio_init_channel()
{
   _qext_dsp_radio_init_channel(&radio.FM, QEXT_DSP_RADIO_DEFAULT_FM_FREQ);
   _qext_dsp_radio_init_channel(&radio.AM, QEXT_DSP_RADIO_DEFAULT_AM_FREQ);
}

int qext_dsp_radio_set_type(char type)
{
    int idx;
    qdsp_radio_band band;

    if(radio.type == type)
        return 0;

    switch(type)
    {
        case QEXT_DSP_RADIO_BAND_TYPE_FM:
            band = QDSP_BAND_FM;
            idx  = FM_IDX; 
            break;
            
        case QEXT_DSP_RADIO_BAND_TYPE_AM:
            band = QDSP_BAND_AM;
            idx  = AM_IDX; 
            break;
            
        default:                                        
            return -1;
    }

    if(qdsp_radio_set_band(band) < 0)
        return -1;

    radio.param = &radio_param[idx];
    radio.type = radio.param->band;
    return 0;
}

int qext_dsp_radio_set_band(unsigned char band)
{
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_band_data* band_data;
    short freq;
    int band_idx = GET_BAND_IDX(band);

    if(qext_dsp_radio_set_type(GET_BAND_TYPE(band)) < 0)
        return -1;

    rdata  = qext_dsp_radio_get_cur_rdata();
    band_data = qext_dsp_radio_get_band_data(rdata, band_idx);

    if(band_data == NULL)
    {
        la_log_info("get band data error (band : 0x%02x)", band);
        return -1;
    }

    freq = band_data->ch[band_data->curCh];
    
    if(qext_dsp_radio_set_freq(freq) == 0)
        qext_dsp_radio_enable_preset();        
    
    rdata->curBand = band_idx;
    return 0;
}

void qext_dsp_radio_next_band()
{
    unsigned char band;
    char band_num = qext_dsp_radio_get_cur_bandNum();

    if(band_num + 1 > radio.param->max_band_num)
    {
        if(radio.type == QEXT_DSP_RADIO_BAND_TYPE_FM)
            band = SET_BAND(1, QEXT_DSP_RADIO_BAND_TYPE_FM);
        else
            band = SET_BAND(1, QEXT_DSP_RADIO_BAND_TYPE_AM);
    }
    else
        band = SET_BAND(band_num + 1, QEXT_DSP_RADIO_BAND_TYPE_FM);
    
    qext_dsp_radio_set_band(band);
}

void qext_dsp_radio_prev_band()
{
    unsigned char band;
    char band_num = qext_dsp_radio_get_cur_bandNum();

    if(band_num == 1)
    {    
        if(radio.type == QEXT_DSP_RADIO_BAND_TYPE_FM)
            band = SET_BAND(radio_param[FM_IDX].max_band_num, QEXT_DSP_RADIO_BAND_TYPE_AM);
        else
            band = SET_BAND(radio_param[AM_IDX].max_band_num, QEXT_DSP_RADIO_BAND_TYPE_FM);
    }
    else
        band = SET_BAND(band_num - 1, radio.type);
    
    qext_dsp_radio_set_band(band);
}

void qext_dsp_radio_switch_fm_band()
{
    qext_dsp_radio_data* rdata = qext_dsp_radio_get_rdata(QEXT_DSP_RADIO_BAND_TYPE_FM);

    if(rdata == NULL)
    {
        la_log_info("get FM data error");
        return;
    }
    
    qext_dsp_radio_set_band(SET_BAND(IDX_TO_NUM(rdata->curBand), QEXT_DSP_RADIO_BAND_TYPE_FM));
}

void qext_dsp_radio_switch_am_band()
{
    qext_dsp_radio_data* rdata = qext_dsp_radio_get_rdata(QEXT_DSP_RADIO_BAND_TYPE_AM);

    if(rdata == NULL)
    {
       la_log_info("get AM data error");
        return;
    }
    
    qext_dsp_radio_set_band(SET_BAND(IDX_TO_NUM(rdata->curBand), QEXT_DSP_RADIO_BAND_TYPE_AM));
}

int qext_dsp_radio_set_channel(qext_dsp_radio_band_data* band_data, unsigned char idx)
{
    short freq;
    
    if(idx >= radio.param->max_ch_num)
    {
        la_log_info("wrong index of channel (idx : %d) ", idx);
        return -1;
    }

    freq = band_data->ch[idx];
    
    if(qext_dsp_radio_set_freq(freq) < 0)
        return -1; 
    
    band_data->curCh = idx;
    qext_dsp_radio_enable_preset();
    
    return 0;
}

void qext_dsp_radio_switch_channel(unsigned char idx)
{
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_band_data* band_data;
    
    rdata = qext_dsp_radio_get_cur_rdata();
    band_data = qext_dsp_radio_get_band_data(rdata, rdata->curBand);

    if(band_data == NULL)
    {
        la_log_info("get band data error (band idx : %d)", rdata->curBand);
        return;
    }
    
    qext_dsp_radio_set_channel(band_data, idx);
}

void qext_dsp_radio_next_channel()
{
    char max = radio.param->max_ch_num;
    char idx = qext_dsp_radio_get_cur_chIdx();
    qext_dsp_radio_switch_channel((idx + max + 1) % max);
}

void qext_dsp_radio_prev_channel()
{
    char max = radio.param->max_ch_num;
    char idx = qext_dsp_radio_get_cur_chIdx();
    qext_dsp_radio_switch_channel((idx + max + 1) % max);
}

int qext_dsp_radio_set_freq(short freq)
{
    if(qext_dsp_radio_check_freq(freq) < 0)
        return -1;

    if(qdsp_radio_set_freq(freq) < 0)
    {
        la_log_warn("set frequency error");
        return -1;
    }
    
    radio.freq = freq;
    return 0;
}

void qext_dsp_radio_next_freq()
{
    short freq = radio.freq;

    freq += radio.param->step;
    
    if(freq > radio.param->max_freq)
        freq = radio.param->min_freq;

    qext_dsp_radio_set_freq(freq);
}

void qext_dsp_radio_prev_freq()
{
    short freq = radio.freq;
    
    freq -= radio.param->step;

    if(freq < radio.param->min_freq)
        freq = radio.param->max_freq;

    qext_dsp_radio_set_freq(freq);
}

int qext_dsp_radio_check_freq(short freq)
{  
    if(radio.param == NULL)
        return -1;
    
    if(freq < radio.param->min_freq || freq > radio.param->max_freq)
    {
        la_log_warn("invalid %s frequency, value=%d", qext_dsp_radio_str_band_type(radio.type), freq);
        return -1;
    }
    return 0;
}

void qext_dsp_radio_check_preset(short freq)
{
    int ch;
    qext_dsp_radio_data* rdata;
    qext_dsp_radio_band_data* band_data;
    
    rdata = qext_dsp_radio_get_cur_rdata();
    band_data = qext_dsp_radio_get_band_data(rdata, rdata->curBand);

    if(band_data == NULL)
        return;

    if(band_data->ch[band_data->curCh] == freq)
    {
        qext_dsp_radio_enable_preset(); 
        return;
    }

    for(ch=0; ch<radio.param->max_ch_num; ch++)
    {
        if(band_data->ch[ch] == freq)
        {
            band_data->curCh = ch;
            qext_dsp_radio_enable_preset(); 
            return;
        }
    }

    qext_dsp_radio_disable_preset();
}

int qext_dsp_radio_write_preset(qext_dsp_radio_band_data* band_data, qext_dsp_radio_param* param, unsigned char *data, int count)
{
    int ch_idx, done = 0;
    
    la_assert(band_data);

    if(count > param->max_ch_num)
        count = param->max_ch_num;

    for(ch_idx=0; ch_idx<param->max_ch_num; ch_idx++)
    {   
        band_data->ch[ch_idx] = _2uc_to_word(&data[2*ch_idx]);
        count--;
        done++;
        if(count == 0)
            break;
    }
    return done;
}


void qext_dsp_radio_get_stereo()
{
    radio.stereo = (qdsp_radio_is_stereo()) ? 1 : 0;
}

qext_dsp_radio_data* qext_dsp_radio_get_rdata(char band_type)
{ 
    if(band_type == QEXT_DSP_RADIO_BAND_TYPE_FM)
        return &radio.FM;
    else if(band_type == QEXT_DSP_RADIO_BAND_TYPE_AM)
        return &radio.AM;

    la_log_warn("get radio data with wrong band type");
    
    return NULL;
}

char qext_dsp_radio_get_cur_bandIdx()
{
    return qext_dsp_radio_get_cur_rdata()->curBand;
}

char qext_dsp_radio_get_cur_chIdx()
{
    qext_dsp_radio_data* rdata = qext_dsp_radio_get_cur_rdata();
    return rdata->band[rdata->curBand].curCh;
}

char qext_dsp_radio_get_cur_bandNum()
{
    return qext_dsp_radio_get_cur_bandIdx() + 1;
}

char qext_dsp_radio_get_cur_chNum()
{
    return qext_dsp_radio_get_cur_chIdx() + 1;
}


qext_dsp_radio_param* qext_dsp_radio_get_param(char band_type)
{ 
    if(band_type == QEXT_DSP_RADIO_BAND_TYPE_FM)
        return &radio_param[FM_IDX];
    else if(band_type == QEXT_DSP_RADIO_BAND_TYPE_AM)
        return &radio_param[AM_IDX];

    la_log_warn("get radio param with wrong band type");
    
    return NULL;
}

qext_dsp_radio_data* qext_dsp_radio_get_cur_rdata()
{
    qext_dsp_radio_data* rdata = qext_dsp_radio_get_rdata(radio.type);

    if(rdata == NULL)
    {
        qext_dsp_radio_set_type(QEXT_DSP_RADIO_BAND_TYPE_FM);
        rdata = &radio.FM;
    }
    
    return rdata;
}

qext_dsp_radio_band_data* qext_dsp_radio_get_band_data(qext_dsp_radio_data* radio_data, int band_idx)
{ 
    la_assert(radio_data);
    la_assert(radio.param);
    
    if (band_idx >= radio.param->max_band_num || band_idx < 0)
        return NULL;
    
    return &radio_data->band[band_idx];
}

/*********************************
  Information functions
**********************************/
const char* qext_dsp_radio_str_band_type(char band)
{
    switch(band)
    {
        case QEXT_DSP_RADIO_BAND_TYPE_FM:   return "FM"; 
        case QEXT_DSP_RADIO_BAND_TYPE_AM:   return "AM";
        default:                            break;
    }
    return "unknown";
}

const char* qext_dsp_radio_str_run_func(char run_func)
{
    switch(run_func)
    {
        case QEXT_DSP_RADIO_FUNC_IDLE:  return "IDLE"; 
        case QEXT_DSP_RADIO_FUNC_SEEK:  return "SEEK"; 
        case QEXT_DSP_RADIO_FUNC_SCAN:  return "SCAN"; 
        case QEXT_DSP_RADIO_FUNC_AST:   return "AST"; 
        case QEXT_DSP_RADIO_FUNC_FREQ:  return "CHANGE FREQ"; 
        default:                        break;
    }
    return "unknown";
}
