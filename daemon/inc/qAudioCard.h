#ifndef _QAUDIO_CARD_H
#define _QAUDIO_CARD_H

#include <stdint.h>
#include <sys/select.h>
#include <alsa/asoundlib.h>
#include "qAudioUtil.h"
#include "qAudioLog.h"


#define LA_CARD_PCM_NAME     "lstaudio"
#define LA_CARD_PCM_NAME_MAX (16)
#define LA_CARD_PANEL_NAME   (32)

#define LA_CARD_PLAYBACK_MAX (16)
#define LA_CARD_CAPTURE_MAX  (8)

#define LA_CARD_PLAYBACK             (SND_PCM_STREAM_PLAYBACK)
#define LA_CARD_CAPTURE              (SND_PCM_STREAM_CAPTURE)

#define LA_CARD_FORMAT               (SND_PCM_FORMAT_S16_LE)
#define LA_CARD_ACCESS               (SND_PCM_ACCESS_RW_INTERLEAVED)
#define LA_CARD_CHANNELS             (2)
#define LA_CARD_RATE                 (48000)
#define LA_CARD_SOFT_RESAMPLE        (1)
#define LA_CARD_LATENCY              (100000)
#define LA_CARD_BUFFER_SIZE          (8192)
#define LA_CARD_PERIOD_SIZE          (2048)
#define LA_CARD_FORMAT_BITS          (16)
#define LA_CARD_FRAME_BYTES          ((LA_CARD_CHANNELS * LA_CARD_FORMAT_BITS) / 8)
#define LA_CARD_SEC_BYTES            (LA_CARD_RATE * LA_CARD_FRAME_BYTES)

#define LA_CARD_DIR_EXACT            (0)
#define LA_CARD_DIR_NEAR_GREATER     (1)
#define LA_CARD_DIR_NEAR_LESS        (-1)

#define byte_to_frame(bytes)    	 (bytes/LA_CARD_FRAME_BYTES)
#define frame_to_byte(frames)        (frames*LA_CARD_FRAME_BYTES)

typedef struct _la_card_param	la_card_param_t;
typedef struct _la_card	la_card_t;


struct _la_card_param
{
    snd_pcm_stream_t        stream;
    
    snd_pcm_format_t        format;
    unsigned int            channels;
    unsigned int            rate;
    snd_pcm_access_t        access;

    snd_pcm_uframes_t       buffer_size;
    snd_pcm_uframes_t       period_size;

    int                     monotonic;
    int                     can_pause;

    snd_pcm_channel_area_t  area;


    size_t bits_per_sample;
    size_t bytes_per_frame;
    size_t chunk_bytes;
};


struct _la_card
{
    int                 id;
	char                name[LA_CARD_PANEL_NAME];
    char                pcm[LA_CARD_PCM_NAME_MAX];
	
	int  				playback_num;
	int  				capture_num;
    void*               playback_list[LA_CARD_PLAYBACK_MAX];
	void*               capture_list[LA_CARD_CAPTURE_MAX];
	la_list_t           node;

	la_card_param_t     param;
    int             	delay;
};


la_errno_t la_card_probe(void);
la_errno_t la_card_new(int id, char* name, la_list_t* head);
void       la_card_delet(void);

void la_card_append_pcm(int card_id, int playback_id);
void la_card_clear_pcm(int card_id);
void la_card_set_pcm(int card_id, int playback_id);
void la_card_remove_pcm(int card_id, int playback_id);
void la_card_start_record(int id);
void la_card_stop_record(int id);

la_card_t* la_card_get_card(int id);

void la_card_set_params(snd_pcm_t* handle, la_card_param_t* param);
int la_card_get_hw_by_name(const char* name);

#endif /* _QAUDIO_CARD_H */

