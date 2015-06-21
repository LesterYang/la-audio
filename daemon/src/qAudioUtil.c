/*
 *  qAudioUtil.c
 *  Copyright © 2014  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : Utilities function
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <sys/errno.h>
#include "qAudioUtil.h"
#include "qAudioLog.h"


void la_init_head(la_list_t* head)
{
    head->next=NULL;
    head->prev=NULL;
}

void la_list_add(la_list_t *head, la_list_t* _new)
{
    __la_list_add(_new, head, head->next);
}

void la_list_add_tail(la_list_t *head, la_list_t* _new)
{
	while(head->next != NULL)
	{
		head = head->next;
	}

	head->next = _new;
	_new->prev = head;

	_new->next = NULL;
}

void la_list_del(la_list_t *entry)
{
    __la_list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

int la_positive_digit_to_integer(char* data)
{
    int result = 0;
    int idx = 0;

    while(isdigit(data[idx]))
    {
        result = 10*result + (data[idx] - '0');
        idx++;
    }
    
    return (idx != 0) ? result : -1;
}

int la_digit_3bytes_to_integer(unsigned char* data)
{
    int result = 0;
    int idx = 0;

    for(idx=0; idx<3; idx++)
    {
        if(!isdigit(data[idx]))
            return -1;
        result = 10*result + (data[idx] - '0');
    }

    return result;
}



void* la_malloc(size_t size)
{
    void *p;

    if (size > 0 && size < LA_MAX_ALLOC_SIZE){
    	if (!(p = malloc(size))){
            la_log_error("malloc error");
    		return NULL;
    	}
    }
    else{
    	la_log_error("allocation size error");
    	return NULL;
    }
    return p;
}

void* la_calloc(size_t size)
{
    void *p;

    if (size > 0 && size < LA_MAX_ALLOC_SIZE){
    	if (!(p =  calloc(1, size))){
    		la_log_error("calloc error");
    		return NULL;
    	}
    }
    else{
    	la_log_error("allocation size error");
    	return NULL;
    }
    return p;
}

void la_free(void *p)
{
    if (!p)
        return;
    free(p);
}

char* la_strdup(const char *s)
{
    if (s)
    {
    	int l = strlen(s);

    	if(s[l-1] == 0xa)
            l = ( l>=2 && s[l-2] == 0xd) ? l-2 : l-1;
        
        char *r = (char*)la_calloc(l+1);
        memcpy(r, s, l);
        return r;
    }
    else
        return NULL;
}

char *la_strlcpy(char *b, const char *s, size_t l)
{
    size_t k;
    k = strlen(s);
    if (k > l-1)
        k = l-1;
    memcpy(b, s, k);
    b[k] = 0;
    return b;
}

int la_str_index(char* s, char c)
{
    int idx;
    int len = strlen(s);

    for(idx = 0; idx < len; idx++)
    {
        if(s[idx] == c)
            break;
    }
    
    return (idx == len) ? -1 : idx;
}

ssize_t la_read(int fd, void *buf, size_t count)
{
    for (;;) {
        ssize_t r;
        if ((r = read(fd, buf, count)) < 0)
            if (errno == EINTR)
                continue;
        return r;
    }
}

ssize_t la_write(int fd, const void *buf, size_t count, int *type)
{
    if (!type || *type == 0) {
        ssize_t r;
        for (;;) {
            if ((r = send(fd, buf, count, MSG_NOSIGNAL)) < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }
            return r;
        }
        if (errno != ENOTSOCK)
            return r;
        if (type)
            *type = 1;
    }
    for (;;) {
        ssize_t r;
        if ((r = write(fd, buf, count)) < 0)
            if (errno == EINTR)
                continue;
        return r;
    }
}

ssize_t la_loop_read(int fd, void *data, size_t size)
{
    ssize_t ret = 0;

    while (size > 0) {
        ssize_t r;
        if ((r = la_read(fd, data, size)) < 0)
            return r;
        if (r == 0)
            break;
        ret += r;
        data = (uint8_t*) data + r;
        size -= (size_t) r;
    }
    return ret;
}

ssize_t la_loop_write(int fd, const void *data, size_t size, int *type)
{
    ssize_t ret = 0;
    int _type;

    if (!type) {
        _type = 0;
        type = &_type;
    }
    while (size > 0) {
        ssize_t r;
        if ((r = la_write(fd, data, size, type)) < 0)
            return r;

        if (r == 0)
            break;
        ret += r;
        data = (const uint8_t*) data + r;
        size -= (size_t) r;
    }
    return ret;
}

int la_close(int fd)
{
    for (;;) {
        int r;
        if ((r = close(fd)) < 0)
            if (errno == EINTR)
                continue;
        return r;
    }
}

static void* la_internal_thread_func(void *data)
{
    la_thread_t *t = (la_thread_t *)data;
    la_assert(t);

    t->id = pthread_self();
    la_atomic_inc(&t->running);
    t->thread_func(t->userdata);
    la_atomic_sub(2, &t->running);
    return NULL;
}


la_thread_t* la_thread_new(la_thread_func_t thread_func, void *data)
{
	la_assert(thread_func);

	la_thread_t *t;
	t=(la_thread_t *)la_malloc(sizeof(la_thread_t));
	t->thread_func = thread_func;
	t->userdata = data;
	t->joined = false;
	t->name = NULL;

	la_atomic_set(&t->running,0);
	if (pthread_create(&t->id, NULL, la_internal_thread_func, t) < 0) {
        la_free(t);
        la_log_error("thread_new error\n");
        return NULL;
    }
    la_atomic_inc(&t->running);
	return t;
}

void la_thread_free(la_thread_t *t)
{
	la_assert(t);
	pthread_detach(t->id);
    pthread_cancel(t->id);
	la_free(t->name);
	la_free(t);
}

void la_thread_wait_free(la_thread_t *t)
{
    la_assert(t);
    pthread_cancel(t->id);
    if(la_thread_join(t) != 0)
        la_log_warn("thread_join error");
    la_free(t->name);
    la_free(t);
}

int la_thread_join(la_thread_t *t)
{
	if (t->joined)
	        return -1;
	t->joined = true;
	return pthread_join(t->id, NULL);
}

void* la_thread_get_data(la_thread_t *t)
{
    la_assert(t);
    return t->userdata;
}

void la_thread_cancellation_point()
{
    pthread_testcancel();
}


la_mutex_t* la_mutex_new(bool recursive, bool inherit_priority)
{
	la_mutex_t *m;
	
	m = (la_mutex_t *)la_calloc(sizeof(la_mutex_t));

#if LA_ENABLE_POSIX_MUTEX
	int r;
	pthread_mutexattr_t attr;
	
	memset(&attr, 0, sizeof(pthread_mutexattr_t));

	la_assert(pthread_mutexattr_init(&attr) == 0);

	if (recursive)
		la_assert(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);

	if (inherit_priority)
		la_assert(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT) == 0);

    if ((r = pthread_mutex_init(&m->mutex, &attr))) {
        //back to normal mutexes.
        la_assert((r == ENOTSUP) && inherit_priority);

        la_assert(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_NONE) == 0);
        la_assert(pthread_mutex_init(&m->mutex, &attr) == 0);
    }
#endif

    return m;
}

void la_mutex_free(la_mutex_t*m)
{
	la_assert(m);
	
#if LA_ENABLE_POSIX_MUTEX
    if(pthread_mutex_destroy(&m->mutex) != 0)
        perror("mutex free error");
#endif
    la_free(m);
}

void la_mutex_lock(la_mutex_t* m)
{
	la_assert(m);
	
#if LA_ENABLE_POSIX_MUTEX
	la_assert(pthread_mutex_lock(&m->mutex) == 0);
#else
	int count = 0;
    
    while(m->mutex)
    {
        usleep(100);
        count++;
        if(count > 100000)
        {
            la_log_warn("ignore dead lock");
            break;
        }
    }
    m->mutex = true;
#endif
}


void la_mutex_unlock(la_mutex_t* m)
{
	la_assert(m);
	
#if LA_ENABLE_POSIX_MUTEX
	la_assert(pthread_mutex_unlock(&m->mutex) == 0);
#else
	 if(m->mutex == false)
         la_log_error("double unlock la_mutex");
    m->mutex = false;
#endif
}



char *la_path_get_filename(const char *p)
{
    char *fn;

    if (!p)
        return NULL;

    if ((fn = strrchr(p, '/')))
        return fn+1;

    return (char*) p;
}

