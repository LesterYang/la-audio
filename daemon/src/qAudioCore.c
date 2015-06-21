/*
 *  qAudioCore.c
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
#include <ctype.h>

#include "qAudioMainLoop.h"
#include "qAudioPlayback.h"
#include "qAudioCapture.h"
#include "qAudioCard.h"
#include "qAudioCore.h"

typedef struct _la_core_data la_core_data_t;

struct _la_core_data{
    volatile bool   run;
	
	int 			master_card;
    int 			panel_num;
	la_list_t		panel_head;

	int  			maxfd;
	fd_set 			fifofds;
	int 			playback_num;
	la_list_t		playback_head;
	la_thread_t*	playback_thread;
	
	int 			capture_num;
	la_list_t 		capture_head;
	
	snd_output_t* 	log;
	bool 		    verbose;
};


static la_core_data_t qa;


void la_load_asound_conf(void);
void la_update_card(char* msg);
void la_update_playback(char* msg);
void la_update_capture(char* msg);
void la_set_master_card(char* msg);


la_errno_t la_init()
{
    la_errno_t err = LA_ERRNO_SUCCESS;

    qa.run = true;
    qa.playback_thread = NULL;

    la_init_head(&qa.panel_head);
    la_init_head(&qa.playback_head);
    la_init_head(&qa.capture_head);

    la_load_asound_conf();

    if((err = la_capture_probe()) != LA_ERRNO_SUCCESS)
            goto err_init;

    if((err = la_playback_probe()) != LA_ERRNO_SUCCESS)
            goto err_init;

    if((err = la_card_probe()) != LA_ERRNO_SUCCESS)
            goto err_init;


    if(qa.playback_num)
    la_playback_start(&qa.playback_thread);

    return LA_ERRNO_SUCCESS;

err_init:
    la_deinit();
    return err;
}

void la_deinit(void)
{
	qa.run = false;
	
	if(qa.playback_thread)
	{
		la_thread_wait_free(qa.playback_thread);
		qa.playback_thread = NULL;
	}

    la_capture_delet();
    la_card_delet();
    la_playback_delet();
	
    if(qa.verbose)
		snd_output_close(qa.log);

	
}

void la_show_pcm_structure()
{
    if(snd_output_stdio_attach(&qa.log, stderr, 0) == 0)
        qa.verbose = true;
}

bool la_more_pcm_info()
{
    return qa.verbose;
}

void la_set_max_fd(int fd)
{
	qa.maxfd = fd;
}

int la_get_max_fd()
{
	return qa.maxfd;
}

fd_set* la_get_fdset()
{
	return &qa.fifofds;
}

int la_get_panel_num()
{
	return qa.panel_num;
}

int la_get_playback_num()
{
	return qa.playback_num;
}

int la_get_capture_num()
{
	return qa.capture_num;
}

int la_get_master_card()
{
	return qa.master_card;
}

la_list_t* la_get_capture_head()
{
	return &qa.capture_head;
}

la_list_t* la_get_panel_head()
{
	return &qa.panel_head;
}

la_list_t* la_get_playback_head()
{
	return &qa.playback_head;
}

bool la_is_running()
{
	return (bool)qa.run;
}

snd_output_t* la_get_output_log()
{
    return qa.log;
}


void la_load_asound_conf()
{
	FILE *fr;
	char* str;
	char buf[LA_BUF_SIZE];

	fr = fopen("/etc/asound.conf", "r");

	if(fr == NULL)
	{
		la_log_error("open asound.conf error");
		return;
	}

	while( !feof(fr) )
	{
	    memset(buf, 0, LA_BUF_SIZE);
		
		if(fgets(buf, LA_BUF_SIZE, fr) == NULL)
			continue;

		if(buf[0] == 'p')
            break;
		else if (buf[0] != '#' || buf[LA_BUF_SIZE - 2] != 0)
			continue;

        if((str = strstr(buf, LA_PLAYBACK_PCM_NAME)))
			la_update_playback(str);
        else if ((str = strstr(buf, LA_CAPTURE_PCM_NAME)))
			la_update_capture(buf); /* hw id is set before LA_CAPTURE_PCM_NAME in 
                                                              asound.conf, so use 'buf' instead of using 'str' */
		else if ((str = strstr(buf, LA_CARD_PCM_NAME)))
			la_update_card(str);
        else if ((str = strstr(buf, LA_MASTER_CARD))) /* LST_MASTER_CARD=X in asound.conf*/
			la_set_master_card(str);

	}

    fclose(fr);
}

