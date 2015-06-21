#ifndef _QAUDIO_CORE_H
#define _QAUDIO_CORE_H

#include <stdint.h>
#include <sys/select.h>
#include <alsa/asoundlib.h>
#include "qAudioUtil.h"
#include "qAudioLog.h"

#define LA_BUF_SIZE (255)
#define LA_CMD_SIZE (64)


// parse parameters in asound.conf 
#define LA_MASTER_CARD  "LST_MASTER_CARD="
#define LA_MASTER_LEN   (sizeof(LA_MASTER_CARD))
#define LA_DMIX_PCM     "dmix_"
#define LA_HW_PCM       "hw_"

la_errno_t la_init(void);
void la_deinit(void);

void la_show_pcm_structure(void);
bool la_more_pcm_info(void);

void la_set_max_fd(int fd);
int la_get_max_fd(void);
fd_set* la_get_fdset(void);

int la_get_panel_num(void);
int la_get_playback_num(void);
int la_get_capture_num(void);
int la_get_master_card(void);
la_list_t* la_get_capture_head(void);
la_list_t* la_get_panel_head(void);
la_list_t* la_get_playback_head(void);
bool la_is_running(void);
snd_output_t* la_get_output_log(void);

#endif /* _QAUDIO_CORE_H */

