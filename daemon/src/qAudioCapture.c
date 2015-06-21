/*
 *  qAudioCapture.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */

#include <sys/types.h>
#include <sys/stat.h>
       
#include "qAudioCapture.h"
#include "qAudioPlayback.h"
#include "qAudioCard.h"
#include "qAudioCore.h"

// LA_CAPTURE_BUFFER_SIZE is just tuned results for RSE001/AVN002
// Period size of capture is LA_CAPTURE_BUFFER_SIZE/4 if asound.conf hasn't set 
// period size. It's for auto-capture device such as USB card, Android ...  
#define LA_CAPTURE_BUFFER_SIZE (4096)

int la_capture_open_keyword_pcm(la_capture_t* capture);
la_errno_t la_capture_open_pcm(la_capture_t* capture);
la_errno_t la_capture_close_pcm(la_capture_t* capture);
int la_capture_recovery(snd_pcm_t* handle, int err);
int la_capture_read(la_capture_t* capture);
void la_capture_write(la_capture_t* capture, snd_pcm_sframes_t frames);
//void la_capture_reset_select_time(la_capture_t* capture, struct timeval* tv);

void la_capture_thread_func(void *data);

la_errno_t la_capture_probe()
{
	la_capture_t* capture;

    list_for_each_entry(la_get_capture_head(), capture, node)
    { 
        capture->mutex = la_mutex_new(true, true);
        capture->chunk = LA_CARD_PERIOD_SIZE;
        capture->stream_len = LA_CARD_BUFFER_SIZE;
        capture->param.access = LA_CARD_ACCESS;
		capture->param.stream = LA_CARD_CAPTURE;
		capture->param.format = LA_CARD_FORMAT;
		capture->param.channels = LA_CARD_CHANNELS;
		capture->param.rate = LA_CARD_RATE;
        capture->param.buffer_size = LA_CAPTURE_BUFFER_SIZE;
	    capture->stream = (char*)la_calloc(LA_CARD_BUFFER_SIZE);

		if(capture->stream == NULL)
			return LA_ERRNO_ALLOC;
    }

	return LA_ERRNO_SUCCESS;
}

void la_capture_delet()
{
	la_capture_t* capture;

	while((capture = list_first_entry(la_get_capture_head(), la_capture_t, node)) != NULL)
	{
        if(capture->connect == true)
            la_capture_stop(capture);

		la_list_del(&capture->node);
		la_free(capture->stream);
        if(capture->mutex)
            la_mutex_free(capture->mutex);
		la_free(capture);
	}
}


la_errno_t la_capture_new(int id, int hw_id, int pipe_id, char* name, la_list_t* head)
{
    la_capture_t* capture = la_calloc(sizeof(la_capture_t));

	if(capture == NULL)
		return LA_ERRNO_ALLOC;
	
    capture->id      = id;
    capture->hw_id   = hw_id;
    capture->pipe_id = pipe_id;
    
    sprintf(capture->pcm, "plug:%s%d", LA_CAPTURE_PCM_NAME, id);
    sprintf(capture->name, "%s", la_strunknown(name));

    la_list_add_tail(head, &capture->node);
	
    return LA_ERRNO_SUCCESS;

}

int la_capture_open_keyword_pcm(la_capture_t* capture)
{
    int card;
    char pcm[LA_CAPTURE_SRC_NAME_MAX]={0};
    
    card = la_card_get_hw_by_name(capture->name);

    if(card < 0)
    {
        la_log_warn("no %s sound card",capture->name);
        return -1;
    }
    
    sprintf(pcm, "plughw:%d", card);
    la_log_debug("open capture %s ...", pcm);
    
    return snd_pcm_open(&capture->handle, pcm, capture->param.stream, 0);
}


la_errno_t la_capture_open_pcm(la_capture_t* capture)
{
    int err;
    char pcm[LA_PLAYBACK_PCM_NAME_MAX]={0};
    la_playback_t* playback = la_playback_get_playback(capture->pipe_id);
    la_card_t* card = la_card_get_card(playback->master_panel);
    la_card_param_t param;

    la_assert(capture);

    if(playback == NULL || card == NULL)
        return LA_ERRNO_NO_DEV;

    memcpy(&param, &card->param, sizeof(la_card_param_t));
    sprintf(pcm, "%s%d", LA_PLAYBACK_PCM_NAME, playback->id);

	if((err = snd_pcm_open(&capture->pipe_handle, pcm, param.stream, 0)) < 0)
	{
		la_log_error("open %s error : %s", pcm, snd_strerror(err));
        la_free(pcm);
		return LA_ERRNO_OPEN_PCM;
	}

    la_card_set_params(capture->pipe_handle, &param);

    if(capture->hw_id == -1)
        err = la_capture_open_keyword_pcm(capture);
    else
	    err = snd_pcm_open(&capture->handle, capture->pcm, capture->param.stream, 0);

    if (err < 0)
    {
		la_log_error("capture open error : %s", snd_strerror(err));
        #if 0
        la_close(capture->pipe_fd);
        #else
        snd_pcm_close(capture->pipe_handle);
        snd_config_update_free_global();
        #endif
		return LA_ERRNO_OPEN_PCM;
	}
    else
        la_log_notice("open %s -> %s",capture->name, playback->fifo);

    la_card_set_params(capture->handle, &capture->param);

    if( capture->stream_len < (capture->chunk = capture->param.chunk_bytes))
    {       
        capture->stream_len = capture->chunk;  
        capture->stream = (char*)realloc(capture->stream, capture->stream_len);
        la_log_warn("re-allocate %s buffer size : %d", capture->name, capture->stream_len);
    }

    if(snd_pcm_nonblock(capture->handle, 1) < 0)
        la_log_warn("set capture nonblock error");

    capture->pipe_start = true;
    capture->connect = true;
    
    return LA_ERRNO_SUCCESS;
}

