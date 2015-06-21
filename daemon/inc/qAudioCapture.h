#ifndef _QAUDIO_CAPTURE_H
#define _QAUDIO_CAPTURE_H

#include <alsa/asoundlib.h>
#include "qAudioUtil.h"
#include "qAudioLog.h"
#include "qAudioCard.h"

typedef struct _la_capture	la_capture_t;

#define LA_CAPTURE_PCM_NAME       "lstrc"
#define LA_CAPTURE_PCM_NAME_MAX   (16)
#define LA_CAPTURE_SRC_NAME_MAX   (24)
#define LA_CAPTURE_PIPE_NAME       "play_sw"


#define LA_CAPTURE_UDELAY         (25000)
#define LA_CAPTURE_DELAY          (1)


#define LA_CAPTURE_CARD_MAX       (5)
#define LA_CAPTURE_CMD_SIZE       (64)

typedef struct _la_capture	la_capture_t;

struct _la_capture
{
    int                 id;
    int                 hw_id;
    int                 pipe_id;
	char                name[LA_CAPTURE_SRC_NAME_MAX];
    char                pcm[LA_CAPTURE_PCM_NAME_MAX];

    int                 pipe_fd;

	la_list_t           node;

    snd_pcm_t*          handle;
    snd_pcm_t*          pipe_handle;
    bool                pipe_start;
	char*               stream;
    int                 stream_len;
	int                 chunk;
    la_card_param_t     param;
	volatile bool       connect;
    la_thread_t*	    thread;
    la_mutex_t*         mutex;

    void (*recv_cb)(unsigned char* buf, int len);
};

la_errno_t la_capture_probe(void);
la_errno_t la_capture_new(int id, int hw_id, int pipe_id, char* name, la_list_t* head);
void       la_capture_delet(void);

la_capture_t* la_capture_get_capture(int id);
la_capture_t* la_capture_get_capture_playback(int playback_id);
la_errno_t la_capture_start(la_capture_t* capture);
la_errno_t la_capture_stop(la_capture_t* capture);


#endif /* _QAUDIO_CAPTURE_H */

