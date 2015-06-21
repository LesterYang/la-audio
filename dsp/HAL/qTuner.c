/*
 *  qTuner.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
#include <stdlib.h>

#include "TEF7000/tef7000.h"

const qtuner_ops* qturner_getOps(qtuner_device tuner)
{
    switch(tuner)
    {
        case QTUNER_DEV_TEF7000:    return tef7000_getOps();          
        default:                    break;
    }

    return NULL;
}

