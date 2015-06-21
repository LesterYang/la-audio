#ifndef _TEF7000_REGS_H
#define _TEF7000_REGS_H

#define TEF7000_REG_TUNER   (0x00)
#define TEF7000_REG_CFALIGN (0x03)
#define TEF7000_REG_RFAGC   (0x04)
#define TEF7000_REG_IFAGC   (0x05)
#define TEF7000_REG_TEST    (0x06)


// MSA mode 
#define MSA_MODE_BUFFER     (0)
#define MSA_MODE_PRESET     (1)
#define MSA_MODE_SEARCH     (2)
#define MSA_MODE_AF_UPDATE  (3)
#define MSA_MODE_AF_JUMP    (4)
#define MSA_MODE_AF_CHECK   (5)
#define MSA_MODE_LOAD       (6)
#define MSA_MODE_END        (7)

// MSA control register
#define MSA_REGC_BUFFER     (0)
#define MSA_REGC_CONTROL    (1)

// Band type
#define BAND_TYPE_AM_LW         (0)
#define BAND_TYPE_AM_MW_EU      (1)
#define BAND_TYPE_AM_MW_USA     (2)
#define BAND_TYPE_AM_SW         (3)
#define BAND_TYPE_FM            (4)
#define BAND_TYPE_FM_OIRT       (5)
#define BAND_TYPE_WB            (6)
#define BAND_TYPE_ORBCOMM       (7)


#endif /* _TEF7000_REGS_H */

