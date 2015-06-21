/*
 *  qAudioIpc.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : Open IPC chanel to communicate to AP
 */
 
#include <unistd.h>
#include <string.h>

#include "qAudioIpc.h"
#include "qAudioCore.h"
#include "qAudioCard.h"
#include "qAudioMainLoop.h"
#include "qAudioUtil.h"
#include "qAudioLog.h"

#include "../ext/qExtDsp.h"

#define IPC_DEFAULT_NAME "LSTSHL"
#define IPC_DBG          (0)
#define IPC_RETRY        (3)

#if (!LA_IPC_DISABLE)
#include <lst_ipc_client_lib.h>

typedef struct _la_ipc_data{
        LST_Channel *server;
        LST_RECV_EVENT recv_func;
        LST_PROTOCOL_ST status;
        char name[IPC_MAX_NAME];
        char debug;
        char *target;
        unsigned char *msg;
        int  len;
}la_ipc_data_t;
static la_ipc_data_t g_client;

#endif

typedef struct _la_ipc_param{
    char hdr;
    char cmd;
    int len;
}la_ipc_param_t;


void la_ipc_recv(const char *from, unsigned int len, unsigned char *msg);
void la_ipc_set_pcm(unsigned int len, unsigned char *msg);
void la_ipc_clear_pcm(unsigned int len, unsigned char *msg);
void la_ipc_append_pcm(unsigned int len, unsigned char *msg);
void la_ipc_remove_pcm(unsigned int len, unsigned char *msg);
void la_ipc_start_record(unsigned int len, unsigned char *msg);
void la_ipc_stop_record(unsigned int len, unsigned char *msg);



int la_ipc_open(char* name)
{
#if LA_IPC_DISABLE
    return 0;
#else

    int retry, len;

    g_client.status = PROTOCOL_IDLE;
    g_client.recv_func = la_ipc_recv;
    if (name && *name && (len=strlen(name))<IPC_MAX_NAME) 
    {
        memcpy(g_client.name, name, len);
    }
    else 
    {
        memcpy(g_client.name, IPC_DEFAULT_NAME, strlen(IPC_DEFAULT_NAME));
    }

    for(retry=0;;retry++)
    {
        g_client.server = lst_open_channel(g_client.name, 0, IPC_DBG);

        if(g_client.server != NULL)
            break;

        usleep(100000);

        if(retry == IPC_RETRY)
        {
            la_log_warn("open \"%s\" channel status timeout",g_client.name);
            return 1;
        }
    }

    la_log_info("open IPC client : \"%s\"",g_client.name);
    // set receiving event callback function
    lst_set_event(g_client.server, g_client.recv_func);

    return 0;
#endif
}

void la_ipc_close()
{
#if LA_IPC_DISABLE
#else
    if(g_client.server)
    {
        lst_close_channel(g_client.server);
        g_client.server = NULL;
    }
#endif
}

void la_ipc_recv(const char *from, unsigned int len, unsigned char *msg)
{
    la_log_debug("recv len %d, from %s", len, from);

    if(len <= 0)
        return;

    switch(msg[0]&0xF0)
    {
        case LA_IPC_CMD_DSP_AUDIO:      qext_dsp_ipc_audio(from, len, msg);     break;
        case LA_IPC_CMD_DSP_RADIO:      qext_dsp_ipc_radio(from, len, msg);     break;
        case LA_IPC_CMD_DSP_BACKUP:     qext_dsp_backup_data();                 break;
        default:
            switch(msg[0])
            {
                case LA_IPC_CMD_SET_PCM:        la_ipc_set_pcm(len-1, &msg[1]);         break;
                case LA_IPC_CMD_CLEAR_PCM:      la_ipc_clear_pcm(len-1, &msg[1]);       break;
                case LA_IPC_CMD_APPEND_PCM:     la_ipc_append_pcm(len-1, &msg[1]);      break;
                case LA_IPC_CMD_REMOVE_PCM:     la_ipc_remove_pcm(len-1, &msg[1]);      break;
                case LA_IPC_CMD_START_RECORD:   la_ipc_start_record(len-1, &msg[1]);    break;
                case LA_IPC_CMD_STOP_RECORD:    la_ipc_stop_record(len-1, &msg[1]);     break;
                default:                                                                break;
            }                                                                 
        break;
    }    
}

void la_ipc_send(char *to, unsigned char *msg, int len)
{
#if LA_IPC_DISABLE
    return;
#else
    if(!g_client.server)
        return;

    g_client.status = lst_send_buffer(g_client.server, to, msg, len, IPC_NOACK);  
    
    if(g_client.status!=PROTOCOL_ACK_OK)
        la_log_debug("send to %s error", to);
#endif

}


void la_ipc_set_pcm(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_SET_PCM_LEN)
		return;

    int card_id = msg[0] - '0';
    int playback_id = la_digit_3bytes_to_integer(&msg[1]);

    if(playback_id < 0)
    {
        la_log_warn("la_ipc_set_pcm() playback id error");
        return;
    }

    la_card_set_pcm(card_id, playback_id);
	la_log_show_status();
}

