/*
 *  saf7741.c
 *  Copyright © 2015  
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@yahoo.com.tw>
 *  Description : 
 */
 
#include <string.h>

#include <saf7741.h>

#include "saf7741_audio.h"
#include "saf7741_radio.h"

#include "saf7741_util.h"
#include "saf7741_audio_regs.h"
#include "saf7741_radio_regs.h"
#include "saf7741_i2c.h"
#include "saf7741_patch.h"

int saf7741_EnableTunerI2C()
{
    // Command to set IFAGC at maximun attenuation to allow correct IFADC start-up
    return (saf7741_radioResetTuner() < 0) ? -1 : 0;
}

int saf7741_ResetAllDSP()
{
    unsigned char VAL[3] = {0};
    int data = 0;
    union{
        char val;
        struct{
            unsigned char pc_reset   :1;
            unsigned char iic_dyn    :1;
            unsigned char en_extpram :1;
			unsigned char reserved   :5;
        };
    }adsp={0},sdsp={0},tdsp1e={0},tdsp2={0},tdsp1={0};


    adsp.pc_reset     = 1;
    adsp.iic_dyn      = 0;
    adsp.en_extpram   = 0;
    sdsp.pc_reset     = 1;
    sdsp.iic_dyn      = 0;
    sdsp.en_extpram   = 0;    
    tdsp1e.pc_reset   = 1;
    tdsp1e.iic_dyn    = 0;
    tdsp1e.en_extpram = 0;    
    tdsp2.pc_reset    = 1;
    tdsp2.iic_dyn     = 0;
    tdsp2.en_extpram  = 0;    
    tdsp1.pc_reset    = 1;
    tdsp1.iic_dyn     = 0;
    tdsp1.en_extpram  = 0;    
   
    data |= tdsp1.val;
    data |= tdsp2.val  << 3;
    data |= tdsp1e.val << 6;
    data |= sdsp.val   << 9;
    data |= adsp.val   << 12;

    i_to_3uc(VAL, data);

    saf7741_log(DbgStartUpRadio, "DSP_CTR : 0x%06x", data);   // 0x001249 

    return saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL));
}

int saf7741_SetClkPLL()
{   
    // Set DSP speed and input out frequency
    unsigned char VAL[3] = {0};
    int clk = 0;

    int dsp_clock_kHz   = 135200;
    int dsp_sel         = CLKPLL_CTR_divided_by_2;
    int spdif_sel       = CLKPLL_CTR_divided_by_4;
    int dsp_ppl_msel    =  (dsp_sel * dsp_clock_kHz / 650) - 212;
    int audio_ref_sel   = CLKPLL_CTR_AUDIO_PLL;
    int appl_in_sel     = CLKPLL_CTR_WS_HOST_48k;
    int fssys_iosel     = CLKPLL_CTR_input;

    clk |= dsp_sel; 
    clk |= spdif_sel        << 2; 
    clk |= dsp_ppl_msel     << 4; 
    clk |= audio_ref_sel    << 17; 
    clk |= appl_in_sel      << 19; 
    clk |= fssys_iosel      << 23;

    saf7741_log(DbgStartUpAudio, "CLKPPL : 0x%06x", clk);

    i_to_3uc(VAL, clk);
    return saf7741_i2c_write(CLKPLL_CTR, VAL, sizeof(VAL));
}


int saf7741_Patch()
{
    // Patch 0.4 TDSP1E TDSP2
    int idx;
    unsigned char PATCH[4] = {0};
    int tdsp1e_addr = PATCH_TDSP1E_ADDR;
    int tdsp2_addr  = PATCH_TDSP2_ADDR;

    i_to_4uc(PATCH, dwTdsp1eTable[0]);
    if(saf7741_i2c_write(tdsp1e_addr, PATCH, 4) < 0)
        return -1; 
    
    tdsp1e_addr += 0x008000;

    for(idx=1; idx<PATCH_TDSP1E_LENGTH; idx++)
    {
        i_to_4uc(PATCH, dwTdsp1eTable[idx]);
        if(saf7741_i2c_write(tdsp1e_addr, PATCH, 4) < 0)
            return -1;
        tdsp1e_addr++;
    }

    i_to_4uc(PATCH, dwTdsp2Table[0]);
    if(saf7741_i2c_write(tdsp2_addr, PATCH, 4) < 0)
        return -1; 

    tdsp2_addr += 0x008000;

    for(idx=1; idx<PATCH_TDSP2_LENGTH; idx++)
    {
        i_to_4uc(PATCH, dwTdsp1eTable[idx]);
        if(saf7741_i2c_write(tdsp2_addr, PATCH, 4) < 0)
            return -1;
        tdsp2_addr++;
    }

    return 0;
}

