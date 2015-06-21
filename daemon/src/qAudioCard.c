/*
 *  qAudioCard.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "qAudioMainLoop.h"
#include "qAudioCard.h"
#include "qAudioPlayback.h"
#include "qAudioCapture.h"
#include "qAudioCore.h"


#define list_for_each_playback(i, card, playback) \
        for(i=0; i<LA_CARD_PLAYBACK_MAX && (playback = (la_playback_t*)card->playback_list[i]) && i<card->playback_num; i++)

#define list_for_each_capture(i, card, capture) \
		for(i=0; i<LA_CARD_CAPTURE_MAX && (capture = (la_capture_t*)card->capture_list[i]) && i<card->capture_num; i++)


la_errno_t la_card_probe()
{
    la_card_t* card;

    list_for_each_entry(la_get_panel_head(), card, node)
    {
        memset(card->playback_list, 0, LA_CARD_PLAYBACK_MAX);
		memset(card->capture_list, 0, LA_CARD_CAPTURE_MAX);

		card->playback_num = 0;
		card->capture_num = 0;
		card->param.access = LA_CARD_ACCESS;
		card->param.stream = LA_CARD_PLAYBACK;
		card->param.format = LA_CARD_FORMAT;
		card->param.channels = LA_CARD_CHANNELS;
		card->param.rate = LA_CARD_RATE;
    }

	return LA_ERRNO_SUCCESS;
}


void la_card_delet()
{
    la_card_t* card;

	while((card = list_first_entry(la_get_panel_head(), la_card_t, node)) != NULL)
    {
		la_list_del(&card->node);
		la_free(card);
    }
}

la_errno_t la_card_new(int id, char* name, la_list_t* head)
{
	la_card_t* card = la_calloc(sizeof(la_card_t));

	if(card == NULL)
		return LA_ERRNO_ALLOC;

	card->id = id;
	sprintf(card->pcm, "%s%d", LA_CARD_PCM_NAME, id);
    sprintf(card->name, "%s", la_strunknown(name));

	la_list_add_tail(head, &card->node);

	return LA_ERRNO_SUCCESS;
}


void la_card_append_pcm(int card_id, int playback_id)
{
    int idx;
	la_card_t* card = la_card_get_card(card_id);
	la_playback_t* playback = la_playback_get_playback(playback_id);
	la_playback_t* exist_playback;

	if(!card || !playback)
		return;
	
    list_for_each_playback(idx, card, exist_playback)
    {
		if(exist_playback == playback)
		{
			la_log_warn("%s has already existed in %s playback list", playback->name, card->name);
			return;
		}
    }

	if(playback->master_panel == card->id)
	{
	    la_capture_t* capture = la_capture_get_capture_playback(playback->id);

        // To check whether need to record or not?
        if(capture && la_capture_start(capture) != LA_ERRNO_SUCCESS)
            return;

        if(la_playback_set_master_softvol(playback, "100%") == LA_ERRNO_SUCCESS)
			playback->master_mute = false;
	}
	else
	{
	    // Get duplicated stream from fifo
	    if(la_playback_open_pcm(playback, card_id) != LA_ERRNO_SUCCESS)
		    return;
	}
		
	card->playback_list[card->playback_num++] = playback;
}

void la_card_clear_pcm(int card_id)
{
    int idx;
	la_playback_t* playback;
	la_card_t* card = la_card_get_card(card_id);

	if(!card)
		return;

	list_for_each_playback(idx, card, playback)
	{
		if(card->id == playback->master_panel)
        {   
            // To check whether need to stop recording or not?
            la_capture_t* capture = la_capture_get_capture_playback(playback->id);
                
            if(capture)
                la_capture_stop(capture);

            if(la_playback_set_master_softvol(playback, "0%") != LA_ERRNO_SUCCESS)
                la_log_warn("shell command error!!");

            playback->master_mute = true;
        }        
        else
            la_playback_close_pcm(playback, card_id);

		card->playback_list[idx] = NULL;
	}

    card->playback_num = 0;
}

void la_card_set_pcm(int card_id, int playback_id)
{
	la_card_clear_pcm(card_id);
	la_card_append_pcm(card_id, playback_id);
}

void la_card_remove_pcm(int card_id, int playback_id)
{
    int idx;
	la_card_t* card = la_card_get_card(card_id);
    la_playback_t* playback = la_playback_get_playback(playback_id);
    la_playback_t* exist_playback = NULL;

	if(!card || !playback)
		return;
	
    list_for_each_playback(idx, card, exist_playback)
    {
		if(exist_playback == playback)
		    break;
        else
            exist_playback = NULL;
    }

    if(exist_playback == NULL)
    {
        la_log_warn("%s doesn't exist in %s playback list", playback->name, card->name);
        return;
    }

	if(card->id == playback->master_panel)
    {   
    	la_capture_t* capture = la_capture_get_capture_playback(playback->id);

        if(capture)
            la_capture_stop(capture);

        if(la_playback_set_master_softvol(playback, "0%") != LA_ERRNO_SUCCESS)
            la_log_warn("shell command error!!");

        playback->master_mute = true;
    }        
    else
        la_playback_close_pcm(playback, card_id);

	card->playback_list[idx] = NULL;
    
	for(idx++; idx<card->playback_num; idx++)
	{
		card->playback_list[idx-1] = card->playback_list[idx];
	}
	card->playback_list[idx-1] = NULL;
    card->playback_num--;
}

void la_card_start_record(int id)
{
    la_capture_t* capture = la_capture_get_capture(id);

    if(capture)
        la_capture_start(capture);
}

void la_card_stop_record(int id)
{
    la_capture_t* capture = la_capture_get_capture(id);

    if(capture)
        la_capture_stop(capture);
}


la_card_t* la_card_get_card(int id)
{
    la_card_t* card = NULL;

    list_for_each_entry(la_get_panel_head(), card, node)
    {
        if(card->id == id)
			break;
    }
    return card;
}

void la_card_set_params(snd_pcm_t* handle, la_card_param_t* param)
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t   buffer_frames;
    snd_pcm_uframes_t   period_frames;
    snd_pcm_uframes_t   start_threshold_frames;
    snd_pcm_uframes_t   stop_threshold_frames;

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    if(snd_pcm_hw_params_any(handle, hwparams) < 0)
        return;

    snd_pcm_hw_params_set_access(handle, hwparams, param->access);
    snd_pcm_hw_params_set_format(handle, hwparams, param->format);
    snd_pcm_hw_params_set_channels(handle, hwparams, param->channels);
    snd_pcm_hw_params_set_rate_near(handle, hwparams, &param->rate, LA_CARD_DIR_EXACT);

    if((buffer_frames = param->buffer_size) == 0)
        snd_pcm_hw_params_get_buffer_size_max(hwparams, &buffer_frames);
    
    period_frames = buffer_frames / 4;

    snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &buffer_frames);
    // No effect if asound.conf has set period size
    snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_frames, LA_CARD_DIR_EXACT);

	param->monotonic = snd_pcm_hw_params_is_monotonic(hwparams);
	param->can_pause = snd_pcm_hw_params_can_pause(hwparams);

	if (snd_pcm_hw_params(handle, hwparams) < 0)
	    return;

    snd_pcm_hw_params_get_period_size(hwparams, &param->period_size, 0);
    snd_pcm_hw_params_get_buffer_size(hwparams, &param->buffer_size);

    snd_pcm_sw_params_current(handle, swparams);

    snd_pcm_sw_params_set_avail_min(handle, swparams, param->period_size);

    start_threshold_frames = param->buffer_size;
    stop_threshold_frames = param->buffer_size;

    if(param->stream == SND_PCM_STREAM_PLAYBACK)
        snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold_frames);
    else
        snd_pcm_sw_params_set_start_threshold(handle, swparams, 1);
  
    snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold_frames);
    
	if (snd_pcm_sw_params(handle, swparams) < 0)
	    return;

    if(la_more_pcm_info())
    {
        snd_pcm_dump(handle, la_get_output_log());
    }

    param->bits_per_sample = snd_pcm_format_physical_width(param->format);
    param->bytes_per_frame = (param->bits_per_sample * param->channels) / 8;
    param->chunk_bytes = param->period_size * param->bytes_per_frame;
}

int la_card_get_hw_by_name(const char* name)
{
    int card = -1;
       
    for (snd_card_next(&card); card>=0; snd_card_next(&card)) 
    {
       char* str=NULL;

       snd_card_get_name(card, &str);

       la_log_debug("search card %d : %s",card, la_strnull(str));

       if(str && strstr(str, name))
       {
            la_free(str);
            return card;
       }
       la_free(str);
    }
        
    return -1;
}

