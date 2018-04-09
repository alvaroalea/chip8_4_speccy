/****************************************************************************/
/**                                                                        **/
/**                                HiTech.c                                **/
/**                                                                        **/
/** Bug fixes and speed-ups for Hi-Tech C CP/M-80 3.09                     **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#ifdef HI_TECH_C
#ifdef z80

#include <stdlib.h>

/* In the library version, c and n and reversed */
void *memset (void *p,int c,size_t n)
{
 register char *_p=(char*)p;
 while (n--) *_p++=c;
 return p;
}

#asm
        ; Sped up shift logical right routine
        ; HL=HL>>B
        global  shlr
shlr:   ld      a,b
        or      a               ; Check for zero shift
        ret     z
        sub     8               ; Is the shift >8 ?
        jp      nc,1f           ; yup
        srl     h               ; Shift is <8, do it the hard way
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        dec     b
        ret     z
        srl     h
        rr      l
        ret
1:      ld      l,h             ; Shift 8 bits
        ld      h,0
        ret     z               ; Return if shift==8
        srl     l               ; If not, shift some more
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        dec     a
        ret     z
        srl     l
        ret
#endasm

#endif  /* z80 */
#endif  /* HI_TECH_C */
