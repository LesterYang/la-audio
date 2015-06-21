#ifndef _QAUDIO_LOG_H
#define _QAUDIO_LOG_H

#define ENV_VAR_DEBUG_LEVEL "LA_DBG_LV"
#define ENV_VAR_DEBUG_BT    "LA_DBG_BT"

#define ENV_VAR_PRINT_TIME  "LA_LOG_TIME"
#define ENV_VAR_PRINT_FUNC  "LA_LOG_FUNC"
#define ENV_VAR_PRINT_META  "LA_LOG_META"
#define ENV_VAR_PRINT_LEVEL "LA_LOG_LEVEL"

#define LA_LOG_BT_FRAME_MAX (8)

#define la_log_debug(...)  la_log_level(LA_LOG_DEBUG,  __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define la_log_info(...)   la_log_level(LA_LOG_INFO,   __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define la_log_notice(...) la_log_level(LA_LOG_NOTICE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define la_log_warn(...)   la_log_level(LA_LOG_WARN,   __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define la_log_error(...)  la_log_level(LA_LOG_ERROR,  __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

typedef enum _la_errno la_errno_t;
typedef enum _la_log_target la_log_target_t;
typedef enum _la_log_level la_log_level_t;
typedef enum _la_log_flags la_log_flags_t;
typedef struct _la_log la_log_t;

enum _la_errno{
    LA_ERRNO_SUCCESS    =  0,       // No error
    LA_ERRNO_NO_DEV     = -1,       // No such device or device doesn't initialize
    LA_ERRNO_ALLOC      = -2,       // Allocate error
    LA_ERRNO_OPEN_PCM   = -3,       // Open pcm error
    LA_ERRNO_OPEN_FIFO  = -4,       // Open fifo error
    LA_ERRNO_MASTER_DEV = -5,       // Can't set master device
    LA_ERRNO_DEV_BUSY   = -6,       // Device busy
    LA_ERRNO_SHELL      = -7,       // Shell command error

	LA_ERRNO_OTHER      = -127      // Other error
};


enum _la_log_target {
    LA_LOG_STDERR,                  // default 
    LA_LOG_SYSLOG,
    LA_LOG_NULL,                    // to /dev/null 
    LA_LOG_FD,                      // to a file descriptor, e.g. a char device 
    LA_LOG_TARGET_MAX
};

enum _la_log_level {
    LA_LOG_ERROR  = 0,              // Error messages 
    LA_LOG_WARN   = 1,              // Warning messages 
    LA_LOG_NOTICE = 2,              // Notice messages 
    LA_LOG_INFO   = 3,              // Info messages 
    LA_LOG_DEBUG  = 4,              // debug message 
    LA_LOG_LEVEL_MAX
};

enum _la_log_flags {
    LA_LOG_PRINT_NONE  = 0x01,      // Show nothing
    LA_LOG_PRINT_TIME  = 0x02,      // Show time 
    LA_LOG_PRINT_FUNC  = 0x04,      // Show function 
    LA_LOG_PRINT_META  = 0x08,      // Show extended location information 
    LA_LOG_PRINT_LEVEL = 0x10       // Show log level prefix 
};

struct _la_log
{
    int             fd;
    la_log_target_t target;
    la_log_level_t  lv;
    la_log_flags_t  flag;
    int             backtrace;
};


void la_log_init(void);
void la_log_deinit(void);
void la_log_set_level(la_log_level_t l);
void la_log_set_target(la_log_target_t t);
void la_log_set_backtrace(int frames);
void la_log_enable_flag(la_log_flags_t flag);
void la_log_disable_flag(la_log_flags_t flag);
void la_log_open_target_file(const char* file);


void la_log_level(la_log_level_t lv, const char* file, int line, const char *func, const char *fmt, ...);

const char* la_err_str(la_errno_t no);

void la_log_show_status(void);
void la_log_show_card_status(void);
void la_log_show_playback_status(void);
void la_log_show_capture_status(void);


#endif /* _QAUDIO_LOG_H */

