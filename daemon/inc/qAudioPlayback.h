#ifndef _QAUDIO_PLAYBACK_H
#define _QAUDIO_PLAYBACK_H

#include <stdint.h>
#include <sys/select.h>
#include <alsa/asoundlib.h>
#include "qAudioUtil.h"
#include "qAudioLog.h"


#define LA_PLAYBACK_FIFO_NAME     "/dev/fifo_lstsw"
#define LA_PLAYBACK_FIFO_NAME_MAX (24)
#define LA_PLAYBACK_PCM_NAME      "lstsw"
#define LA_PLAYBACK_PCM_NAME_MAX  (16)
#define LA_PLAYBACK_SOFTVOL       "lstdmix"
#define LA_PLAYBACK_SOFTVOL_MAX   (16)
#define LA_PLAYBACK_AP_NAME_MAX   (32)

#define LA_PLAYBACK_CARD_MAX      (5)

#define LA_PLAYBACK_CMD_SIZE      (64)


typedef struct _la_playback	la_playback_t;

typedef struct _la_playback_pcm	la_playback_pcm_t;

struct _la_playback_pcm{
    void*               card;
    snd_pcm_t*          handle;
	bool                start;
};

struct _la_playback
{
    int                 id;
	char                name[LA_PLAYBACK_AP_NAME_MAX];
	char                softvol[LA_PLAYBACK_SOFTVOL_MAX];
	
	int                 fd;
    char                fifo[LA_PLAYBACK_FIFO_NAME_MAX];
	
	int                 card_num;
    la_playback_pcm_t** card_list;
    la_list_t           node;

    la_thread_t*        thread;
    la_mutex_t*         mutex;
    la_mutex_t*         mutex_stream;
	char*               stream;
    int                 stream_len;
	int                 chunk;
    int                 master_panel;
	bool                master_mute;

};


la_errno_t la_playback_probe(void);
la_errno_t la_playback_new(int id, char* name, int master_panel, la_list_t* head);
void       la_playback_delet(void);

void la_playback_start(la_thread_t** thread);

void la_playback_update_chunk(int chunk);
la_errno_t la_playback_set_master_softvol(la_playback_t* playback, char* volume);

la_playback_t* la_playback_get_playback(int id);

la_errno_t la_playback_open_pcm(la_playback_t* playback, int card_id);
la_errno_t la_playback_close_pcm(la_playback_t* playback, int card_id);

void la_playback_write(la_playback_t* playback, snd_pcm_sframes_t frames);


#endif /* _QAUDIO_PLAYBACK_H */