int saf7741_AdspIOCtrl()
{
    unsigned char VAL[3] = {0};
    int ctrl = 0;

    int iis1_fmt        = ADSP_IO_CTR_FMT_PHILIPS;
    int iis2_fmt        = ADSP_IO_CTR_FMT_PHILIPS;
    int iis3_fmt        = ADSP_IO_CTR_FMT_PHILIPS;
    int iis4_fmt        = ADSP_IO_CTR_FMT_PHILIPS;
    int host_io_fmt     = ADSP_IO_CTR_FMT_PHILIPS;
    int host_io_mode    = ADSP_IO_CTR_HOST_IO_SLAVE;
    int host_io_ena     = ADSP_IO_CTR_HOST_IO_DISABLE;
    int host_io_fmt_div = ADSP_IO_CTR_FMT_PHILIPS;      // it's' not supported on SAF7741HV bound out
    int host_io_ena_div = 0;                            // it's' not supported on SAF7741HV bound out
    int dvd_ctr         = ADSP_IO_CTR_SPDIF_MODE;
    int spdif_ctr       = ADSP_IO_CTR_SPDIF1D_DVD34;

    ctrl |= iis1_fmt;
    ctrl |= iis2_fmt        << 3;
    ctrl |= iis3_fmt        << 6;
    ctrl |= iis4_fmt        << 9;
    ctrl |= host_io_fmt     << 12;
    ctrl |= host_io_mode    << 15;
    ctrl |= host_io_ena     << 16;
    ctrl |= host_io_fmt_div << 17;
    ctrl |= host_io_ena_div << 20;
    ctrl |= dvd_ctr         << 21;
    ctrl |= spdif_ctr       << 22;

    i_to_3uc(VAL, ctrl);
    saf7741_log(DbgStartUpAudio, "ADSP_IO_CTR : 0x%06x", ctrl);   // 0x0636DB 

    return saf7741_i2c_write(ADSP_IO_CTR, VAL, sizeof(VAL));
}

int saf7741_StartUp()
{
    unsigned char VAL_WORD[2] = {0};
    unsigned char VAL[3] = {0};
 
    if(saf7741_EnableTunerI2C() < 0) return -1;
    if(saf7741_ResetAllDSP() < 0)    return -1;
    if(saf7741_SetClkPLL() < 0)      return -1;
    if(saf7741_Patch() < 0)          return -1;  
    
// 5. Disable RGPDAC1&2 and IFAD1
    i_to_2uc(VAL_WORD, 0x3B20);
    if(saf7741_i2c_write(APD_CTRL1, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;

// 6. Disable PDC1 clock
    i_to_2uc(VAL_WORD, 0x07FE);
    if(saf7741_i2c_write(CLK_EN, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;

// 7. PDC2: Default changed
    i_to_3uc(VAL, 0x000CC6);
    if(saf7741_i2c_write(PDC2_AGC_CTRL_4, VAL, sizeof(VAL)) < 0)
        return -1;

// 8. PDC2: AGC gain default
    i_to_3uc(VAL, 0xEA7DBF);
    if(saf7741_i2c_write(PDC2_AGC_GAIN_1_2, VAL, sizeof(VAL)) < 0)
        return -1;
    
// 9. Write AGC gain default, required to active the AGC_GAIN_1_2 setting
    i_to_3uc(VAL, 0x0FF000);
    if(saf7741_i2c_write(PDC2_AGC_GAIN_0_0, VAL, sizeof(VAL)) < 0)
        return -1;
    
// 10. Release TDSP 1
    i_to_3uc(VAL, 0x001248);
    if(saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL)) < 0)
        return -1;

// 11. Kick off radio program
    i_to_3uc(VAL, 0x000001);
    if(saf7741_i2c_write(TDSP1_X_FW_KeyCod, VAL, sizeof(VAL)) < 0)
        return -1;

// 12. Release  TDSP1E
    i_to_3uc(VAL, 0x001208);
    if(saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL)) < 0)
        return -1;

#if 0 // sample code
// Release DSP2
    i_to_3uc(VAL, 0x001200);
    if(saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL)) < 0)
        return -1;
#endif
    
// 13. Release TDSP 2, SDSP and ADSP
    i_to_3uc(VAL, 0x000000);
    if(saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL)) < 0)
        return -1;

// 14. Enable dynamic PARAM (to use patches)
    i_to_3uc(VAL, 0x000090);
    if(saf7741_i2c_write(DSP_CTR, VAL, sizeof(VAL)) < 0)
        return -1;

