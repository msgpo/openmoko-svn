/*
 * dfu-programmer
 *
 * $Id: commands.h,v 1.2 2005/09/25 01:27:42 schmidtw Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <usb.h>
#include "arguments.h"

int execute_command( struct usb_dev_handle *device,
                     int interface,
                     struct programmer_arguments args );
#endif
