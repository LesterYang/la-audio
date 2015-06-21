#ifndef _QDSP_H
#define _QDSP_H

#include "qDspAudio.h"
#include "qDspRadio.h"

const char* qdsp_version(void);

int qdsp_open(qdsp_device dev);
int qdsp_reset(void);
void qdsp_close(void);



#endif /* _QDSP_H */

