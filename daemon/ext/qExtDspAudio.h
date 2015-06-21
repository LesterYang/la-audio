#ifndef _QEXT_DSP_AUDIO_H
#define _QEXT_DSP_AUDIO_H

#define QEXT_ADSP_DEFAULT_VOLUME                (5)

// ============================
// command header
// ============================
#define QEXT_DSP_CMD_AUDIO_STATUS		        (0x30)
#define QEXT_DSP_CMD_AUDIO_SOURCE		        (0x31)
#define	QEXT_DSP_CMD_AUDIO_MUTE			        (0x32)
#define	QEXT_DSP_CMD_AUDIO_VOLUME		        (0x33)
#define	QEXT_DSP_CMD_AUDIO_SPEED		        (0x34)
#define	QEXT_DSP_CMD_AUDIO_FAD_BAL		        (0x35)
#define	QEXT_DSP_CMD_AUDIO_BAS_MID_TRE	        (0x36)
#define	QEXT_DSP_CMD_AUDIO_LOUDNESS		        (0x37)
#define	QEXT_DSP_CMD_AUDIO_EQ			        (0x38)
#define	QEXT_DSP_CMD_AUDIO_VOICE_STAGE	        (0x39)

// ============================
// define data of command
// ============================
#define QEXT_DSP_AUDIO_USER_UNMUTE              (0x00)
#define QEXT_DSP_AUDIO_USER_MUTE                (0x01)
#define QEXT_DSP_AUDIO_USER_MUTE_TOGGLE         (0x02)
#define QEXT_DSP_AUDIO_APP_UNMUTE               (0x10)
#define QEXT_DSP_AUDIO_APP_MUTE                 (0x11)
#define QEXT_DSP_AUDIO_APP_MUTE_TOGGLE          (0x12)
#define QEXT_DSP_AUDIO_MUTE_DIRECT              (0x71)

#define QEXT_DSP_AUDIO_INC_VOL                  (0x80)
#define QEXT_DSP_AUDIO_DEC_VOL                  (0x81)
#define QEXT_DSP_AUDIO_MAX_VOL                  (32)

#define QEXT_DSP_AUDIO_IGN_FAD                  (127)
#define QEXT_DSP_AUDIO_INC_FAD                  (-128)
#define QEXT_DSP_AUDIO_DEC_FAD                  (-127)
#define QEXT_DSP_AUDIO_MIN_FAD                  (-7)
#define QEXT_DSP_AUDIO_MAX_FAD                  (7)
#define QEXT_DSP_AUDIO_IGN_BAL                  (127)
#define QEXT_DSP_AUDIO_INC_BAL                  (-128)
#define QEXT_DSP_AUDIO_DEC_BAL                  (-127)
#define QEXT_DSP_AUDIO_MIN_BAL                  (-7)
#define QEXT_DSP_AUDIO_MAX_BAL                  (7)

#define QEXT_DSP_AUDIO_IGN_BAS                  (127)
#define QEXT_DSP_AUDIO_INC_BAS                  (-128)
#define QEXT_DSP_AUDIO_DEC_BAS                  (-127)
#define QEXT_DSP_AUDIO_MIN_BAS                  (-7)
#define QEXT_DSP_AUDIO_MAX_BAS                  (7)
#define QEXT_DSP_AUDIO_IGN_MID                  (127)
#define QEXT_DSP_AUDIO_INC_MID                  (-128)
#define QEXT_DSP_AUDIO_DEC_MID                  (-127)
#define QEXT_DSP_AUDIO_MIN_MID                  (-7)
#define QEXT_DSP_AUDIO_MAX_MID                  (7)
#define QEXT_DSP_AUDIO_IGN_TRE                  (127)
#define QEXT_DSP_AUDIO_INC_TRE                  (-128)
#define QEXT_DSP_AUDIO_DEC_TRE                  (-127)
#define QEXT_DSP_AUDIO_MIN_TRE                  (-7)
#define QEXT_DSP_AUDIO_MAX_TRE                  (7)

#define QEXT_DSP_AUDIO_LOUDNESS_OFF             (0)
#define QEXT_DSP_AUDIO_LOUDNESS_ON              (1)
#define QEXT_DSP_AUDIO_LOUDNESS_TOGGLE          (2)

#define QEXT_DSP_AUDIO_NORMAL                   (0x00)
#define QEXT_DSP_AUDIO_JAZZ                     (0x01)
#define QEXT_DSP_AUDIO_VOLCAL                   (0x02)
#define QEXT_DSP_AUDIO_POP                      (0x03)
#define QEXT_DSP_AUDIO_CLASSIC                  (0x04)
#define QEXT_DSP_AUDIO_ROCK                     (0x05)
#define QEXT_DSP_AUDIO_USER                     (0x06)
#define QEXT_DSP_AUDIO_INC_EQ                   (0x80)
#define QEXT_DSP_AUDIO_DEC_EQ                   (0x81)



typedef enum _qext_dsp_audio_source qext_dsp_audio_source_t;


enum _qext_dsp_audio_source{
    QEXT_DSP_AS_RADIO        = 0x00,
    QEXT_DSP_AS_AUX          = 0x04,
    QEXT_DSP_AS_DTV          = 0x05,
    QEXT_DSP_AS_IPOD         = 0x06,
    QEXT_DSP_AS_MHL          = 0x07,
    QEXT_DSP_AS_NAVI         = 0x08,
    QEXT_DSP_AS_BT_HFP       = 0x10,
    QEXT_DSP_AS_BT_A2DP      = 0x11,
    QEXT_DSP_AS_DISC_CD      = 0x18,
    QEXT_DSP_AS_DISC_USB     = 0x19,
    QEXT_DSP_AS_DISC_SD      = 0x1A,
    QEXT_DSP_AS_DISC_CHANGER = 0x1B,
    QEXT_DSP_AS_ARM_USB      = 0x20,
    QEXT_DSP_AS_ARM_NAVI     = 0x21,
    QEXT_DSP_AS_ARM_BT_HFP   = 0x22,
    QEXT_DSP_AS_ARM_BT_A2DP  = 0x23,
    
    QEXT_DSP_AS_NULL         = 0xFF
};


void qext_dsp_audio_save(void);
void qext_dsp_audio_load(void);


void qext_dsp_audio_status(void);
void qext_dsp_audio_source(unsigned int len, unsigned char *msg);
void qext_dsp_audio_mute(unsigned int len, unsigned char *msg);
void qext_dsp_audio_volume(unsigned int len, unsigned char *msg);
void qext_dsp_audio_speed(unsigned int len, unsigned char *msg);
void qext_dsp_audio_fad_bal(unsigned int len, unsigned char *msg);
void qext_dsp_audio_bas_mid_tre(unsigned int len, unsigned char *msg);
void qext_dsp_audio_loudness(unsigned int len, unsigned char *msg);
void qext_dsp_audio_eq(unsigned int len, unsigned char *msg);
void qext_dsp_audio_voice_stage(unsigned int len, unsigned char *msg);

void qext_dsp_audio_show_status(void);


#endif /* _QEXT_DSP_AUDIO_H */ 
