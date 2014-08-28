/*-----------------------------------------------------
 * Copyright (C) 2006 Vision Magic Ltd.
 * All Rights Reserved
 *-----------------------------------------------------*/
/*
 *---------VMD642_video_out.cmd---------
 *
 */
MEMORY
{
  L2  : o = 00000000h l = 00040000h /* internal SRAM, 256KB */
  VIN : o = 80000000h l = 00100000h /* external memory, 1MB  */
  BUF1: o = 80100000h l = 00100000h /* external memory, 1MB  */
  BUF2: o = 80200000h l = 00100000h /* external memory, 1MB  */
  BUF3: o = 80300000h l = 00100000h /* external memory, 1MB  */
  DF12: o = 80400000h l = 00100000h /* external memory, 1MB  */
  DF23: o = 80500000h l = 00100000h /* external memroy, 1MB  */
  VOUT: o = 81000000h l = 00f00000h /* external memory, 15MB */
}

SECTIONS
{
    .cinit      >       L2
    .text       >       L2
    .stack      >       L2
    .bss        >       L2
    .const      >       L2
    .data       >       L2
    .far        >       L2
    .switch     >       L2
    .sysmem     >       L2
    .tables     >       L2
    .cio        >       L2

    .capChaAYSpace >    VIN
    .capChaACbSpace >   VIN
    .capChaACrSpace >   VIN

    .ChaAYSpace1 >      BUF1
    .ChaACbSpace1 >     BUF1
    .ChaACrSpace1 >     BUF1
    .ChaAYSpace2 >      BUF2
    .ChaACbSpace2 >     BUF2
    .ChaACrSpace2 >     BUF2
    .ChaAYSpace3 >      BUF3
    .ChaACbSpace3 >     BUF3
    .ChaACrSpace3 >     BUF3

    .ChaAYSpaceDiff12 > DF12
    .ChaACbSpaceDiff12> DF12
    .ChaACrSpaceDiff12> DF12
    .ChaAYSpaceDiff23 > DF23
    .ChaACbSpaceDiff23> DF23
    .ChaACrSpaceDiff23> DF23    

    .disChaAYSpace >    VOUT
    .disChaACbSpace >   VOUT
    .disChaACrSpace >   VOUT
    
    .external >         VOUT
}                           
