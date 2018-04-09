/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                               C8Debug.h                                **/
/**                                                                        **/
/** This file contains a small debugger                                    **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include "CHIP8.h"

#ifdef DEBUG

#include <stdio.h>

/****************************************************************************/
/* This routine is called every opcode when chip8_trace==1. It prints the   */
/* current register contents and the opcode being executed                  */
/****************************************************************************/
void chip8_debug (word opcode,struct chip8_regs_struct *regs)
{
 int i;
 byte v1[4],v2[4];
 sprintf (v1,"V%x",(opcode>>8)&0x0f);
 sprintf (v2,"V%x",(opcode>>4)&0x0f);
 printf ("PC=%04x: %04x - ",regs->pc,opcode);
 switch (opcode>>12)
 {
  case 0:
   printf ("SYS  %03X - ",opcode&0xff);
   switch (opcode&0xfff)
   {
    case 0xe0:
     puts ("Clear screen");
     break;
    case 0xee:
     puts ("Return from subroutine");
     break;
    default:
     puts ("Unknown system call");
   }
   break;
  case 1:
   printf ("JMP  %03x",opcode&0xfff);
   break;
  case 2:
   printf ("CALL %03x",opcode&0xfff);
   break;
  case 3:
   printf ("SKEQ %s,%02x",v1,opcode&0xff);
   break;
  case 4:
   printf ("SKNE %s,%02x",v1,opcode&0xff);
   break;
  case 5:
   printf ("SKEQ %s,%s",v1,v2);
   break;
  case 6:
   printf ("LD   %s,%02x",v1,opcode&0xff);
   break;
  case 7:
   printf ("ADD  %s,%02x",v1,opcode&0xff);
   break;
  case 8:
   switch (opcode&0x0f)
   {
    case 0:
     printf ("LD   %s,%s",v1,v2);
     break;
    case 1:
     printf ("OR   %s,%s",v1,v2);
     break;
    case 2:
     printf ("AND  %s,%s",v1,v2);
     break;
    case 3:
     printf ("XOR  %s,%s",v1,v2);
     break;
    case 4:
     printf ("ADD  %s,%s",v1,v2);
     break;
    case 5:
     printf ("SUB  %s,%s",v1,v2);
     break;
    case 6:
     printf ("SHR  %s,%s",v1,v2);
     break;
    case 7:
     printf ("RSB  %s,%s",v1,v2);
     break;
    case 14:
     printf ("SHL  %s,%s",v1,v2);
     break;
    default:
     printf ("Illegal opcode");
   }
   break;
  case 9:
   printf ("SKNE %s,%s",v1,v2);
   break;
  case 10:
   printf ("MVI  %03x",opcode&0xfff);
   break;
  case 11:
   printf ("JMI  %03x",opcode&0xfff);
   break;
  case 12:
   printf ("RAND %s,%s",v1,v2);
   break;
  case 13:
   printf ("SPRITE %s,%s,%x",v1,v2,opcode&0x0f);
   break;
  case 14:
   switch (opcode&0xff)
   {
    case 0x9e:
     printf ("SKP  %s",v1);
     break;
    case 0xa1:
     printf ("SKNP %s",v1);
     break;
    default:
     printf ("Illegal opcode");
   }
   break;
  case 15:
   switch (opcode&0xff)
   {
    case 0x07:
     printf ("LD  %s,DT",v1);
     break;
    case 0x0a:
     printf ("LD  %s,KEY",v1);
     break;
    case 0x15:
     printf ("LD   DT,%s",v1);
     break;
    case 0x18:
     printf ("LD   ST,%s",v1);
     break;
    case 0x1e:
     printf ("ADD  I,%s",v1);
     break;
    case 0x29:
     printf ("LD   I,SPRITE(%s)",v1);
     break;
    case 0x33:
     printf ("BCD  %s",v1);
     break;
    case 0x55:
     printf ("LD   [I],%s",v1);
     break;
    case 0x65:
     printf ("LD   %s,[I]",v1);
     break;
    default:
     printf ("Illegal opcode");
   }
   break;
 }
 printf ("\n Registers: ");
 for (i=0;i<16;++i) printf ("%02x ",(regs->alg[i])&0xff);
 printf ("\n Index: %03x Stack:%03x Delay:%02x Sound:%02x\n",
         regs->i&0xfff,regs->sp&0xfff,regs->delay&0xff,regs->sound&0xff);
}
#endif
