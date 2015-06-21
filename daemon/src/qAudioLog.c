/*
 *  qAudioLog.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include <stdarg.h>
#include <execinfo.h>
#include "qAudioLog.h"
#include "qAudioMainLoop.h"
#include "qAudioCard.h"
#include "qAudioPlayback.h"
#include "qAudioCapture.h"
#include "qAudioCore.h"

static const char level_to_char[] = {
    [LA_LOG_ERROR] = 'E',
    [LA_LOG_WARN] = 'W',
    [LA_LOG_NOTICE] = 'N',
    [LA_LOG_INFO] = 'I',
    [LA_LOG_DEBUG] = 'D'
};


static la_log_t log;

void la_log_set_fd(int fd);
size_t la_vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
size_t la_snprintf(char *str, size_t size, const char *fmt, ...);
char* la_log_allocate_backtrace(int frames);




void la_log_init(void)
{
    const char *env_var;

    log.target = LA_LOG_STDERR;
    log.fd = -1;
	log.flag = 0;
	log.backtrace = 0;

	if ((env_var = getenv(ENV_VAR_DEBUG_LEVEL))) 
	{
		if((log.lv = (la_log_level_t) atoi(env_var)) >= LA_LOG_LEVEL_MAX)
			log.lv = LA_LOG_LEVEL_MAX-1;
	}
	else
	{
		log.lv = LA_LOG_NOTICE;
	}

	if ((env_var = getenv(ENV_VAR_DEBUG_BT)))
	{
		if((log.backtrace = atoi(env_var)) < 0)
			log.backtrace = 0;	 
 	}

    if ((env_var = getenv(ENV_VAR_PRINT_TIME)) && atoi(env_var) != 0)
        log.flag |= LA_LOG_PRINT_TIME;

    if ((env_var = getenv(ENV_VAR_PRINT_FUNC)) && atoi(env_var) != 0)
        log.flag |= LA_LOG_PRINT_FUNC;

    if ((env_var = getenv(ENV_VAR_PRINT_META)) && atoi(env_var) != 0)
        log.flag |= LA_LOG_PRINT_META;

    if ((env_var = getenv(ENV_VAR_PRINT_LEVEL)) && atoi(env_var) != 0)
        log.flag |= LA_LOG_PRINT_LEVEL;

	
}

void la_log_deinit(void)
{
    if(log.fd >= 0)
        la_close(log.fd);

}

void la_log_set_fd(int fd) 
{
    if (fd >= 0 && log.target == LA_LOG_FD)
    {
        if(log.fd >= 0)
            la_close(log.fd);
        log.fd = fd;
    }
}

void la_log_set_level(la_log_level_t level) 
{
    if(level < LA_LOG_LEVEL_MAX)
        log.lv = level;
}

void la_log_set_target(la_log_target_t target) 
{
    if(target < LA_LOG_TARGET_MAX)
        log.target = target;
}

void la_log_set_backtrace(int frames) 
{
    if(frames > 0)
        log.backtrace = (frames > LA_LOG_BT_FRAME_MAX) ? LA_LOG_BT_FRAME_MAX : frames;
}

void la_log_enable_flag(la_log_flags_t flag)
{
    LA_BIT_SET(log.flag, flag);
}

void la_log_disable_flag(la_log_flags_t flag)
{
    LA_BIT_CLR(log.flag, flag);
}

void la_log_open_target_file(const char* file)
{
    int fd;
    if( (fd = open(file, O_RDWR | O_NONBLOCK)) >= 0 )
    {
        la_log_set_fd(fd);
    }
}

const char* la_err_str(la_errno_t no)
{
    switch(no)
    {
        case LA_ERRNO_SUCCESS:      return " Success";
        case LA_ERRNO_NO_DEV:       return " No such device";
        case LA_ERRNO_ALLOC:        return " Allocate error";
        case LA_ERRNO_OPEN_PCM:     return " Open pcm error";
        case LA_ERRNO_OPEN_FIFO:    return " Open fifo error";
        case LA_ERRNO_MASTER_DEV:   return " Can't set master device";
        case LA_ERRNO_DEV_BUSY:     return " Device busy";
	    case LA_ERRNO_OTHER:        return " Other error";
	    case LA_ERRNO_SHELL:        return " Shell command error";
        default:            break;
    }
    return "unknown";
}


void la_log_print(la_log_level_t lv, const char* file, int line, const char *func, const char *fmt, va_list ap)
{
	char text[1024], location[128], timestamp[24];
	char *msg, *newline;
    char* bt = NULL;

    la_vsnprintf(text, sizeof(text), fmt, ap);

    if (((log.flag & LA_LOG_PRINT_META) || lv == LA_LOG_ERROR) && file && line > 0 && func)
        la_snprintf(location, sizeof(location), "%s:%i %s() ", file, line, func);
    else if (((log.flag & (LA_LOG_PRINT_META|LA_LOG_PRINT_FUNC)) || lv == LA_LOG_ERROR) && func)
        la_snprintf(location, sizeof(location), "%s() ", func);
    else
	    location[0] = 0;

    if (log.flag & LA_LOG_PRINT_TIME)
    {
        la_time_t lt = time(NULL);
        la_time_info_t* info = localtime(&lt);;   
                
        la_snprintf(timestamp, sizeof(timestamp), "%d/%02d/%02d %02d:%02d:%02d "
                                                ,info->tm_year + 1900
                                                ,info->tm_mon + 1
                                                ,info->tm_mday
                                                ,info->tm_hour
                                                ,info->tm_min
                                                ,info->tm_sec);
     } 
    else
        timestamp[0] = 0;

    if (log.backtrace> 0)
        bt = la_log_allocate_backtrace(log.backtrace);


    for (msg = text; msg; msg = newline) 
    {
        if ((newline = strchr(msg, '\n'))) 
        {
            *newline = 0;
            newline++;
        }

        //ignore strings only made out of whitespace
        if (msg[strspn(msg, "\t ")] == 0)
            continue;

        switch(log.target)
        {
            case LA_LOG_STDERR:
                if (log.flag & LA_LOG_PRINT_LEVEL)
                    fprintf(stderr, "lst-audio : %s[%c] %s%s%s\n", timestamp, level_to_char[log.lv], location, msg, la_strempty(bt));
                else
                    fprintf(stderr, "lst-audio : %s%s%s%s\n", timestamp, location, msg, la_strempty(bt));
                break;
                    
            case LA_LOG_SYSLOG:
                break;
                        
            case LA_LOG_FD:
                break;
                        
            default:
                break;
        }
    }

    la_free(bt);
}

void la_log_level(la_log_level_t lv, const char* file, int line, const char *func, const char *fmt, ...)
{
	if(lv <= log.lv && log.target != LA_LOG_NULL)
	{
	    va_list ap;
	    
	    va_start(ap, fmt);
	    la_log_print(lv, file, line, func, fmt, ap);
	    va_end(ap);
	}
}

size_t la_vsnprintf(char *str, size_t size, const char *fmt, va_list ap) 
{
    int ret;

    la_assert(str);
    la_assert(size > 0);
    la_assert(fmt);

    ret = vsnprintf(str, size, fmt, ap);

    str[size-1] = 0;

    if (ret < 0)
        return strlen(str);

    if ((size_t) ret > size-1)
        return size-1;

    return (size_t) ret;
}

size_t la_snprintf(char *str, size_t size, const char *fmt, ...)
{
    size_t ret;
    va_list ap;

    la_assert(str);
    la_assert(size > 0);
    la_assert(fmt);

    va_start(ap, fmt);
    ret = la_vsnprintf(str, size, fmt, ap);
    va_end(ap);

    return ret;
}

char* la_log_allocate_backtrace(int frames)
{
    void* trace[32];
    int num_addr, num_frame, idx, amount;
    char **symbols, *bt, *ptr;	

    num_addr = backtrace(trace, LA_ELEMENTSOF(trace));

    if (num_addr <= 0)
        return NULL;

    symbols = backtrace_symbols(trace, num_addr);

    if (!symbols)
        return NULL;

    num_frame = LA_MIN(num_addr, frames);

    amount = 4;

    for (idx=0; idx<num_frame; idx++)
    {
        amount += 2;
        amount += strlen(la_path_get_filename(symbols[idx]));
    }

    bt = (char*)la_calloc(amount * sizeof(char));

    if(!bt)
        return NULL;

    strcpy(bt, " (");
    ptr = bt + 2;

    for (idx=0; idx<num_frame; idx++)
    {
        const char *sym;

        strcpy(ptr, "<-");
        ptr += 2;

        sym = la_path_get_filename(symbols[idx]);

        strcpy(ptr, sym);
        ptr += strlen(sym);
    }

    strcpy(ptr, ")");

    la_free(symbols);

    return bt;
}



void la_log_show_status()
{
    la_log_show_card_status();
    la_log_show_playback_status();
    la_log_show_capture_status();
}

void la_log_show_card_status()
{
    int idx;
    la_card_t* card;

    la_log_notice("=========== panel status ===========");

    list_for_each_entry(la_get_panel_head(), card, node)
    {
        if(card->playback_num == 0)
            la_log_notice(" %16s -> (null)", card->name);
        else
            la_log_notice(" %16s -> [0] %s ", card->name, ((la_playback_t*)card->playback_list[0])->name);
        
        for(idx=1; idx<card->playback_num; idx++)
        {
            if(card->playback_list[idx] != NULL)
                la_log_notice("                  -> [%d] %s", idx,((la_playback_t*)card->playback_list[idx])->name);
        }
    }
}

void la_log_show_playback_status()
{
    la_playback_t* playback;

    la_log_notice("========= playback status =========");
    
    list_for_each_entry(la_get_playback_head(), playback, node)
    {
        int idx = 0;
        
        if(playback->card_num == 0 || playback->card_list[0] == NULL || playback->card_list[0]->card == NULL)
        {
            if(playback->master_mute)
                la_log_notice(" %16s -> (null)", playback->name);
            else
                la_log_notice(" %16s -> [M] %s", playback->name, la_card_get_card(playback->master_panel)->name);
            continue;
        }
        else
        {
            if(playback->master_mute)
            {
                idx++;
                la_log_notice(" %16s -> [0] %s", playback->name, ((la_card_t*)playback->card_list[0]->card)->name);
            }
            else
                la_log_notice(" %16s -> [M] %s", playback->name, la_card_get_card(playback->master_panel)->name);
        }

        do
        {
            if(playback->card_list[idx] && playback->card_list[idx]->card)
                la_log_notice("                  -> [%d] %s", idx,((la_card_t*)playback->card_list[idx]->card)->name);
        }while(++idx < playback->card_num);
    }
}


void la_log_show_capture_status()
{
	la_capture_t* capture;

    la_log_notice("========== capture status =========");
		
	list_for_each_entry(la_get_capture_head(), capture, node)
	{
        la_log_notice(" %16s -> %s", capture->name, (capture->connect) ? "recoding" : "idle");
	}
}



