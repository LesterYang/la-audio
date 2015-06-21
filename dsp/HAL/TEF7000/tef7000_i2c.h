#ifndef _TEF7000_I2C_H
#define _TEF7000_I2C_H

#define TEF7000_I2C_DEV             "/dev/i2c-1"

#define TEF7000_ADDR_GROUND         (0xC0)
#define TEF7000_ADDR_FLOAT          (0xC2)
#define TEF7000_ADDR_CONN_VCC       (0xC4)

#define TEF7000_I2C_7BIT_DEV_ADDR   (TEF7000_ADDR_GROUND >> 1)

#define E2P_MAIN_I2C_7BIT_DEV_ADDR  (0xA0 >> 1)
#define E2P_BACK_I2C_7BIT_DEV_ADDR  (0xA2 >> 1)

#define TEF7000_I2C_MAX_W_DATA_LEN  (8)
#define TEF7000_I2C_MAX_R_DATA_LEN  (4)


int tef7000_i2c_write(unsigned char msa, unsigned char* buf, size_t len);
int  tef7000_i2c_read(unsigned char* buf, size_t len);
int  tef7000_i2c_open(void);
void tef7000_i2c_close(void);


#endif /* _TEF7000_I2C_H */

