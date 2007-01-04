/*  openmoko-dialer-window-outgoing.h
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
 #include "moko-dialer-includes.h"

 #ifndef _OPENMOKO_DIALER_WINDOW_INCOMING_H
#define _OPENMOKO_DIALER_WINDOW_INCOMING_H

#ifdef __cplusplus



extern "C"

{
#endif



gint window_incoming_init( MOKO_DIALER_APP_DATA* p_dialer_data);

void window_incoming_prepare(MOKO_DIALER_APP_DATA * appdata);
#ifdef __cplusplus
}
#endif

#endif 