void la_ipc_clear_pcm(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_CLEAR_PCM_LEN)
		return;

	int card_id = msg[0] - '0';

    la_card_clear_pcm(card_id);
	la_log_show_status();
}

void la_ipc_append_pcm(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_APPEND_PCM_LEN)
		return;
	
    int card_id = msg[0] - '0';
    int playback_id = la_digit_3bytes_to_integer(&msg[1]);

    if(playback_id < 0)
    {
        la_log_warn("la_ipc_append_pcm() playback id error");
        return;
    }


    la_card_append_pcm(card_id, playback_id);
	la_log_show_status();
}

void la_ipc_remove_pcm(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_REMOVE_PCM_LEN)
		return;
	
    int card_id = msg[0] - '0';
    int playback_id = la_digit_3bytes_to_integer(&msg[1]);

    if(playback_id < 0)
    {
        la_log_warn("la_ipc_remove_pcm() playback id error");
        return;
    }


    la_card_remove_pcm(card_id, playback_id);
	la_log_show_status();
}


void la_ipc_start_record(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_START_RECORD_LEN)
		return;

    int capture_id = la_digit_3bytes_to_integer(msg);

    if(capture_id < 0)
    {
        la_log_warn("la_ipc_start_record() capture id error");
        return;
    }

    la_card_start_record(capture_id);
    la_log_show_status();
}

void la_ipc_stop_record(unsigned int len, unsigned char *msg)
{
    if(len < LA_IPC_CMD_STOP_RECORD_LEN)
		return;
    
    int capture_id = la_digit_3bytes_to_integer(msg);

    if(capture_id < 0)
    {
        la_log_warn("la_ipc_start_record() capture id error");
        return;
    }

    la_card_stop_record(capture_id);
    la_log_show_status();
}


/*
la_intf_device_t la_ipc_get_intf_lstsw(char ipc_playback)
{
	la_intf_device_t lstsw = LA_INTF_DEVICE_LSTSW_NULL;
	switch(ipc_playback)
	{
        case LA_IPC_PLAYBACK_LSTPLY0:   lstsw = LA_INTF_DEVICE_LSTSW0;   break;
        case LA_IPC_PLAYBACK_LSTPLY1:   lstsw = LA_INTF_DEVICE_LSTSW1;   break;
        case LA_IPC_PLAYBACK_LSTPLY2:   lstsw = LA_INTF_DEVICE_LSTSW2;   break;
        case LA_IPC_PLAYBACK_DMR:       lstsw = LA_INTF_DEVICE_LSTSW3;   break;
        case LA_IPC_PLAYBACK_MIRROR:    lstsw = LA_INTF_DEVICE_LSTSW4;   break;
		case LA_IPC_PLAYBACK_NAVI:      lstsw = LA_INTF_DEVICE_LSTSW5;   break;
		case LA_IPC_PLAYBACK_USB_CAP:   lstsw = LA_INTF_DEVICE_LSTSW6;   break;
        case LA_IPC_PLAYBACK_HDMI_CAP:  lstsw = LA_INTF_DEVICE_LSTSW7;   break;
        default:                                                   break;
	}
	return lstsw;
}

la_intf_card_t la_ipc_get_intf_card(char ipc_panel)
{
	la_intf_card_t card = LA_INTF_CARD_NULL;
	switch(ipc_panel)
	{
        case LA_IPC_PANEL0: card = LA_INTF_CARD0;     break;
        case LA_IPC_PANEL1: card = LA_INTF_CARD1;     break;
        case LA_IPC_PANEL2: card = LA_INTF_CARD2;     break;
        default:                                      break;
	}
    return card;
}


const char* la_ipc_playback_str(char ipc_playback)
{
	switch(ipc_playback)
	{
        case LA_IPC_PLAYBACK_LSTPLY0:     return "LSTPLY0";
        case LA_IPC_PLAYBACK_LSTPLY1:     return "LSTPLY1";
        case LA_IPC_PLAYBACK_LSTPLY2:     return "LSTPLY2";
        case LA_IPC_PLAYBACK_DMR:         return "DMR";
        case LA_IPC_PLAYBACK_MIRROR:      return "MIRROR LINK";
		case LA_IPC_PLAYBACK_NAVI:        return "NAVI";
		case LA_IPC_PLAYBACK_USB_CAP:     return "USB";
        case LA_IPC_PLAYBACK_HDMI_CAP:    return "HDMI";
        default:                    break;
	}
    return "unknown";
}

const char* la_ipc_panel_str(char ipc_panel)
{
	switch(ipc_panel)
	{
        case LA_IPC_PANEL0: return "panel 0";
        case LA_IPC_PANEL1: return "panel 1";
        case LA_IPC_PANEL2: return "panel 2";
        default:            break;
	}
     return "unknown";
}
*/
