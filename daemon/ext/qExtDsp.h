#ifndef _QEXT_DSP_H
#define _QEXT_DSP_H

#define DSP_BACKUP_PATH "/tmp/dsp/"
#define ADSP_BACKUP     DSP_BACKUP_PATH"audio_data"
#define RDSP_BACKUP     DSP_BACKUP_PATH"radio_data"

int  qext_dsp_init(void);
void qext_dsp_deinit(void);

void qext_dsp_load_data(void);
void qext_dsp_backup_data(void);


void qext_dsp_ipc_audio(const char *from, unsigned int len, unsigned char *msg);
void qext_dsp_ipc_radio(const char *from, unsigned int len, unsigned char *msg);
void qext_dsp_ipc_send(unsigned char *msg, int len);


#endif /* _QEXT_DSP_H */

