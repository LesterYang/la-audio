#ifndef _QTUNER_H
#define _QTUNER_H

typedef struct _qtuner_ops qtuner_ops;

typedef enum
{
    QTUNER_DEV_TEF7000,

    QTUNER_DEV_UNKNOWN = -1
}qtuner_device;

typedef enum
{
    QTUNER_BAND_AM_LW,
    QTUNER_BAND_AM_MW_EU,
    QTUNER_BAND_AM_MW_USA,
    QTUNER_BAND_AM_SW,
    QTUNER_BAND_FM,
    QTUNER_BAND_FM_OIRT,
    QTUNER_BAND_WB,
    QTUNER_BAND_ORBCOMM
}qtuner_band;

typedef enum
{
    // TEF7000
    QTUNER_MODE_BUFFER,
    QTUNER_MODE_PRESET,
    QTUNER_MODE_SEARCH,
    QTUNER_MODE_AF_UPDATE,
    QTUNER_MODE_AF_JUMP,
    QTUNER_MODE_AF_CHECK,
    QTUNER_MODE_LOAD,
    QTUNER_MODE_END,

    // Other
    
    QTUNER_MODE_NULL = -1
}qtuner_mode;


/*--------------------------------
Function : a set of tuner options 
Return    : int
                 succeed :  0 
                 error      :  < 0 
---------------------------------*/
struct _qtuner_ops{
    int  (*init)(void);
    int  (*open)(void);
    void (*close)(void);
    int  (*reset)(void);

    int  (*search)(qtuner_band band, short freq);
    int  (*set_freq)(qtuner_mode mode, qtuner_band band, short freq);
    int  (*set_ifcf)(qtuner_mode mode, char ifcf);
    void (*clear)(void);
};

const qtuner_ops* qturner_getOps(qtuner_device tuner);

#endif /* _QTUNER_H */

