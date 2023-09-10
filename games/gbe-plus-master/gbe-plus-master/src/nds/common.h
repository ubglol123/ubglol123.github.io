// GB Enhanced+ Copyright Daniel Baxter 2015
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : common.h
// Date : September 14, 2015
// Description : Common functions and definitions
//
// A bunch of consts
// Consts are quick references to commonly used memory locations 

#include "common/common.h"

#ifndef NDS_COMMON
#define NDS_COMMON

//Display A registers
const u32 NDS_DISPCNT_A = 0x4000000;
const u32 NDS_DISPSTAT = 0x4000004;
const u32 NDS_VCOUNT = 0x4000006;
const u32 NDS_BG0CNT_A = 0x4000008;
const u32 NDS_BG1CNT_A = 0x400000A;
const u32 NDS_BG2CNT_A = 0x400000C;
const u32 NDS_BG3CNT_A = 0x400000E;

const u32 NDS_BG0HOFS_A = 0x4000010;
const u32 NDS_BG0VOFS_A = 0x4000012;
const u32 NDS_BG1HOFS_A = 0x4000014;
const u32 NDS_BG1VOFS_A = 0x4000016;
const u32 NDS_BG2HOFS_A = 0x4000018;
const u32 NDS_BG2VOFS_A = 0x400001A;
const u32 NDS_BG3HOFS_A = 0x400001C;
const u32 NDS_BG3VOFS_A = 0x400001E;

const u32 NDS_BG2PA_A = 0x4000020;
const u32 NDS_BG2PB_A = 0x4000022;
const u32 NDS_BG2PC_A = 0x4000024;
const u32 NDS_BG2PD_A = 0x4000026;

const u32 NDS_BG2X_A = 0x4000028;
const u32 NDS_BG2Y_A = 0x400002C;

const u32 NDS_BG3PA_A = 0x4000030;
const u32 NDS_BG3PB_A = 0x4000032;
const u32 NDS_BG3PC_A = 0x4000034;
const u32 NDS_BG3PD_A = 0x4000036;

const u32 NDS_BG3X_A = 0x4000038;
const u32 NDS_BG3Y_A = 0x400003C;

const u32 NDS_WIN0H_A = 0x4000040;
const u32 NDS_WIN1H_A = 0x4000042;

const u32 NDS_WIN0V_A = 0x4000044;
const u32 NDS_WIN1V_A = 0x4000046;

const u32 NDS_WININ_A = 0x4000048;
const u32 NDS_WINOUT_A = 0x400004A;

const u32 NDS_BLDCNT_A = 0x4000050;
const u32 NDS_BLDALPHA_A = 0x4000052;
const u32 NDS_BLDY_A = 0x4000054;

const u32 NDS_CAPCNT = 0x4000064;

const u32 NDS_MASTER_BRIGHT_A = 0x400006C;

//Timers
const u32 NDS_TM0CNT_L = 0x4000100;
const u32 NDS_TM1CNT_L = 0x4000104;
const u32 NDS_TM2CNT_L = 0x4000108;
const u32 NDS_TM3CNT_L = 0x400010C;

const u32 NDS_TM0CNT_H = 0x4000102;
const u32 NDS_TM1CNT_H = 0x4000106;
const u32 NDS_TM2CNT_H = 0x400010A;
const u32 NDS_TM3CNT_H = 0x400010E;

//Display B registers
const u32 NDS_DISPCNT_B = 0x4001000;
const u32 NDS_BG0CNT_B = 0x4001008;
const u32 NDS_BG1CNT_B = 0x400100A;
const u32 NDS_BG2CNT_B = 0x400100C;
const u32 NDS_BG3CNT_B = 0x400100E;

const u32 NDS_BG0HOFS_B = 0x4001010;
const u32 NDS_BG0VOFS_B = 0x4001012;
const u32 NDS_BG1HOFS_B = 0x4001014;
const u32 NDS_BG1VOFS_B = 0x4001016;
const u32 NDS_BG2HOFS_B = 0x4001018;
const u32 NDS_BG2VOFS_B = 0x400101A;
const u32 NDS_BG3HOFS_B = 0x400101C;
const u32 NDS_BG3VOFS_B = 0x400101E;

const u32 NDS_BG2PA_B = 0x4001020;
const u32 NDS_BG2PB_B = 0x4001022;
const u32 NDS_BG2PC_B = 0x4001024;
const u32 NDS_BG2PD_B = 0x4001026;

const u32 NDS_BG2X_B = 0x4001028;
const u32 NDS_BG2Y_B = 0x400102C;

const u32 NDS_BG3PA_B = 0x4001030;
const u32 NDS_BG3PB_B = 0x4001032;
const u32 NDS_BG3PC_B = 0x4001034;
const u32 NDS_BG3PD_B = 0x4001036;

const u32 NDS_BG3X_B = 0x4001038;
const u32 NDS_BG3Y_B = 0x400103C;

const u32 NDS_WIN0H_B = 0x4001040;
const u32 NDS_WIN1H_B = 0x4001042;

const u32 NDS_WIN0V_B = 0x4001044;
const u32 NDS_WIN1V_B = 0x4001046;

