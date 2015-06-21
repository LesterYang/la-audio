/*
 *  qAudioMainLoop.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

//#include <gperftools/profiler.h>

#include "qAudioCore.h"
#include "qAudioMainLoop.h"

#include "../ext/qExtDsp.h"

enum {
	OPT_VERSION = 1,
	OPT_PERIOD_SIZE,
	OPT_BUFFER_SIZE,
	OPT_DISABLE_RESAMPLE,
	OPT_DISABLE_CHANNELS,
	OPT_DISABLE_FORMAT,
	OPT_DISABLE_SOFTVOL,
	OPT_TEST_POSITION,
	OPT_TEST_COEF,
	OPT_TEST_NOWAIT,
	OPT_MAX_FILE_TIME,
	OPT_PROCESS_ID_FILE,
	OPT_USE_STRFTIME,

	OPT_DBG_TIME,
	OPT_DBG_FUNC,
	OPT_DBG_META,
	OPT_DBG_LEVEL,
};



struct la_status_info la_status[] = {
    {LA_STATUS_NONE,        "None"},
    {LA_STATUS_DAEMON_INIT, "DaemonInit"},
    {LA_STATUS_IPC_INIT,    "IPCInit"},
    {LA_STATUS_RUNNING,     "Running"},
    {LA_STATUS_DEINIT,      "Deinit"},
    {LA_STATUS_REINIT,      "Reinit"},
    {LA_STATUS_ERROR,       "Error"},
    {LA_STATUS_EXIT,        "Exit"},
    {-1,                    NULL},
};

struct option long_opts[] = {
    {"help",                0, 0,   'h'},
    {"file",                1, 0,   'f'},
    {"silence",             0, 0,   's'},
    {"debug",               1, 0,   'd'},
    {"backtrace",           1, 0,   'b'},
    {"client",              1, 0,   'c'},
    {"verbose",             0, 0,   'v'},
    {"version",             0, 0,   OPT_VERSION},
    {"show-time",           0, 0,   OPT_DBG_TIME},
    {"show-func",           0, 0,   OPT_DBG_FUNC},
    {"show-meta",           0, 0,   OPT_DBG_META},
    {"show-level",          0, 0,   OPT_DBG_LEVEL},
    {"enable-dsp",          0, 0,   'e'},
    {0,                     0, 0,   0}
};

static la_mainloop_t la_mainloop;

void la_usage(const char* arg)
{
    fprintf(stderr, "Usage : %s [OPTIONS ...]\n"
                    "OPTIONS\n"
                    "   -c, --client     NAME     opne NAME IPC-client\n"
                    "   -v, --verbose             show PCM structure\n"
                    "       --version             show version\n"
                    "   -d, --debug      LV       set debug level(0~4)[default:2]\n"
                    "   -b, --backtrace  NUM      show NUM frames[default:0]\n"
                    "   -f, --file       FILE     send output to FILE\n"
                    "   -s, --silence             send output to /dev/null\n"
                    "       --show-time           print time\n"
                    "       --show-func           print function name\n"
                    "       --show-meta           print more information\n"
                    "       --show-level          print debug level\n"
                    "   -e  --enable-dsp          enable extern DSP\n"
                    "   -h, --help                help\n"
                    ,arg);
    exit(0);
}

void la_version()
{
    la_log_warn("version %s", QAUDIO_VERSION);
    exit(0);
}

void la_switch_status(la_status_t status)
{
     la_log_info("status, %10s -> %s", 
                    la_strnull(la_status[la_mainloop.status].str), 
                    la_strnull(la_status[status].str));
     la_mainloop.status = status;
}

static void signal_handler(int sig)
{
    la_deinit();
    if(la_mainloop.enable_dsp)
        qext_dsp_deinit();
    exit(1);
}

la_status_t la_mainloop_get_status()
{
    return la_mainloop.status;
}

void la_mainloop_set_status(la_status_t status)
{
    if(status < LA_STATUS_MAX && status > LA_STATUS_NONE)
        la_switch_status(status);
}


void pcm_test()
{
    unsigned char msg[]="010";
    
    la_log_notice("add pcm 0 on panel 1");
    la_ipc_recv("test", 3, msg);
	
    la_log_notice("add pcm 1 on panel 1");
    msg[2]='1';
    la_ipc_recv("test", 3, msg);
	
    la_log_notice("add pcm 2 on panel 1");
    msg[2]='2';
    la_ipc_recv("test", 3, msg);
	
    la_log_notice("add pcm 2 on panel 1");
    msg[2]='2';
    la_ipc_recv("test", 3, msg);  
	
    la_log_notice("add pcm 2 on panel 0");
    msg[1]='0';
    la_ipc_recv("test", 3, msg);
	
    la_log_notice("add pcm 0 on panel 0");
    msg[2]='0';
    la_ipc_recv("test", 3, msg);

#if 0
    la_log_notice("start record 0");
    msg[0]='a';
    msg[1]='0';
    la_ipc_recv("test", 2, msg);

    sleep(5);

    la_log_notice("stop record 0");
    msg[0]='b';
    la_ipc_recv("test", 2, msg);
#endif
    return;
}

int main(int argc, char *argv[])
{
    int opt_idx, c;
    size_t len;
    char *short_opts = "hc:vf:sd:b:e";
    bool verbose = false;

    la_log_init();

    while ((c = getopt_long(argc, argv, short_opts, long_opts, &opt_idx)) != -1)
    {
        switch(c)
        {
            case 'h':
                la_usage(argv[0]);
                break;

            case 'f':
                la_log_set_target(LA_LOG_FD);
                la_log_open_target_file(optarg);
                break;

            case 's':
                la_log_set_target(LA_LOG_NULL);
                break;
                
            case 'd':
                la_log_set_level((la_log_level_t)atoi(optarg));
                break;
                
            case 'b':
                la_log_set_backtrace(atoi(optarg));
                break;
                
            case 'c':
                if((len = strlen(optarg)) < IPC_MAX_NAME)
                    memcpy(la_mainloop.client, optarg, len);
                break;

            case OPT_VERSION:
                la_version();
                break;

            case 'v':
                verbose = true;
                break;

            case OPT_DBG_TIME:
                la_log_enable_flag(LA_LOG_PRINT_TIME);
                break;
                
            case OPT_DBG_FUNC:
                la_log_enable_flag(LA_LOG_PRINT_FUNC);
                break;
                
            case OPT_DBG_META:
                la_log_enable_flag(LA_LOG_PRINT_META);
                break;
                
            case OPT_DBG_LEVEL:
                la_log_enable_flag(LA_LOG_PRINT_LEVEL);
                break;

            case 'e':
                la_mainloop.enable_dsp = true;
                break;
                
            default:
                break;
        }
    }

    while(la_mainloop.status != LA_STATUS_EXIT)
    {
        switch(la_mainloop.status)
        {
            case LA_STATUS_NONE:
                la_log_notice("version %s", QAUDIO_VERSION);
                la_switch_status(LA_STATUS_DAEMON_INIT);
                break;

            case LA_STATUS_DAEMON_INIT:
                if(la_init() == LA_ERRNO_SUCCESS)
                {
                    if(verbose)
                        la_show_pcm_structure();
                    
                    signal(SIGINT, signal_handler);
                    signal(SIGTERM, signal_handler);

                    //pcm_test();
                    la_switch_status(LA_STATUS_IPC_INIT);
                }
                else
                   sleep(LA_MAINLOOP_DELAY);
                break;

            case LA_STATUS_IPC_INIT:
            	if(!la_ipc_open(la_mainloop.client))
                { 
                    if(la_mainloop.enable_dsp && qext_dsp_init() != 0)
                        la_mainloop.enable_dsp = false;
                    la_switch_status(LA_STATUS_RUNNING);
                }
                else
            	    sleep(LA_MAINLOOP_DELAY);
                break;

            case LA_STATUS_RUNNING:
                sleep(LA_MAINLOOP_DELAY);
                break;

            case LA_STATUS_DEINIT:
                la_deinit();
                la_ipc_close();
      			la_switch_status(LA_STATUS_EXIT);
                break;

            case LA_STATUS_REINIT:
                la_switch_status(LA_STATUS_DAEMON_INIT);
                break;

            case LA_STATUS_ERROR:
                la_switch_status(LA_STATUS_DEINIT);
                break;

            case LA_STATUS_EXIT:
                break;

            default:
                la_switch_status(LA_STATUS_ERROR);
                break;
        };
    }

    la_log_deinit();
    return 0;
}
