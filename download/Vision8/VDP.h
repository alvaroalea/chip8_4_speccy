/****************************************************************************/
/**                                                                        **/
/**                                 VDP.h                                  **/
/**                                                                        **/
/** Routines for use with TMS9918 and compatible VDPs                      **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#ifndef __VDP_H
#define __VDP_H

unsigned char vdp_get_ctrl_port (void);
unsigned char vdp_get_data_port (void);
void vdp_set_write_address (int offset);
void vdp_set_read_address (int offset);
unsigned char vdp_read_data (void);
unsigned char vdp_read_status (void);
void vdp_write_ctrl (unsigned char data);
void vdp_write_data (unsigned char data);
void vdp_set_register (unsigned char reg,unsigned char val);
void vdp_fill_video_ram (int offset,int count,unsigned char val);
void vdp_write_video_ram (int offset,unsigned char *addr,int count);
void vdp_read_video_ram (int offset,unsigned char *addr,int count);

#endif
