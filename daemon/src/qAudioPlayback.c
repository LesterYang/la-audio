/*
 *  qAudioPlayback.c
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
#include "qAudioCore.h"

#define list_for_each_pcm(i, playback, pcm) \
        for(i=0; i<playback->card_num && (pcm = playback->card_list[i]); i++)

int la_playback_recovery (la_playback_pcm_t* pcm, int err);
la_playback_t* la_playback_get_notified_object();
void la_playback_reset_select_time(struct timeval* tv);      
void la_playback_thread_func(void *data);

la_errno_t la_playback_probe()
{
    snd_pcm_t* handle;
    la_playback_t* playback;
	int maxfd = 0;

    list_for_each_entry(la_get_playback_head(), playback, node)
    {
        char pcm[LA_PLAYBACK_PCM_NAME_MAX]={0};
    
        sprintf(pcm, "%s%d", LA_PLAYBACK_PCM_NAME, playback->id);
        
		playback->card_num = 0;
        playback->mutex = la_mutex_new(true,true);
        playback->mutex_stream = la_mutex_new(true,true);
		playback->chunk = LA_CARD_BUFFER_SIZE;
        playback->stream_len = LA_CARD_BUFFER_SIZE;
	    playback->stream = (char*)la_calloc(LA_CARD_BUFFER_SIZE);
        playback->card_list = (la_playback_pcm_t**)la_calloc(LA_PLAYBACK_CARD_MAX * sizeof(la_playback_pcm_t*));
        
        if(snd_pcm_open((snd_pcm_t**)&handle, pcm, SND_PCM_STREAM_PLAYBACK, 0) == 0)
        {         
            snd_pcm_close((snd_pcm_t*)handle);
            snd_config_update_free_global();
        }
        
        if(la_playback_set_master_softvol(playback, "0%") == LA_ERRNO_SUCCESS)
        {
            playback->master_mute = true;
            la_log_debug("init %s done", pcm);
        }
        else
        {
            la_log_debug("init %s fail", pcm);
        }


	    if(playback->stream == NULL)
            return LA_ERRNO_ALLOC;
	
        if (mkfifo(playback->fifo, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) == -1)
        {
            if(errno != EEXIST)
            {
                la_log_error("make %s error", playback->fifo);
                continue;
            }
            else
                la_log_notice("%s exist",playback->fifo);
        }

        // All of the writer close the FIFO, the reader sees the EOF and executes the request
        // select() will always return readable, but read nothing.
        // To open with  O_RDWR ensures that you have at least one writer on the FIFO
        if( (playback->fd = open(playback->fifo, O_RDWR | O_NONBLOCK)) >= 0 )
        {
            la_log_info("Open %s", playback->fifo);
			if(playback->fd > maxfd)
				maxfd = playback->fd;
        }
		else
		{
			la_log_error("Open %s error", playback->fifo);
		}
    }

	la_set_max_fd(maxfd);

	return LA_ERRNO_SUCCESS;
}

void la_playback_delet()
{
    int idx;
	la_playback_t* playback;

	while((playback = list_first_entry(la_get_playback_head(), la_playback_t, node)) != NULL)
	{
        if(playback->fd > 0)
        {
			la_close(playback->fd);
	
	        if(unlink(playback->fifo) == -1)
	            la_log_warn("unlink %s error", playback->fifo);
        }

        playback->fd = -1;
		la_list_del(&playback->node);
		la_free(playback->stream);
        la_free(playback->mutex);
        la_free(playback->mutex_stream);
        la_playback_set_master_softvol(playback, "0%");
    
        for(idx=0; idx<playback->card_num; idx++)
        { 
            la_playback_pcm_t* pcm = playback->card_list[idx];

            if(pcm == NULL)
                continue;

            playback->card_list[idx] = NULL;

            if(pcm->handle)
            {
                snd_pcm_drop(pcm->handle);
                snd_pcm_close(pcm->handle);
        		pcm->handle = NULL;
                la_log_notice("%s close %s", playback->name, (pcm->card) ? ((la_card_t*)pcm->card)->name : "(null)");
            }
         
            snd_config_update_free_global();
            la_free(pcm);
        }
        la_free(playback->card_list);
		la_free(playback);
	}
}

void la_playback_start(la_thread_t** thread)
{
    *thread = la_thread_new(la_playback_thread_func, la_get_playback_head());
}


la_errno_t la_playback_new(int id, char* name, int master_panel, la_list_t* head)
{

    la_playback_t* playback = la_calloc(sizeof(la_playback_t));

	if(playback == NULL)
		return LA_ERRNO_ALLOC;
	
    playback->id = id;
	playback->master_panel = master_panel;
    
	sprintf(playback->fifo, "%s%d", LA_PLAYBACK_FIFO_NAME, id);
	sprintf(playback->softvol, "%s%d", LA_PLAYBACK_SOFTVOL, id);
    sprintf(playback->name, "%s", la_strunknown(name));

    la_list_add_tail(head, &playback->node);
			
    return LA_ERRNO_SUCCESS;
}

void la_playback_update_chunk(int chunk)
{
    // need ot modify for each playback depend on different pcm in the feature

	la_playback_t* playback;

	list_for_each_entry(la_get_playback_head(), playback, node)
	{ 
	    la_mutex_lock(playback->mutex_stream);
        
        if( playback->stream_len < (playback->chunk = chunk))
        {
    		la_log_warn("re-allocate size : %d",chunk);
    		playback->stream = (char*)realloc(playback->stream, chunk);
        }
        
        la_mutex_unlock(playback->mutex_stream);
	}
}

la_errno_t la_playback_set_master_softvol(la_playback_t* playback, char* volume)
{
	char cmd[LA_PLAYBACK_CMD_SIZE]={0};

	sprintf(cmd, "amixer -c %d sset %s %s >/dev/null 2>&1", la_get_master_card(), playback->softvol, volume);	 

	la_log_debug("%s", cmd);

	return (system(cmd)) ? LA_ERRNO_SHELL : LA_ERRNO_SUCCESS;
}

la_playback_t* la_playback_get_playback(int id)
{
    la_playback_t* playback = NULL;

    list_for_each_entry(la_get_playback_head(), playback, node)
    {
        if(playback->id == id)
			break;
    }
    return playback;
}

la_errno_t la_playback_open_pcm(la_playback_t* playback, int card_id)
{
    int idx, err;
    la_playback_pcm_t* pcm;
    la_card_t* card = la_card_get_card(card_id);

	if(card == NULL)
		return LA_ERRNO_NO_DEV;

	if(playback->card_num >= LA_PLAYBACK_CARD_MAX)
	{
	    la_log_warn("card list of %s is full", playback->name);
		return LA_ERRNO_OTHER;
	}

    for(idx=0; idx<playback->card_num; idx++)
    {
        if(playback->card_list[idx] == NULL)
        {
            la_log_warn("%s card list data error!!", playback->name);
            continue;
        }
    
		if((la_card_t*)playback->card_list[idx]->card == card)
		{
			la_log_warn("%s has already existed in %s card list", card->name, playback->name);
			return LA_ERRNO_DEV_BUSY;
		}		
    }

    pcm = (la_playback_pcm_t*)la_calloc(LA_PLAYBACK_CARD_MAX * sizeof(la_playback_pcm_t));

    if(pcm == NULL)
        return LA_ERRNO_ALLOC;

	if((err = snd_pcm_open(&pcm->handle, card->pcm, card->param.stream, 0)) < 0)
	{
		la_log_error("playback open error : %s",snd_strerror(err));
        la_free(pcm);
		return LA_ERRNO_OPEN_PCM;
	}
    else
        la_log_notice("%s open %s",playback->name, card->name);

    pcm->card = card;
	pcm->start= true;

    la_card_set_params(pcm->handle, &card->param);

    la_mutex_lock(playback->mutex_stream);
    if( playback->stream_len < (playback->chunk = card->param.chunk_bytes))
    {       
        playback->stream_len = playback->chunk;  
        playback->stream = (char*)realloc(playback->stream, playback->stream_len);
        la_log_warn("re-allocate %s buffer size : %d", playback->name, playback->stream_len);
    }
    la_mutex_unlock(playback->mutex_stream);

    la_mutex_lock(playback->mutex);
    playback->card_list[playback->card_num++] = pcm;
    la_mutex_unlock(playback->mutex);

	return LA_ERRNO_SUCCESS;
}

la_errno_t la_playback_close_pcm(la_playback_t* playback, int card_id)
{
    int idx;
    la_playback_pcm_t* pcm = NULL;
    la_card_t* card = la_card_get_card(card_id);

	if(card == NULL)
		return LA_ERRNO_NO_DEV;

	if(playback->card_num == 0)
	{
		la_log_warn("card list of %s is empty", playback->name);
		return LA_ERRNO_OTHER;
	}

    la_mutex_lock(playback->mutex);

    for(idx=0; idx<playback->card_num; idx++)
    { 
		if(playback->card_list[idx] && (la_card_t*)playback->card_list[idx]->card == card)
        {        
            pcm = playback->card_list[idx];
            playback->card_list[idx] = NULL;
			break;
        }
    }

	if(pcm == NULL)
	{
		la_log_warn("%s doesn't exist in %s card list", card->name, playback->name);
        la_mutex_unlock(playback->mutex);
		return LA_ERRNO_NO_DEV;
	}

    if(pcm->handle)
    {
        snd_pcm_drop(pcm->handle);
        snd_pcm_close(pcm->handle);
		pcm->handle = NULL;
        la_log_warn("%s close %s", playback->name, ((la_card_t*)pcm->card)->name);
    }
 
    snd_config_update_free_global();
    la_free(pcm);
 
	for(idx++; idx<playback->card_num; idx++)
	{
		playback->card_list[idx-1] = playback->card_list[idx];
	}
	playback->card_list[idx-1] = NULL;
    playback->card_num--;
    
    la_mutex_unlock(playback->mutex);

	return LA_ERRNO_SUCCESS;
}

int la_playback_recovery (la_playback_pcm_t* pcm, int err)
{
    if(err == -EPIPE)
    {
		err = snd_pcm_prepare(pcm->handle);
    }
	else if(err == -ESTRPIPE && snd_pcm_resume(pcm->handle) == 0)
	{
		err = snd_pcm_prepare(pcm->handle);
	}

	if(err == 0)
		pcm->start = true;

	return err;
}

void la_playback_write(la_playback_t* playback, snd_pcm_sframes_t frames)
{
    la_assert(playback);
    snd_pcm_sframes_t ret;
	la_playback_pcm_t* pcm;
	int idx;

    la_mutex_lock(playback->mutex);

	list_for_each_pcm(idx, playback, pcm)
	{	
		if(!pcm->handle)
			continue;
		if(!la_is_running())
			break;

        //la_dbg(Q_ERR, "%s write %s %ld frames",playback->name, la_strnull(((la_card_t*)pcm->card)->name), frames);			

  		ret = snd_pcm_writei(pcm->handle, playback->stream, frames);

		if(ret == -EAGAIN && pcm->start)
		{
			pcm->start = false;
			if(snd_pcm_start(pcm->handle) < 0)
			{
				la_log_warn("start %s -> %s error",playback->name, ((la_card_t*)pcm->card)->name);
				continue;
			}	
			ret = snd_pcm_writei(pcm->handle, playback->stream, frames);
		}
		
		if(ret < 0)
        {            
            if(la_playback_recovery(pcm, ret) < 0)
                la_log_warn("write %s failed : %s", ((la_card_t*)pcm->card)->name, snd_strerror(ret));
        }  
	}

    la_mutex_unlock(playback->mutex);
}

la_playback_t* la_playback_get_notified_object()
{
	la_playback_t* playback = NULL;

	list_for_each_entry(la_get_playback_head(), playback, node)
    {
		if(FD_ISSET(playback->fd, la_get_fdset()))
			break;
    }
    return playback;
}


void la_playback_reset_select_time(struct timeval* tv)
{
    la_playback_t* playback;

	FD_ZERO(la_get_fdset());

    list_for_each_entry(la_get_playback_head(), playback, node)
    {
		FD_SET(playback->fd, la_get_fdset());
    }

    tv->tv_sec  = 0;
    tv->tv_usec = 500000;
}


void la_playback_thread_func(void *data)
{
	int ret;
	struct timeval tv;
	volatile la_playback_t* playback;

	while(la_is_running())
	{  
		ret = 0;
		
		do{
			if(ret > 0 && (playback = la_playback_get_notified_object()))
			{		
			    la_mutex_lock(playback->mutex_stream);
				ret = la_read(playback->fd, playback->stream, playback->chunk);
                la_mutex_unlock(playback->mutex_stream);

				if(ret > 0 && playback->card_num!= 0)
					la_playback_write((la_playback_t*)playback, byte_to_frame(ret));
			}
			la_playback_reset_select_time(&tv);
		}while((ret=select(la_get_max_fd() + 1,  la_get_fdset(), NULL, NULL, &tv)) >= 0 && la_is_running());
	}
}


