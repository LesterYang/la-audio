/*
 *  saf7741_i2c.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <saf7741.h>
#include "saf7741_i2c.h"

#define SAF7741_I2C_MAX_BUF_SIZE    (SAF7741_I2C_MAX_DATA_LEN + 5)

#define I2C_WRITE_FLAG  (!I2C_M_RD)
#define I2C_READ_FLAG   (I2C_M_RD)

struct saf7741_i2c {
    int fd;
    pthread_mutex_t* mutex;
};

static struct saf7741_i2c saf7741_i2c_data;

void saf7741_i2c_deinit_mutex()
{
    if(!saf7741_i2c_data.mutex)
        return;
    pthread_mutex_destroy(saf7741_i2c_data.mutex);
    free(saf7741_i2c_data.mutex);
    saf7741_i2c_data.mutex = NULL;
}


void saf7741_i2c_init_mutex()
{
    if(saf7741_i2c_data.mutex)
        saf7741_i2c_deinit_mutex();

    saf7741_i2c_data.mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

    if(!saf7741_i2c_data.mutex) 
        return;

    pthread_mutexattr_t attr;
    memset(&attr, 0, sizeof(pthread_mutexattr_t));
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);

    if(pthread_mutex_init(saf7741_i2c_data.mutex, &attr))
    {
        pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_NONE);
        pthread_mutex_init(saf7741_i2c_data.mutex, &attr);
    }
}

int saf7741_i2c_write(int addr, unsigned char* buf, size_t len)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msgs;
    unsigned char tx[SAF7741_I2C_MAX_BUF_SIZE] = {0};

    if(len > SAF7741_I2C_MAX_DATA_LEN)
        return -1;

    tx[0] = (addr>>16)&0xFF;
    tx[1] = (addr>>8)&0xFF;
    tx[2] = (addr)&0xFF;
    memcpy(&tx[3], buf, len);

    msgs.addr       = SAF7741_I2C_7BIT_DEV_ADDR;
    msgs.flags      = I2C_WRITE_FLAG;
    msgs.len        = len+3;
    msgs.buf        = tx;
    ioctl_data.nmsgs= 1;
    ioctl_data.msgs = &msgs;

    return (ioctl(saf7741_i2c_data.fd, I2C_RDWR, &ioctl_data) == 1) ? 0 : -1;
}

int saf7741_i2c_read(int addr, unsigned char* buf, size_t len)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msgs[2];
    unsigned char rx[SAF7741_I2C_REG_SIZE] = {0};


    rx[0] = (addr>>16)&0xFF;
    rx[1] = (addr>>8)&0xFF;
    rx[2] = (addr)&0xFF;

    msgs[0].addr    = SAF7741_I2C_7BIT_DEV_ADDR;
    msgs[0].flags   = 0;
    msgs[0].len     = SAF7741_I2C_REG_SIZE;
    msgs[0].buf     = rx;
    msgs[1].addr    = SAF7741_I2C_7BIT_DEV_ADDR;
    msgs[1].flags   = I2C_READ_FLAG;
    msgs[1].len     = len;
    msgs[1].buf     = buf;
    ioctl_data.nmsgs= 2;
    ioctl_data.msgs = msgs;

    return (ioctl(saf7741_i2c_data.fd, I2C_RDWR, &ioctl_data) == 2) ? 0 : -1;
}

int saf7741_i2c_open()
{
    if(saf7741_i2c_data.fd > 0)
        close(saf7741_i2c_data.fd);

    return (saf7741_i2c_data.fd = open(SAF7742_I2C_DEV, O_RDWR));
}

void saf7741_i2c_close()
{
    if(saf7741_i2c_data.fd > 0)
        close(saf7741_i2c_data.fd);
    saf7741_i2c_data.fd = -1;
}