void la_update_card(char* msg)
{
    int id, idx;
	int pos = sizeof(LA_CARD_PCM_NAME) - 1;

	if(!isdigit(msg[pos]))
		return;

    /* set card(panel) id */
	id = msg[pos] - '0';


    /* set name string */
    idx = la_str_index(msg, '"');

    if(idx > 0 && idx+2 < strlen(msg))
    {
        msg = &msg[idx+1];
        if((idx = la_str_index(msg, '"')) == -1)
            msg = NULL;
        else
            msg[idx] = 0x0;
    }
    else
        msg = NULL;

	if(la_card_new(id, msg, &qa.panel_head) == LA_ERRNO_SUCCESS)
		qa.panel_num++;

}


void la_update_playback(char* msg)
{
    int id, master_panel, len, idx;
	char* str = NULL;
	int pos = 0;

    /* set playback id [ lstswX in asound.conf ] */
    pos = sizeof(LA_PLAYBACK_PCM_NAME) - 1;

    if((id = la_positive_digit_to_integer(&msg[pos])) < 0)
        return;

    /* set master panel id [ dmix_X in asound.conf ] */
	str = strstr(msg, LA_DMIX_PCM);
	pos = sizeof(LA_DMIX_PCM) - 1;

	if(str == NULL || !isdigit(str[pos]))
		return;

    master_panel = str[pos] - '0';

    /* set name string */
    len = strlen(str);
    idx = la_str_index(str, '"');

    if(idx > 0 && idx+2 < len)
    {
        str = &str[idx+1];
        if((idx = la_str_index(str, '"')) == -1)
            str = NULL;
        else
            str[idx] = 0x0;
    }
    else
        str = NULL;

    if(la_playback_new(id, str, master_panel, &qa.playback_head) == LA_ERRNO_SUCCESS)
		qa.playback_num++;
	 
}



void la_update_capture(char* msg)
{
    int id, hw_id, pipe_id, len, idx;
	char* str = NULL;
    int pos = 0;

    /* set hw id [hw_X] */
    str = strstr(msg, LA_HW_PCM);
	pos = sizeof(LA_HW_PCM) - 1;

	if(str == NULL)
		return;
    
    if(isdigit(str[pos]))
        hw_id = str[pos] - '0';
    else if (str[pos] == 'a' || str[pos] == 'A')
        hw_id = -1; /* auto get sound card hardware id*/
    else
        return;

    /* set capture id [ lstrcX in asound.conf ] */
    str = strstr(msg, LA_CAPTURE_PCM_NAME);
    pos = sizeof(LA_CAPTURE_PCM_NAME) - 1;

    if((id = la_positive_digit_to_integer(&str[pos])) < 0)
        return;

    /* set pipe playback id [ play_swX in asound.conf ] */
    str = strstr(str, LA_CAPTURE_PIPE_NAME);
    pos = sizeof(LA_CAPTURE_PIPE_NAME) - 1;

    if((pipe_id = la_positive_digit_to_integer(&str[pos])) < 0)
        return;

    /* set name string */
    len = strlen(str);
    idx = la_str_index(str, '"');

    if(idx > 0 && idx + 2 < len)
    {
        str = &str[idx+1];
        if((idx = la_str_index(str, '"')) == -1)
            str = NULL;
        else
            str[idx] = 0x0;
    }
    else
        str = NULL;
    
    if(la_capture_new(id, hw_id, pipe_id, str, &qa.capture_head) == LA_ERRNO_SUCCESS)
		qa.capture_num++;
    
}

void la_set_master_card(char* msg)
{
	int pos = sizeof(LA_MASTER_CARD) - 1;

	if(strlen(msg) <= pos || !isdigit(msg[pos]))
		return;

	qa.master_card = msg[pos] - '0';
	la_log_info("Master audio card %d", qa.master_card);

}

