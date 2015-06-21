#ifndef _QDSP_RADIO_H
#define _QDSP_RADIO_H

#include "qDspDev.h"

int  qdsp_radio_init(qdsp_device dev);
void qdsp_radio_deinit(void);

int  qdsp_radio_set_band(qdsp_radio_band band);
int  qdsp_radio_set_freq(short freq);
bool qdsp_radio_is_stereo(void);

void qdsp_radio_load_align(void);


#endif /* _QDSP_RADIO_H */