// 15. Set hook for patch 0.4 TDSP1E
    i_to_3uc(VAL, 0x008000);
    if(saf7741_i2c_write(pFW_Main_1E_Hook_Begin, VAL, sizeof(VAL)) < 0)
        return -1;

// 16. Set hook for patch 0.4 TDSP2
    i_to_3uc(VAL, 0x008000);
    if(saf7741_i2c_write(pFW_Main_2_Hook_Begin, VAL, sizeof(VAL)) < 0)
        return -1;

// 17. Kick off ADSP
    i_to_3uc(VAL, 0x000001);
    if(saf7741_i2c_write(ADSP_X_KeyCode, VAL, sizeof(VAL)) < 0)
        return -1;

// 18. Sample rate converter 1.  Enable : 0x800000, Disable : 0x000000
    i_to_3uc(VAL, 0x800000);
    if(saf7741_i2c_write(SDSP_X_CtrlTS1, VAL, sizeof(VAL)) < 0)
        return -1;
    
#if 1 // sample code
    if(saf7741_i2c_write(SDSP_X_CtrlTS2, VAL, sizeof(VAL)) < 0)
        return -1; 
    if(saf7741_i2c_write(SDSP_X_CtrlTS3, VAL, sizeof(VAL)) < 0)
        return -1;
    if(saf7741_i2c_write(SDSP_X_CtrlTS4, VAL, sizeof(VAL)) < 0)
        return -1;
    if(saf7741_i2c_write(SDSP_X_CtrlTS5, VAL, sizeof(VAL)) < 0)
        return -1;
#endif

// 19. Correct AGC_d thresholds & timing, to be set after release of TDPs
    i_to_3uc(VAL, 0x704317);
    if(saf7741_i2c_write(PDC2_AGC_CTRL_0, VAL, sizeof(VAL)) < 0)
        return -1;