const u32 NDS_WININ_B = 0x4001048;
const u32 NDS_WINOUT_B = 0x400104A;

const u32 NDS_BLDCNT_B = 0x4001050;
const u32 NDS_BLDALPHA_B = 0x4001052;
const u32 NDS_BLDY_B = 0x4001054;

const u32 NDS_MASTER_BRIGHT_B = 0x400106C;

//Misc Display registers
const u32 NDS_VRAMCNT_A = 0x4000240;
const u32 NDS_VRAMCNT_B = 0x4000241;
const u32 NDS_VRAMCNT_C = 0x4000242;
const u32 NDS_VRAMCNT_D = 0x4000243;
const u32 NDS_VRAMCNT_E = 0x4000244;
const u32 NDS_VRAMCNT_F = 0x4000245;
const u32 NDS_VRAMCNT_G = 0x4000246;
const u32 NDS_VRAMCNT_H = 0x4000248;
const u32 NDS_VRAMCNT_I = 0x4000249;

//WRAM
const u32 NDS_WRAMCNT = 0x4000247;

//Interrupt registers
const u32 NDS_IME = 0x4000208;
const u32 NDS_IE = 0x4000210;
const u32 NDS_IF = 0x4000214;

//DMA registers
const u32 NDS_DMA0SAD = 0x40000B0;
const u32 NDS_DMA0DAD = 0x40000B4;
const u32 NDS_DMA0CNT = 0x40000B8;

const u32 NDS_DMA1SAD = 0x40000BC;
const u32 NDS_DMA1DAD = 0x40000C0;
const u32 NDS_DMA1CNT = 0x40000C4;

const u32 NDS_DMA2SAD = 0x40000C8;
const u32 NDS_DMA2DAD = 0x40000CC;
const u32 NDS_DMA2CNT = 0x40000D0;

const u32 NDS_DMA3SAD = 0x40000D4;
const u32 NDS_DMA3DAD = 0x40000D8;
const u32 NDS_DMA3CNT = 0x40000DC;

const u32 NDS_DMA0FILL = 0x40000E0;
const u32 NDS_DMA1FILL = 0x40000E4;
const u32 NDS_DMA2FILL = 0x40000E8;
const u32 NDS_DMA3FILL = 0x40000EC;

//Input
const u32 NDS_KEYINPUT = 0x4000130;
const u32 NDS_KEYCNT = 0x4000132;
const u32 NDS_EXTKEYIN = 0x4000136;

//IPC
const u32 NDS_IPCSYNC = 0x4000180;
const u32 NDS_IPCFIFOCNT = 0x4000184;
const u32 NDS_IPCFIFOSND = 0x4000188;
const u32 NDS_IPCFIFORECV = 0x4100000;

//Math
const u32 NDS_DIVCNT = 0x4000280;
const u32 NDS_DIVNUMER = 0x4000290;
const u32 NDS_DIVDENOM = 0x4000298;
const u32 NDS_DIVRESULT = 0x40002A0;
const u32 NDS_DIVREMAIN = 0x40002A8;

const u32 NDS_SQRTCNT = 0x40002B0;
const u32 NDS_SQRTRESULT = 0x40002B4;
const u32 NDS_SQRTPARAM = 0x40002B8;

//3D
const u32 NDS_DISP3DCNT = 0x4000060;
const u32 NDS_GXFIFO = 0x4000400;
const u32 NDS_GXCMD = 0x4000440;
const u32 NDS_GXSTAT = 0x4000600;
const u32 NDS_GXRAM_COUNT = 0x4000604;

//SPI Bus
const u32 NDS_AUXSPICNT = 0x40001A0;
const u32 NDS_AUXSPIDATA = 0x40001A2;
const u32 NDS_ROMCNT = 0x40001A4;
const u32 NDS_CARDCMD_LO = 0x40001A8;
const u32 NDS_CARDCMD_HI = 0x40001AC;
const u32 NDS_CARD_DATA = 0x4100010;
const u32 NDS_SEED_0_LO = 0x40001B0;
const u32 NDS_SEED_0_HI = 0x40001B8;
const u32 NDS_SEED_1_LO = 0x40001B4;
const u32 NDS_SEED_1_HI = 0x40001BA;

const u32 NDS_SPICNT = 0x40001C0;
const u32 NDS_SPIDATA = 0x40001C2;

//Sound
const u32 NDS_SOUNDBIAS = 0x4000088;
const u32 NDS_SOUNDXCNT = 0x4000400;
const u32 NDS_SOUNDXSAD = 0x4000404;
const u32 NDS_SOUNDXTMR = 0x4000408;
const u32 NDS_SOUNDXPNT = 0x400040A;
const u32 NDS_SOUNDXLEN = 0x400040C;
const u32 NDS_SOUNDCNT = 0x4000500;

//Misc
const u32 NDS_EXMEM = 0x4000204;
const u32 NDS_POWERCNT = 0x4000304;
const u32 NDS_POSTFLG = 0x4000300;
const u32 NDS_HALTCNT = 0x4000301;
const u32 NDS_RCNT = 0x4000134;
const u32 NDS_RTC = 0x4000138;

#endif // NDS_COMMON 