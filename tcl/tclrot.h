/*
 *  Hamlib tcl/tk bindings - rotator header
 *  Copyright (c) 2001,2002 by Stephane Fillod
 *
 *		$Id: tclrot.h,v 1.1.2.1 2003-02-25 06:01:17 dedmons Exp $
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

#ifndef _TCLROT_H
#define _TCLROT_H

#include <tcl.h>

extern int DoRot(ClientData clientData, Tcl_Interp *interp,
       int objc, Tcl_Obj *CONST objv[]);
extern int DoRotLib(ClientData clientData, Tcl_Interp *interp,
       int objc, Tcl_Obj *CONST objv[]);

#endif /* _TCLROT_H */