// 20. Enable sequential DAC delay
    i_to_2uc(VAL_WORD, 0x0088);
    if(saf7741_i2c_write(DAC_DELAY, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;

// 21~24. Recommended AM level detector coefficients: enables reasonable AM seek time(~50ms)
    i_to_2uc(VAL_WORD, 0x07F5);
    if(saf7741_i2c_write(TDSP1E_Y_IntVal5, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_2uc(VAL_WORD, 0x07CF);
    if(saf7741_i2c_write(TDSP1E_Y_IntVal6, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
    i_to_3uc(VAL, 0x0621A5);
    if(saf7741_i2c_write(TDSP1E_X_IntVal7, VAL, sizeof(VAL)) < 0)
        return -1;
    i_to_3uc(VAL, 0x1F47D0);
    if(saf7741_i2c_write(TDSP1E_X_IntVal8, VAL, sizeof(VAL)) < 0)
        return -1;

// 25. Tuner delay for correct AFU readout
    i_to_3uc(VAL, 0x007080);
    if(saf7741_i2c_write(TDSP1_X_pFW_TunDly, VAL, sizeof(VAL)) < 0)
        return -1;

// 26. Corrected AM noiseblanker value(patch)
    i_to_3uc(VAL, 0x028080);
    if(saf7741_i2c_write(ACIS_1_pIFIntDly, VAL, sizeof(VAL)) < 0)
        return -1;
#if 0
// 27. Main FM to primary channel
    i_to_3uc(VAL, ADSP_EASYP_SrcSw_Tuner1onA);
    if(saf7741_i2c_write(ADSP_X_EasyP_Index, VAL, sizeof(VAL)) < 0)
        return -1;
#endif

// 28. Set primary channel main 1 to 0 dB
    i_to_2uc(VAL_WORD, 0x0800);
    if(saf7741_i2c_write(ADSP_Y_Vol_Main1P, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;

// 29. Set primary channel main 2 to 0 dB
    i_to_2uc(VAL_WORD, 0x0080);
    if(saf7741_i2c_write(ADSP_Y_Vol_Main2P, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;
	
// QiMing setting
#if 1
    // AudioControl_7741.c : line 244
    i_to_2uc(VAL_WORD, 0x0080);
    if(saf7741_i2c_write(TDSP1E_Y_AIPL_1_PilThr, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_FW_AMFrcMonExt, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0146);
    if(saf7741_i2c_write(TDSP1E_Y_FIPL_1_PilThr, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_3uc(VAL, 0x0CCCCD);
    if(saf7741_i2c_write(TDSP1E_X_wFCSB_1_UsrCtrl_in, VAL, sizeof(VAL)) < 0)        return -1;

    // AudioControl_7741.c : line 250
    i_to_2uc(VAL_WORD, 0x07FF);         //linear relation
    if(saf7741_i2c_write(TDSP1E_Y_ACVR_1_LinLogFract, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0194);         // range select slope
    if(saf7741_i2c_write(TDSP1E_Y_ACVR_1_LvlRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x000B);         // range select offset
    if(saf7741_i2c_write(TDSP1E_Y_ACVR_1_LvlRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0233);         // maximum attenuation limited 
    if(saf7741_i2c_write(TDSP1E_Y_ACVR_1_LvlRanMin, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0000);         // range select minimum
    if(saf7741_i2c_write(TDSP1E_Y_ACVR_1_ModRanMin, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;


    // set b0,b1,a1 coefficients for 1st order low pass IIR, refer to Radio User Manual p.70
    i_to_2uc(VAL_WORD, 0x07FF);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Mabb0, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Mabb1, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0146);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_Maba1, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;   

     // AudioControl_7741.c : line 260
    i_to_2uc(VAL_WORD, 0x0DFD);
    if(saf7741_i2c_write(TDSP1E_Y_FCAB_1_LvlRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x00B4);
    if(saf7741_i2c_write(TDSP1E_Y_FCAB_1_LvlRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // FM High Cut - Audio Bandwidth Reduction Stage
    // -3dB bandwidth cut-off frequency at 4kHz, refer to Radio User Manual p.59
    i_to_2uc(VAL_WORD, 0x0464);
    if(saf7741_i2c_write(TDSP1E_Y_FDFR_1_LimFac, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1; 

    // FM High Cut on Multipath
    i_to_2uc(VAL_WORD, 0x011C);
    if(saf7741_i2c_write(TDSP1E_Y_FCAB_1_MltRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0FB9);
    if(saf7741_i2c_write(TDSP1E_Y_FCAB_1_MltRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;    

    // FM Stero Blend on Level, refer to Radio User Manual p.47
    i_to_2uc(VAL_WORD, 0x0D29);
    if(saf7741_i2c_write(TDSP1E_Y_FCSB_1_LvlRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0177);
    if(saf7741_i2c_write(TDSP1E_Y_FCSB_1_LvlRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;    

    // FM Softmute on Level
    i_to_2uc(VAL_WORD, 0x0BD9);
    if(saf7741_i2c_write(TDSP1E_Y_FCVR_1_LvlRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x005F);
    if(saf7741_i2c_write(TDSP1E_Y_FCVR_1_LvlRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;    
    i_to_2uc(VAL_WORD, 0x0076);
    if(saf7741_i2c_write(TDSP1E_Y_FCVR_1_LvlGainSlow, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x07FF);
    if(saf7741_i2c_write(TDSP1E_Y_FCVR_1_LvlGainDelta, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;   

    // FM Theshold Extension, refer to Radio User Manual p.34 ~ p.38
    i_to_2uc(VAL_WORD, 0x01EC);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanSlpTE, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0FF1);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanOfsTE, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;   
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMinTE, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0400);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanMaxTE, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;   
    i_to_2uc(VAL_WORD, 0x01AE);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanSlpBst, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL_WORD, 0x0FE0);
    if(saf7741_i2c_write(TDSP1_Y_FCIB_1_RanOfsBst, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;   


    // Fixed bandwidth setting, refer to Radio User Manual p.95
    i_to_2uc(VAL_WORD, 0x07FF);         // 0x0000 : Automatic, 0x07FF : Munual bandwidth setting
    if(saf7741_i2c_write(TDSP1_Y_ACIB_1_BWSetMan, VAL_WORD, sizeof(VAL_WORD)) < 0)   return -1;
    i_to_2uc(VAL, 0x00000A);            // default : 0x000005
    if(saf7741_i2c_write(TDSP1_X_ACIB_1_FirCtlFix, VAL, sizeof(VAL)) < 0)  return -1; 

    // AM Audio High-Pass Filter : Flat, refer to Radio User Manual p.106
    i_to_2uc(VAL_WORD, 0x07FF);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfg1, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfb11, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfa11, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x07FF);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfg2, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfb12, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0000);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Alfa12, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // AM Audio Hight-Cut Filter : 3.5kHz, refer to Radio User Manual p.107
    i_to_2uc(VAL_WORD, 0x005B);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Mabb2, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0DAD);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Maba2, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x005B);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Mabb0, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0008);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Mabb1, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0590);
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_Maba1, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // AM High Cut, refer to Radio User Manual p.99
    i_to_2uc(VAL_WORD, 0x02F9);         //default 0x0575
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_LimOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x02EA);         //default 0x0130
    if(saf7741_i2c_write(TDSP1E_Y_ADFR_1_LimFac, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // AM High Cut on Level, refer to Radio User Manual p.101
    i_to_2uc(VAL_WORD, 0x0280);
    if(saf7741_i2c_write(TDSP1E_Y_ACAB_1_LvlRanSlp, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;
    i_to_2uc(VAL_WORD, 0x0F00);
    if(saf7741_i2c_write(TDSP1E_Y_ACAB_1_LvlRanOfs, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // AM Demodulator, refer to Radio User Manual p.98
    i_to_2uc(VAL_WORD, 0x02CD);         // 0x151 : demodulator synchroneous mode, 0x2CD : envelope detector mode
    if(saf7741_i2c_write(TDSP1E_Y_AMSD_1_SpdThr, VAL_WORD, sizeof(VAL_WORD)) < 0)  return -1;

    // Set AD input, refer to I2C Map p.22
    i_to_3uc(VAL, 0x004480);            // set input : AIN0_L and AIN0_R  [default:0x0064C0]
    if(saf7741_i2c_write(AD_IN_SEL1, VAL, sizeof(VAL)) < 0) return -1;
    i_to_3uc(VAL, 0x002DD2);            // set input : AIN2_L and AIN2_R  [default:0x006DD2]
    if(saf7741_i2c_write(AD_IN_SEL2, VAL, sizeof(VAL)) < 0) return -1;
    i_to_3uc(VAL, 0x00526D);            // set input : AIN4                         [default:0x00522D]
    if(saf7741_i2c_write(AD_IN_SEL3, VAL, sizeof(VAL)) < 0) return -1;


    // Set Hardware input mode, refer to Audio User Manual p.17
    // Note that this only has to be done once, after starting the DPS program
    i_to_3uc(VAL, 0x000080);            // set AIN0 : high CMRR one ground line mode
    if(saf7741_i2c_write(ADSP_X_SrcSw_AIN0ADInSelOrMask, VAL, sizeof(VAL)) < 0) return -1;
    i_to_3uc(VAL, 0x00006D);            // set AIN4 : differential mode
    if(saf7741_i2c_write(ADSP_X_SrcSw_AIN4ADInSelOrMask, VAL, sizeof(VAL)) < 0) return -1;

#endif

    if(saf7741_radioStartUpTuner() < 0) return -1;


// 36. Release primary channel mute
    i_to_2uc(VAL_WORD, 0x0800);
    if(saf7741_i2c_write(ADSP_Y_Mute_P, VAL_WORD, sizeof(VAL_WORD)) < 0)
        return -1;


// LST setting

    if(saf7741_AdspIOCtrl() < 0) return -1;


    
    // X. Set register AD_IN_SEL1 ... 
    //    
    
    
    // X. At start up, the Chime/PDC sound generator is not enabled. 
    //     Note that this only has to be done once at start up



    saf7741_audioSetChannelMode(SAF7741_CH_MODE_STEREO);
    saf7741_audioSetSourceOnPrimary(SAF7741_A_IIS3);
#if 1
    saf7741_audioSetSecondVol(0);
    saf7741_audioSetPhoneVol(0);
    saf7741_audioSetNaviVol(0);
 
    saf7741_audioSetSecondMute(true);
    saf7741_audioSetPhoneMute(true);
    saf7741_audioSetNaviMute(true);
#endif
    return 0;
}



int saf7741_SwitchTunerInFMLP()
{
#define UseTuner1 0x0000
#define UseTuner2 0x0001

    unsigned char VAL[2] = {0};

    i_to_2uc(VAL, UseTuner2);

    saf7741_i2c_write(TDSP1_Y_FW_1_FMLP_TunSwt, VAL, sizeof(VAL));

    // have to be written to one of the following hardware registers:
    //    PDC1_FMIX_0 and PDC1_FMIX_1 for Tuner 1
    //    PDC2_FMIX_0 and PDC2_FMIX_1 for Tuner 2
    return 0;
}

int saf7741_OpenDev()
{
    if(saf7741_i2c_open() < 0)
    {
        saf7741_log(DbgErr, "saf7741 open i2c error");
        return -1;
    }
    
    if(saf7741_radioOpenTuner() < 0 )
    {
        saf7741_log(DbgErr, "tuner open control interface error");
        return -1;
    }

    return 0;
}

void saf7741_CloseDev()
{
    saf7741_i2c_close();
}

static qdsp_device_ops ops;

const qdsp_device_ops* saf7741_deviceGetOps(void)
{
    ops.open       = saf7741_OpenDev;
    ops.close      = saf7741_CloseDev;
    ops.startup    = saf7741_StartUp;

    return &ops;
}

