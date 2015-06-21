#ifndef _SAF7741_AUDIO_H
#define _SAF7741_AUDIO_H
#include <stdbool.h>
#include <saf7741.h>

int saf7741_audioSetSourceOnPrimary(saf7741_audio_source source);

int saf7741_audioSetPrimaryVol(int vol);
int saf7741_audioSetSecondVol(int vol);
int saf7741_audioSetPhoneVol(int vol);
int saf7741_audioSetNaviVol(int vol);

int saf7741_audioSetPrimaryMute(bool mute);
int saf7741_audioSetSecondMute(bool mute);
int saf7741_audioSetPhoneMute(bool mute);
int saf7741_audioSetNaviMute(bool mute);


int saf7741_audioSetPrimaryBal(short bal);
int saf7741_audioSetFade(short fade);

int saf7741_audioSetBassTone(short bass);
int saf7741_audioSetTrebleTone(short treble);
int saf7741_audioSetMidTone(short middle);

int saf7741_audioSetLoudness(char enable);

int saf7741_audioSetEQPattern(saf7741_eq_pattern pattern);

int saf7741_audioTransVol(int vol);

int saf7741_audioSetChannelMode(saf7741_channel_mode mode);


#endif /* _SAF7741_AUDIO_H */

