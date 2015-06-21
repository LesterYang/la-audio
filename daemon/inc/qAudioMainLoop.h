#ifndef _QAUDIO_MAIN_LOOP_H
#define _QAUDIO_MAIN_LOOP_H

#ifndef QAUDIO_VERSION
#define QAUDIO_VERSION "?"
#endif


#define LA_MAINLOOP_DELAY (5)
#define LA_WORK_UDELAY    (200000)

#include "qAudioIpc.h"
#include "qAudioUtil.h"

typedef enum _la_status      la_status_t;
typedef struct _la_mainloop  la_mainloop_t;

enum _la_status{
    LA_STATUS_NONE,
    LA_STATUS_DAEMON_INIT,
    LA_STATUS_IPC_INIT,
    LA_STATUS_RUNNING,
    LA_STATUS_DEINIT,
    LA_STATUS_REINIT,
    LA_STATUS_ERROR,
    LA_STATUS_EXIT,

    LA_STATUS_MAX
};

struct _la_mainloop
{
   volatile la_status_t status;
   char client[IPC_MAX_NAME];
   bool enable_dsp;
};

struct la_status_info{
    la_status_t type;
    const char* str;
};


la_status_t la_mainloop_get_status(void);
void la_mainloop_set_status(la_status_t status);


#endif /* _QAUDIO_MAIN_LOOP_H */

