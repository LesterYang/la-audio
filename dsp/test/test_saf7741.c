/*
 *  test_saf7741.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include <qDsp.h>

#include "../Hal/qTuner.h"


#define OPS_SEARCH (1)

struct option long_opts[] = {
    {"init",                0, 0,   'i'},
    {"volume",              1, 0,   'v'},
    {"mute",                1, 0,   'm'},
    {"source",              1, 0,   's'},
    {"search",              1, 0,   OPS_SEARCH},
    {"help",                0, 0,   'h'},
    {"read",                0, 0,   'r'},
    {"write",               1, 0,   'w'},
    {"register",            1, 0,   'R'},
    {"version",             0, 0,   'V'},
    {"freq",                1, 0,   'f'},
    {"band",                1, 0,   'b'},
    {"status",              0, 0,   'S'},
    {"delay",               1, 0,   'd'},
    {0,                     0, 0,   0}
};

void qdsp_test_usage()
{
    fprintf(stderr, "Usage: dsp_saf7741_test OPTION\n"
                    "OPTION\n"
                    "   -i  --init          start up\n"
                    "   -s  --source [SRC]  switch source\n"
                    "                           SRC 0 : USB\n"
                    "                               1 : NAVI\n"
                    "                               2 : RADIO\n"
                    "                               3 : DVD\n"
                    "                               4 : DTV\n"
                    "                               5 : AUXIN\n" 
                    "   -v  --volume [VOL]  set volume, VOL: 0~100\n"
                    "   -m  --mute   [FLAG] set mute, FLAG: 0(unmute), 1(mute)\n"
                    "   -f  --freq   [FREQ] set frequency\n"
                    "   -b  --band   [BAND] set band, BAND: 0(AM), 1(FM)\n"
                    "   -R  --register[REG] set register address\n"
                    "   -r  --read          read 3 bytes from register (-R to set register address)\n"
                    "   -w  --write  [DATA] write 3 bytes to register  (-R to set register address)\n"
                    "   -d  --delay  [VAL]  sleep VAL ms\n"
                    "   -S  --status        pull radio status\n"
                    "       --search        search frequency\n"    
                    "   -h  --help          show usage\n"
                    "   -V  --version       show version\n");
}


extern int saf7741_i2c_write(int addr, unsigned char* buf, size_t len);
extern int saf7741_i2c_read(int addr, unsigned char* buf, size_t len);

void qdsp_test_read_saf7741_regs(int reg)
{
    unsigned char  data[3]={0};

    if(saf7741_i2c_read(reg, data, sizeof(data))<0)
        printf("read error\n");
    else
        printf("read  addr 0x%06x : %02x %02x %02x\n", reg, data[0], data[1], data[2]);
}

void qdsp_test_write_saf7741_regs(int reg, int val)
{
    unsigned char data[3]={0};

    data[0] = (unsigned char)((val>>16)&0xFF);
    data[1] = (unsigned char)((val>>8)&0xFF);
    data[2] = (unsigned char)(val&0xFF);

    if(saf7741_i2c_write(reg, data, sizeof(data))<0)
        printf("write error\n");
    else
        printf("write addr 0x%06x : %02x %02x %02x\n", reg, data[0], data[1], data[2]);
}

int _xtoi(char c)
{
    if(c>='0' && c<='9')
        return c - '0';
    else if(c>='a' && c<='f')
        return c - 'a' + 10;
    else if(c>='A' && c<='F')
        return c - 'A' + 10;
 
    return -1;
}


int xtoi(const char* s)
{
    int v = _xtoi(*s++);
    int t;

    if(v < 0)
        return -1;

    while((t = _xtoi(*s++)) >= 0)
    {
        v *= 16;
        v += t;
    }
    
    return v;
}
    
extern int saf7741_StartUp();  
extern int saf7741_OpenDev();
extern int tef7000_pull_status();
extern int tef7000_search(qtuner_band band, short freq);

int main(int argc, const char *argv[])
{
    int opt_idx, c, val=0, rw=-1, reg=0, freq;
    char *short_opts = "iv:m:s:hvrw:R:f:b:Sd:";

    if(argc == 1)
    {
        qdsp_test_usage();
        return 0;
    }

    qdsp_open(QDSP_DEV_SAF7741);
    
    while ((c = getopt_long(argc, (char* const*)argv, short_opts, long_opts, &opt_idx)) != -1)
    {
         switch(c)
         {
            case 'i':
                qdsp_reset();
                break;
                
            case 'v':
                val = atoi(optarg);
                qdsp_audio_set_vol(val);
                break;
                
            case 'm':
                val = atoi(optarg);
                if(val)
                    qdsp_audio_mute();
                else
                    qdsp_audio_unmute();
                break;
                
            case 's':
                val = atoi(optarg);
                qdsp_audio_switch_source((qdsp_audio_channel)val);
                break;

            case 'r':
                rw = 0;
                break;
                
            case 'w':
                rw = 1;
                val = xtoi(optarg);
                break;
                
            case 'R':              
                reg = xtoi(optarg);
                break;
                
            case 'V':
                qdsp_version();
                break;

            case 'f':
                freq = atoi(optarg);
                qdsp_radio_set_freq(freq);
                break;

            case 'b':
                val = atoi(optarg);
                if(val==0)
                    qdsp_radio_set_band(QDSP_BAND_AM);
                else if(val==1)
                    qdsp_radio_set_band(QDSP_BAND_FM);
                break;

            case OPS_SEARCH:
                freq = atoi(optarg);
                printf("search FM %d.%d MHz, return %d\n",freq/100,freq%100, tef7000_search(QTUNER_BAND_FM, freq));
                break;

            case 'S':
                tef7000_pull_status();
                break;
                
            case 'd':
                val = atoi(optarg);
                printf("delay %d ms\n", val);
                usleep(val*1000);
                break;
                
            case 'h':  
            default:
                 qdsp_test_usage();
                 break;
         }
    }


    if(reg > 0)
    {
        if(rw == 0)
            qdsp_test_read_saf7741_regs(reg);
        else if(rw == 1 && val >= 0)
            qdsp_test_write_saf7741_regs(reg, val);
        else
            printf("neet to set -r or -w\n");
    }

    qdsp_close();
    return 0;
}
