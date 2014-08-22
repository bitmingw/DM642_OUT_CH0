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
  L2 : o = 00000000h l = 00040000h /* all SRAM     		*/
  CE01: o = 80000000h l = 00100000h /* external memory   */
  CE02: o = 80100000h l = 00f000000h /* external memory   */
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
    .capChaAYSpace>    CE01
    .capChaACbSpace>   CE01
   /* .capChaACbSpace>   L2*/
    /* .capChaACrSpace>   L2*/
   .capChaACrSpace>   CE01
    .disChaAYSpace>    CE02
    .disChaACbSpace>   CE02
    .disChaACrSpace>   CE02
    .external   >      CE02
}                           
