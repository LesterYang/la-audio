#ifndef _QAUDIO_IPC_H
#define _QAUDIO_IPC_H

#define IPC_MAX_NAME    (8)
#define LA_IPC_DISABLE  (0)

// ============================
// exten command
// ============================
#define LA_IPC_CMD_DSP_AUDIO        (0x30)
#define LA_IPC_CMD_DSP_RADIO        (0xA0)
#define LA_IPC_CMD_DSP_BACKUP       (0x80)

// ============================
// command
// ============================
#define LA_IPC_CMD_SET_PCM          'a'
#define LA_IPC_CMD_CLEAR_PCM        'b'
#define LA_IPC_CMD_APPEND_PCM       'c'
#define LA_IPC_CMD_REMOVE_PCM       'd'

#define LA_IPC_CMD_START_RECORD     'A'
#define LA_IPC_CMD_STOP_RECORD      'B'

// ============================
// command length
// ============================
#define LA_IPC_CMD_SET_PCM_LEN      (4)
#define LA_IPC_CMD_CLEAR_PCM_LEN    (1)
#define LA_IPC_CMD_APPEND_PCM_LEN   (4)
#define LA_IPC_CMD_REMOVE_PCM_LEN   (4)
#define LA_IPC_CMD_START_RECORD_LEN (3)
#define LA_IPC_CMD_STOP_RECORD_LEN  (3)



// ============================
// command content
// ============================
#define LA_IPC_PANEL0               '0'
#define LA_IPC_PANEL1               '1'
#define LA_IPC_PANEL2               '2'

#define LA_IPC_PLAYBACK_LSTPLY0     '0'
#define LA_IPC_PLAYBACK_LSTPLY1     '1'
#define LA_IPC_PLAYBACK_LSTPLY2     '2'
#define LA_IPC_PLAYBACK_DMR         '3'
#define LA_IPC_PLAYBACK_MIRROR      '4'
#define LA_IPC_PLAYBACK_NAVI        '5'
#define LA_IPC_PLAYBACK_USB_CAP     '6'
#define LA_IPC_PLAYBACK_HDMI_CAP    '7'

#define LA_IPC_CAPTURE_HDMI         '0'
#define LA_IPC_CAPTURE_USB          '1'


// ============================
// command position
// ============================
#define LA_IPC_PANEL_POS            (0)
#define LA_IPC_PLAYBACK_POS         (1)


int  la_ipc_open(char* name);
void la_ipc_close();
void la_ipc_send(char *to, unsigned char *msg, int len);
const char* la_ipc_playback_str(char ipc_playback);
const char* la_ipc_panel_str(char ipc_panel);

void la_ipc_work(unsigned int len, unsigned char *msg);
void la_ipc_recv(const char *from, unsigned int len, unsigned char *msg);


#endif /* _QAUDIO_IPC_H */
