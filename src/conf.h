/*
 *  Hamlib Interface - configuration header
 *  Copyright (c) 2000,2001,2002 by Stephane Fillod and Frank Singleton
 *
 *		$Id: conf.h,v 1.3.2.1 2003-02-25 06:01:12 dedmons Exp $
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _CONF_H
#define _CONF_H 1

#include <hamlib/rig.h>
#include "token.h"

int frontend_set_conf(RIG *rig, token_t token, const char *val);
int frontend_get_conf(RIG *rig, token_t token, char *val);

#endif /* _CONF_H */
