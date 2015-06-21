/*
 *  tef7000_i2c.c
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


#include "tef7000_i2c.h"

#define TEF7000_I2C_MAX_W_BUF_SIZE    (TEF7000_I2C_MAX_W_DATA_LEN + 2)
#define TEF7000_I2C_MAX_R_BUF_SIZE    (TEF7000_I2C_MAX_R_DATA_LEN + 2)

#define I2C_WRITE_FLAG  (!I2C_M_RD)
#define I2C_READ_FLAG   (I2C_M_RD)


struct tef7000_i2c {
    int fd;
};

static struct tef7000_i2c tef7000_i2c_data;


int tef7000_i2c_write(unsigned char msa, unsigned char* buf, size_t len)
{    
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msgs;
    unsigned char tx[TEF7000_I2C_MAX_W_BUF_SIZE] = {0};

    if(len > TEF7000_I2C_MAX_W_DATA_LEN)
        return -1;

    tx[0] = msa;
    memcpy(&tx[1], buf, len);

    msgs.addr       = TEF7000_I2C_7BIT_DEV_ADDR;
    msgs.flags      = I2C_WRITE_FLAG;
    msgs.len        = len+1;
    msgs.buf        = tx;
    ioctl_data.nmsgs= 1;
    ioctl_data.msgs = &msgs;

#if 0
        printf("tef7000_i2c_write : %02x ", TEF7000_I2C_7BIT_DEV_ADDR<<1);
        for(int i=0; i<msgs.len; i++)
            printf("%02x ", msgs.buf[i]);
        printf("\n");
#endif

    return (ioctl(tef7000_i2c_data.fd, I2C_RDWR, &ioctl_data) == 1) ? 0 : -1;
}

int  tef7000_i2c_read(unsigned char* buf, size_t len)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msgs;

    msgs.addr    = TEF7000_I2C_7BIT_DEV_ADDR;
    msgs.flags   = I2C_READ_FLAG;
    msgs.len     = len;
    msgs.buf     = buf;
    ioctl_data.nmsgs= 1;
    ioctl_data.msgs = &msgs;

    return (ioctl(tef7000_i2c_data.fd, I2C_RDWR, &ioctl_data) == 1) ? 0 : -1;
}

int  tef7000_i2c_open(void)
{
    if(tef7000_i2c_data.fd > 0)
        close(tef7000_i2c_data.fd);

    return (tef7000_i2c_data.fd = open(TEF7000_I2C_DEV, O_RDWR));
}

void tef7000_i2c_close(void)
{
    if(tef7000_i2c_data.fd > 0)
        close(tef7000_i2c_data.fd);
    tef7000_i2c_data.fd = -1;
}

