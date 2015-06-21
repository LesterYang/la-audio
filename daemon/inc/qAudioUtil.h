#ifndef _QAUDIO_UTIL_H
#define _QAUDIO_UTIL_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>


typedef struct _la_list         la_list_t;
typedef struct _la_mutex        la_mutex_t;
typedef struct _la_thread       la_thread_t;
typedef struct _la_atomic       la_atomic_t;
typedef struct _la_atomic_ptr   la_atomic_ptr_t;
typedef struct timeval          la_timeval_t;
typedef struct timespec         la_timespec_t;
typedef struct tm               la_time_info_t;
typedef time_t                  la_time_t;


typedef void (*la_thread_func_t) (void *data);

// debug

#define LA_ENABLE_POSIX_MUTEX (1)

#define LA_ELEMENTSOF(x) (sizeof(x)/sizeof((x)[0]))

#ifdef __GNUC__
#define LA_MAX(a,b)                             \
    __extension__ ({                            \
            typeof(a) _a = (a);                 \
            typeof(b) _b = (b);                 \
            _a > _b ? _a : _b;                  \
        })
#else
#define LA_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifdef __GNUC__
#define LA_MIN(a,b)                             \
    __extension__ ({                            \
            typeof(a) _a = (a);                 \
            typeof(b) _b = (b);                 \
            _a < _b ? _a : _b;                  \
        })
#else
#define LA_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define LA_MAX_ALLOC_SIZE (1024*1024*64)  //64MB

#ifdef __GNUC__
#define LA_LIKELY(x) (__builtin_expect(!!(x),1))
#define LA_UNLIKELY(x) (__builtin_expect(!!(x),0))
#define LA_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define LA_LIKELY(x) (x)
#define LA_UNLIKELY(x) (x)
#define LA_PRETTY_FUNCTION ""
#endif

#define la_nothing() do {} while (false)

#define LA_BIT_SET(var, bits)	((var) |= (bits))
#define LA_BIT_CLR(var, bits)	((var) &= ~(bits))
#define LA_BIT_AND(var, bits)	((var) &= (bits))
#define LA_BIT_VAL(var, bits)	((var) & (bits))
#define LA_GET_BITS(var, bits)	((var) & ((1ULL << bits)-1)