la_errno_t la_capture_close_pcm(la_capture_t* capture)
{
    la_playback_t* playback = la_playback_get_playback(capture->pipe_id);

    la_assert(capture);

    if(playback == NULL)
        return LA_ERRNO_NO_DEV;
      
    snd_pcm_drop(capture->handle);
    snd_pcm_drop(capture->pipe_handle);
    snd_pcm_close(capture->handle);
    snd_pcm_close(capture->pipe_handle);

    snd_config_update_free_global();

    capture->handle = NULL;
    capture->pipe_handle= NULL;
    
    return LA_ERRNO_SUCCESS;
}

la_capture_t* la_capture_get_capture(int id)
{
    la_capture_t* capture = NULL;

    list_for_each_entry(la_get_capture_head(), capture, node)
    {
        if(capture->id == id)
			break;
    }
    return capture;
}

la_capture_t* la_capture_get_capture_playback(int playback_id)
{
    la_capture_t* capture = NULL;

    list_for_each_entry(la_get_capture_head(), capture, node)
    {
        if(capture->pipe_id == playback_id)
			break;
    }
    return capture;
}

la_errno_t la_capture_start(la_capture_t* capture)
{
    if(capture->connect)
        return LA_ERRNO_DEV_BUSY;

    if(la_capture_open_pcm(capture) != LA_ERRNO_SUCCESS)
        return LA_ERRNO_OPEN_PCM;

	capture->thread = la_thread_new(la_capture_thread_func, capture);

    if(capture->thread == NULL)
        return LA_ERRNO_ALLOC;

	return LA_ERRNO_SUCCESS;
}


la_errno_t la_capture_stop(la_capture_t* capture)
{
    if(capture->connect == false)
        return LA_ERRNO_NO_DEV;

    capture->connect = false; 
	
	if(capture->thread)
	{
		la_thread_wait_free(capture->thread);
		capture->thread = NULL;
	}

    la_capture_close_pcm(capture);
    
	return LA_ERRNO_SUCCESS;
}

int la_capture_recovery(snd_pcm_t* handle, int err)
{
    if(err == -EPIPE)
    {
		err = snd_pcm_prepare(handle);
    }
	else if(err == -ESTRPIPE && snd_pcm_resume(handle) == 0)
	{
		err = snd_pcm_prepare(handle);
	}

    return err;
}


int la_capture_read(la_capture_t* capture)
{
    int data = 0;
    snd_pcm_uframes_t count = byte_to_frame(capture->chunk);
    snd_pcm_sframes_t ret;

    la_log_debug("la_capture is going to read");
    
    while(count > 0 && capture->connect)
    {        
        ret = snd_pcm_readi(capture->handle, &capture->stream[data], count);

        if(ret == -EAGAIN)
        {
            la_log_debug("snd_pcm_readi() return EAGAIN");
            ret = snd_pcm_wait(capture->handle, 100);
            if(ret != 0)
                continue;
        }
        else if(ret < 0)
        {
            if(la_capture_recovery(capture->handle, ret) < 0)
            {
                la_log_warn("read %s failed : %s", capture->name, snd_strerror(ret));
                return ret;
            }
        }
        else if(ret == 0)
        {
            la_log_debug("capture read nothing");
        }
        else
        {
            count -= ret;
            data += ret;
        }        
    }

    return data;
}


void la_capture_write(la_capture_t* capture, snd_pcm_sframes_t frames)
{
    snd_pcm_sframes_t ret;

    if(!capture->pipe_handle)
    {
        if(capture->recv_cb)
            capture->recv_cb((unsigned char*)capture->stream, frames);
        return;
    }

    ret = snd_pcm_writei(capture->pipe_handle, capture->stream, frames);

    if(ret == -EAGAIN && capture->pipe_start)
    {
    	capture->pipe_start = false;
    	if(snd_pcm_start(capture->pipe_handle) < 0)
    	{
    		la_log_warn("%s start to write error",capture->name);
    		return;
    	}	
    	ret = snd_pcm_writei(capture->pipe_handle, capture->stream, frames);
    }

    if(ret < 0)
    { 
        if(ret == -EPIPE)
        {
            ret = snd_pcm_prepare(capture->pipe_handle);
        }
        else if(ret == -ESTRPIPE && snd_pcm_resume(capture->pipe_handle) == 0)
        {
            ret = snd_pcm_prepare(capture->pipe_handle);
        }
        
        if(ret == 0)
            capture->pipe_start = true;
        else if (ret < 0)
            la_log_warn("%s write failed : %s",capture->name, snd_strerror(ret));
    }  
}


#if 0
void la_capture_reset_select_time(la_capture_t* capture, struct timeval* tv)
{
	FD_ZERO(&capture->fifofds);
    FD_SET(capture->fd, &capture->fifofds);

    tv->tv_sec  = 0;
    tv->tv_usec = 500000;
}
#endif
		
void la_capture_thread_func(void *data)
{
    int ret;
    la_capture_t* capture = (la_capture_t*)data;

	while(la_is_running() && capture->connect)
	{  
		ret = la_capture_read(capture);

        if(!capture->connect)
            break;
		else if(ret > 0)     
			la_capture_write(capture, ret);
        else if(ret == 0)
            usleep(LA_CAPTURE_UDELAY);
        else if(ret == -EBADFD || ret == -ENODEV)
            sleep(LA_CAPTURE_DELAY);
	}
}

void la_capture_set_recv_callback(int id, void (*recv)(unsigned char* buf, int len))
{
    la_capture_t* capture = la_capture_get_capture(id);

    if(capture)
        capture->recv_cb = recv;
}


