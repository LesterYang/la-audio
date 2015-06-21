/*
 *  saf7741_util.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include "saf7741_util.h"

int gDbgFlag = DbgLvErr | DbgStartUp | DbgAudio | DbgRadio; 

void saf7741_utilSetDbgLv(int lv)
{
    gDbgFlag = lv;
}


int saf7741_getHexFromFrac(long long frac, int bits, int div, int mask, int shift)
{
    int val = 0;

    // refer to Radio User Manaual p.3, p.4
    // val = [ frac * 2^(bits -1) ] / div
    
    switch(div)
    {
        case 0:                                                     
            break;
            
        case 1000:      
            val = (int)((frac << (bits - 4)) / 125);    
            break;
            
        case 1000000:   
            val = (int)((frac << (bits - 7)) / 15625);  
            break;
            
        case 1000000000:   
            val = (int)((frac << (bits - 10)) / 1953125);  
            break;
            
        default:
            {
                int count = 1;
                while(div&0x1 && bits>count)
                {
                    count++;
                    div >>= 1;
                }
                val = (int)((frac << (bits - count)) / div);
            }
            break;
    }

    return (val & mask) >> shift;
}

