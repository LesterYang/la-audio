#ifndef _SAF7741_I2C_H
#define _SAF7741_I2C_H

#include <sys/types.h>

#define SAF7742_I2C_DEV             "/dev/i2c-1"
#define SAF7741_I2C_REG_SIZE        (3)
#define SAF7741_I2C_7BIT_DEV_ADDR   (0x1c)

#define SAF7741_I2C_MAX_DATA_LEN    (25)


int saf7741_i2c_write(int addr, unsigned char* buf, size_t len);
int saf7741_i2c_read(int addr, unsigned char* buf, size_t len);
int saf7741_i2c_open(void);
void saf7741_i2c_close(void);

inline static void i_to_2uc(unsigned char* buf, int val)
{
    buf[0] = (unsigned char)((val>>8)  & 0xFF);
    buf[1] = (unsigned char)((val)     & 0xFF);
}

inline static void i_to_3uc(unsigned char* buf, int val)
{
    buf[0] = (unsigned char)((val>>16) & 0xFF);
    buf[1] = (unsigned char)((val>>8)  & 0xFF);
    buf[2] = (unsigned char)((val)     & 0xFF);
}

inline static void i_to_4uc(unsigned char* buf, int val)
{
    buf[0] = (unsigned char)((val>>24) & 0xFF);
    buf[1] = (unsigned char)((val>>16) & 0xFF);
    buf[2] = (unsigned char)((val>>8)  & 0xFF);
    buf[3] = (unsigned char)((val)     & 0xFF);
}

#endif /* _SAF7741_I2C_H */

