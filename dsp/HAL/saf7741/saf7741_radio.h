#ifndef _SAF7741_RADIO_H
#define _SAF7741_RADIO_H
#include <saf7741.h>

#define DEFAULT_FM_FREQ         (8750)  // 87.5 MHz
#define DEFAULT_AM_FREQ         (522)   // 522 kHz

#define SAF7741_ALIGN_BACKUP    SAF7741_DIRT_BACKUP"saf7741_radio_align"


int saf7741_radioOpenTuner(void);
int saf7741_radioResetTuner(void);
int saf7741_radioStartUpTuner(void);


int saf7741_radioSetBand(qdsp_radio_band band);
int saf7741_radioSetAMBand(void);
int saf7741_radioSetFMBand(void);

int  saf7741_radioSetFreq(short freq);
bool saf7741_radioSearchFreq(qdsp_radio_band band, short freq);

bool saf7741_radioPilot(void);

int saf7741_radioSetRadioMode(int mode);
int saf7741_radioSelecAnt(int val);
int saf7741_radioRBSDefault(void);
int saf7741_radioFMDeEmphasis(void);
int saf7741_radioSetAMAdjacent(void);
int saf7741_radioSetFMAdjacent(void);
int saf7741_radioSetFMLRDecoderCor(short val);
int saf7741_radioSetVolScaler(short gain);
int saf7741_radioSetFMLvAlign(short val);
int saf7741_radioSetAMLvAlign(short val);
int saf7741_radioSetPDC2FMIX(int fmix0, int fmix1);
void saf7741_radioStoreAlign(void);
void saf7741_radioLoadAlign(void);
int saf7741_radioFMAlign(void);
int saf7741_radioAMAlign(void);

#endif /* _SAF7741_RADIO_H */

