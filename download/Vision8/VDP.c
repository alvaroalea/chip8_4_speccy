/****************************************************************************/
/**                                                                        **/
/**                                 VDP.c                                  **/
/**                                                                        **/
/** Routines for use with TMS9918 and compatible VDPs                      **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include "VDP.h"

#ifdef MSX
#asm
VDP_CTRL_PORT   equ     99h
VDP_DATA_PORT   equ     98h
#endasm
#else
#asm
VDP_CTRL_PORT   equ     0bfh
VDP_DATA_PORT   equ     0beh
#endasm
#endif

#asm
        psect   text

        global  _vdp_get_ctrl_port
_vdp_get_ctrl_port:
        ld      hl,VDP_CTRL_PORT
        ret

        global  _vdp_get_data_port
_vdp_get_data_port:
        ld      hl,VDP_DATA_PORT
        ret

        global  _vdp_write_ctrl
_vdp_write_ctrl:
        pop     hl
        pop     de
        ld      c,VDP_CTRL_PORT
        out     (c),e
        push    de
        jp      (hl)

        global  _vdp_set_write_address
_vdp_set_write_address:
        pop     hl
        pop     de
        ld      c,VDP_CTRL_PORT
        out     (c),e
        set     6,d
        out     (c),d
        push    de
        jp      (hl)

        global  _vdp_set_read_address
_vdp_set_read_address:
        pop     hl
        pop     de
        ld      c,VDP_CTRL_PORT
        out     (c),e
        nop
        out     (c),d
        push    de
        jp      (hl)

        global  _vdp_read_data
_vdp_read_data:
        ld      c,VDP_DATA_PORT
        in      l,(c)
        ld      h,0
        ret

        global  _vdp_read_status
_vdp_read_status:
        ld      c,VDP_CTRL_PORT
        in      l,(c)
        ld      h,0
        ret

        global  _vdp_write_data
_vdp_write_data:
        pop     hl
        pop     de
        ld      c,VDP_DATA_PORT
        out     (c),e
        push    de
        jp      (hl)

        global  _vdp_set_register
_vdp_set_register:
        pop     hl
        pop     de
        ld      a,e
        pop     de
        ld      c,VDP_CTRL_PORT
        out     (c),e
        set     7,a
        out     (c),a
        push    de
        push    de
        jp      (hl)

        global  _vdp_fill_video_ram
_vdp_fill_video_ram:
        pop     hl
        exx
        pop     hl
        pop     de
        pop     bc
        ld      b,c
        ld      c,VDP_CTRL_PORT
        out     (c),l
        set     6,h
        out     (c),h
        ld      c,VDP_DATA_PORT
1:      out     (c),b
        dec     de
        ld      a,d
        or      e
        jp      nz,1b
        exx
        push    de
        push    de
        push    de
        jp      (hl)

        global  _vdp_write_video_ram
_vdp_write_video_ram:
        pop     hl
        exx
        pop     bc
        pop     hl
        pop     de
        ld      a,c
        ld      c,VDP_CTRL_PORT
        out     (c),a
        set     6,b
        out     (c),b
        ld      c,VDP_DATA_PORT
1:      outi
        dec     de
        ld      a,d
        or      e
        jp      nz,1b
        exx
        push    de
        push    de
        push    de
        jp      (hl)

        global  _vdp_read_video_ram
_vdp_read_video_ram:
        pop     hl
        exx
        pop     bc
        pop     hl
        pop     de
        ld      a,c
        ld      c,VDP_CTRL_PORT
        out     (c),a
        nop
        out     (c),b
        ld      c,VDP_DATA_PORT
1:      ini
        dec     de
        ld      a,d
        or      e
        jp      nz,1b
        exx
        push    de
        push    de
        push    de
        jp      (hl)
#endasm
