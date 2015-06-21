#ifndef _QEXT_DSP_RADIO_H
#define _QEXT_DSP_RADIO_H

#define QEXT_DSP_MAX_BAND_NUM           (30)
#define QEXT_DSP_MAX_CHANNEL_NUM        (6)
#define QEXT_DSP_MAX_BUF_SIZE           (QEXT_DSP_MAX_BAND_NUM * QEXT_DSP_MAX_CHANNEL_NUM + 16)

#define QEXT_DSP_MAX_FM_BAND_NUM        (3)
#define QEXT_DSP_MAX_AM_BAND_NUM        (3)
#define QEXT_DSP_MAX_FM_CH_NUM          (QEXT_DSP_MAX_CHANNEL_NUM)
#define QEXT_DSP_MAX_AM_CH_NUM          (QEXT_DSP_MAX_CHANNEL_NUM)

#define QEXT_DSP_AM_FREQ_MAX            (1620)
#define QEXT_DSP_AM_FREQ_MIN            (522)
#define QEXT_DSP_AM_FREQ_STEP           (9)

#define QEXT_DSP_FM_FREQ_MAX            (10800)
#define QEXT_DSP_FM_FREQ_MIN            (8750)
#define QEXT_DSP_FM_FREQ_STEP           (10)


// ============================
// command header
// ============================
#define	QEXT_DSP_CMD_RADIO_STATUS		(0xA0)
#define	QEXT_DSP_CMD_RADIO_CHANGE		(0xA1)
#define	QEXT_DSP_CMD_RADIO_AST_LOCK		(0xA2)
#define	QEXT_DSP_CMD_RADIO_RUN_MODE		(0xA2)
#define	QEXT_DSP_CMD_RADIO_SET_FREQ		(0xA3)
#define	QEXT_DSP_CMD_RADIO_PRESET		(0xAE)

// ============================
// define data of command
// ============================
#define QEXT_DSP_RADIO_BAND_TYPE_FM     (0x01)
#define QEXT_DSP_RADIO_BAND_TYPE_AM     (0x02)

#define QEXT_DSP_RADIO_NEXT             (0x80U)
#define QEXT_DSP_RADIO_PREV             (0x81U)

#define QEXT_DSP_RADIO_CHANGE_BAND      (0x01)
#define QEXT_DSP_RADIO_CHANGE_CH        (0x02)
#define QEXT_DSP_RADIO_CHANGE_FREQ      (0x03)
#define QEXT_DSP_RADIO_CHANGE_BAND_FM   (0xA1)
#define QEXT_DSP_RADIO_CHANGE_BAND_AM   (0xA2)

#define QEXT_DSP_RADIO_FUNC_IDLE        (0x01) 
#define QEXT_DSP_RADIO_FUNC_SEEK        (0x02)
#define QEXT_DSP_RADIO_FUNC_SCAN        (0x03)
#define QEXT_DSP_RADIO_FUNC_AST         (0x04)
#define QEXT_DSP_RADIO_FUNC_FREQ        (0x05)

#define QEXT_DSP_RADIO_PRESET_BAND_SHT  (0x0)
#define QEXT_DSP_RADIO_PRESET_CH_SHT    (0x2)
#define QEXT_DSP_RADIO_PRESET_READ_SHT  (0x4)


#define QEXT_DSP_RADIO_DEFAULT_FM_FREQ  (8750)  // 87.5 MHz
#define QEXT_DSP_RADIO_DEFAULT_AM_FREQ  (522)   // 522 kHz


void qext_dsp_radio_save(void);
void qext_dsp_radio_load(void);
void qext_dsp_radio_show_status(void);

void qext_dsp_radio_status(void);
void qext_dsp_radio_change(unsigned int len, unsigned char *msg);
void qext_dsp_radio_mode(unsigned int len, unsigned char *msg);
void qext_dsp_radio_freq(unsigned int len, unsigned char *msg);
void qext_dsp_radio_preset(unsigned int len, unsigned char *msg);

#endif /* _QEXT_DSP_RADIO_H */