#ifdef LA_ASSERT
#define la_assert(expr)                                                                         \
    do {                                                                                        \
        if (LA_UNLIKELY(!(expr))) {                                                             \
            fprintf(stderr, "lst-audio : Expr '%s' failed at %s:%u, function '%s'. Aborting\n", \
                        #expr , __FILE__, __LINE__, LA_PRETTY_FUNCTION);                        \
            abort();                                                                            \
        }                                                                                       \
    } while (0)
#else
#define la_assert(expr) la_nothing()
#endif

#if 1
#define la_dbg(lv, expr, ...)                       \
do {							                    \
		fprintf(stderr, "lst-audio : ");			\
		fprintf(stderr, expr,  ##__VA_ARGS__);		\
		fprintf(stderr, "\n");					    \
} while (0)
#else

#define la_dbg(lv, expr, ...) do {printf(expr,  ##__VA_ARGS__);} while (false)
#endif

// ===============================================
// list
// ===============================================
#define LIST_POISON1  ((void*)0x00100100)
#define LIST_POISON2  ((void*)0x00200200)

#define la_offsetof(s, m) (size_t)&(((s *)0)->m)

#define la_container_of(ptr, type, member)                                  \
({                                                                         \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);                   \
    (type *)( (char *)__mptr - la_offsetof(type,member) );                  \
})

#define list_first_entry(head_ptr, type, member) 						   \
		((head_ptr)->next) ? la_container_of((head_ptr)->next, type, member) : NULL

#define list_next_entry(pos, member)                                       \
        la_container_of((pos)->member.next, typeof(*(pos)), member)

#define list_next_entry_or_null(pos, member)                               \
        ((pos)->member.next) ? list_next_entry(pos, member) : NULL

#define list_prev_entry(pos, member)                                       \
        la_container_of((pos)->member.prev, typeof(*(pos)), member)

#define list_prev_entry_or_null(pos, member)                               \
        ((pos)->member.prev) ? list_prev_entry(pos, member) : NULL

#define list_for_each_entry(head_ptr, pos, member)                         \
     for (pos = list_first_entry(head_ptr, typeof(*pos), member);          \
          pos != NULL;                                                     \
          pos = list_next_entry_or_null(pos, member))

#define list_for_each_entry_reverse(last_ptr, pos, member)                 \
     for (pos = la_container_of(last_ptr, typeof(*pos), member);            \
          pos != NULL;                                                     \
          pos = list_prev_entry_or_null(pos, member))


		 
struct _la_list{
    la_list_t *next,*prev;
};

void la_init_head(la_list_t* head);
void la_list_add(la_list_t *_new, la_list_t* head);
void la_list_add_tail(la_list_t *_new, la_list_t* head);
void la_list_del(la_list_t *entry);


// ===============================================
// tranfer function
// ===============================================
int la_positive_digit_to_integer(char* data);
int la_digit_3bytes_to_integer(unsigned char* data);



// ===============================================
// allocate and free
// ===============================================
void* la_malloc(size_t size);
void* la_calloc(size_t size);
void  la_free(void *p);
char* la_strdup(const char *s);


// ===============================================
// sting
// ===============================================
char *la_strlcpy(char *b, const char *s, size_t l);
int la_str_index(char* s, char c);


// ===============================================
// read and write
// ===============================================
ssize_t la_read(int fd, void *buf, size_t count);
ssize_t la_write(int fd, const void *buf, size_t count, int *type);
ssize_t la_loop_read(int fd, void *data, size_t size);
ssize_t la_loop_write(int fd, const void*data, size_t size, int *type);


// ===============================================
// file
// ===============================================
int la_close(int fd);

// ===============================================
// atomic
// ===============================================
struct _la_atomic {
    volatile int value;
};
struct _la_atomic_ptr {
    volatile unsigned long value;
};

#define LA_ATOMIC_INIT(i)	{ (i) }
#define LA_ATOMIC_PTR_INIT(v) { .value = (long) (v) }

#ifdef Q_ARM_A8

static inline void la_memory_barrier(void) {
    asm volatile ("mcr  p15, 0, r0, c7, c10, 5  @ dmb");
}


static inline int la_atomic_read(const la_atomic_t *a) {
    la_memory_barrier();
    return a->value;
}

static inline void la_atomic_set(la_atomic_t *a, int i) {
    a->value = i;
    la_memory_barrier();
}

/* Returns the previously set value */
static inline int la_atomic_sub( int i, la_atomic_t *a) {
    unsigned long not_exclusive;
    int new_val, old_val;

    la_memory_barrier();
    do {
        asm volatile ("ldrex    %0, [%3]\n"
                      "sub      %2, %0, %4\n"
                      "strex    %1, %2, [%3]\n"
                      : "=&r" (old_val), "=&r" (not_exclusive), "=&r" (new_val)
                      : "r" (&a->value), "Ir" (i)
                      : "cc");
    } while(not_exclusive);
    la_memory_barrier();

    return old_val;
}

static inline int la_atomic_inc(la_atomic_t *a) {
    return la_atomic_add(1, a);
}

static inline int la_atomic_dec(la_atomic_t *a) {
    return la_atomic_sub(1, a);
}

static inline void* la_atomic_ptr_load(const la_atomic_ptr_t *a) {
    la_memory_barrier();
    return (void*) a->value;
}

static inline void la_atomic_ptr_store(la_atomic_ptr_t *a, void *p) {
    a->value = (unsigned long) p;
    la_memory_barrier();
}

#else
static inline int la_atomic_read(const la_atomic_t *v)
{
    return (*(volatile int *)&(v)->value);
}
static inline void la_atomic_set(la_atomic_t *v, int i)
{
    v->value = i;
}
static inline void la_atomic_add(int i, la_atomic_t *v)
{
    v->value += i;
}
static inline void la_atomic_sub(int i, la_atomic_t *v)
{
    v->value -= i;
}
static inline void la_atomic_inc(la_atomic_t *v)
{
    v->value++;
}
static inline void la_atomic_dec(la_atomic_t *v)
{
    v->value--;
}
#endif


// ===============================================
// thread
// ===============================================


struct _la_thread {
    pthread_t        id;
    la_thread_func_t thread_func;
    void*            userdata;
    bool             joined;
    la_atomic_t      running;
	char*            name;
};


la_thread_t*   la_thread_new(la_thread_func_t thread_func, void *userdata);
void        la_thread_free(la_thread_t *t);
void        la_thread_wait_free(la_thread_t *t);
int         la_thread_join(la_thread_t *t);
void*       la_thread_get_data(la_thread_t *t);
void        la_thread_cancellation_point();


// ===============================================
// mutex
// ===============================================

struct _la_mutex {
#if LA_ENABLE_POSIX_MUTEX
    pthread_mutex_t mutex;
#else
    volatile bool mutex;
#endif
};

la_mutex_t* la_mutex_new(bool recursive, bool inherit_priority);
void 	    la_mutex_free(la_mutex_t *m);
void 	    la_mutex_lock(la_mutex_t *m);
void 	    la_mutex_unlock(la_mutex_t *m);

// ===============================================
// real time clock
// ===============================================


// ===============================================
// other
// ===============================================
char *la_path_get_filename(const char *p);


// ===============================================
// inline function
// ===============================================

static inline void __la_list_add(la_list_t *_new, la_list_t *prev, la_list_t *next)
{
    if(next)
        next->prev=_new;
    if(prev)
        prev->next=_new;

    _new->next=next;
    _new->prev=prev;
}

static inline void __la_list_del(la_list_t *prev, la_list_t *next)
{
	if(next)
		next->prev = prev ;
	else
		next = NULL;

	if(prev)
		prev->next = next;
	else
		prev = NULL;
}

static inline const char *la_strnull(const char *x)
{
    return x ? x : "(null)";
}

static inline const char *la_strunknown(const char *x)
{
    return x ? x : "(unknown)";
}

static inline const char *la_strempty(const char *x) 
{
    return x ? x : "";
}

static inline void la_get_clock_time(la_timespec_t *clock)
{
    clock_gettime(CLOCK_REALTIME, clock);
}

static inline void la_get_time(la_timeval_t *time)
{
    gettimeofday(time, NULL);
}


#endif /* _QAUDIO_UTIL_H */
