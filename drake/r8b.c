/*
 *  Hamlib Drake backend - R-8B description
 *  Copyright (c) 2001 by Stephane Fillod
 *
 *	$Id: r8b.c,v 1.1 2002-07-08 22:10:43 fillods Exp $
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <hamlib/rig.h>
#include "drake.h"


/* FIXME! */
#define R8B_MODES (RIG_MODE_NONE)

#define R8B_FUNC (RIG_FUNC_NONE)

#define R8B_LEVEL_ALL (RIG_LEVEL_NONE)

#define R8B_PARM_ALL (RIG_PARM_NONE)

#define R8B_VFO RIG_VFO_A

/*
 * R-8B rig capabilities.
 *
 * TODO: check this with manual or web site.
 */
const struct rig_caps r8b_caps = {
rig_model: RIG_MODEL_DKR8B,
model_name:"R-8B",
mfg_name: "Drake",
version: "0.1",
copyright: "LGPL",
status: RIG_STATUS_ALPHA,
rig_type: RIG_TYPE_RECEIVER,
ptt_type: RIG_PTT_NONE,
dcd_type: RIG_DCD_NONE,
port_type: RIG_PORT_SERIAL,
serial_rate_min: 9600,
serial_rate_max: 9600,
serial_data_bits: 8,
serial_stop_bits: 1,
serial_parity: RIG_PARITY_NONE,
serial_handshake: RIG_HANDSHAKE_NONE,
write_delay: 0,
post_write_delay: 1,
timeout: 200,
retry: 3,

has_get_func: R8B_FUNC,
has_set_func: R8B_FUNC,
has_get_level: R8B_LEVEL_ALL,
has_set_level: RIG_LEVEL_SET(R8B_LEVEL_ALL),
has_get_parm: R8B_PARM_ALL,
has_set_parm: RIG_PARM_SET(R8B_PARM_ALL),
level_gran: {},                 /* FIXME: granularity */
parm_gran: {},
ctcss_list: NULL,
dcs_list: NULL,
preamp:  { RIG_DBLST_END },
attenuator:  { RIG_DBLST_END },
max_rit: Hz(0),
max_xit: Hz(0),
max_ifshift: Hz(0),
targetable_vfo: 0,
transceive: RIG_TRN_OFF,
bank_qty:  0,
chan_desc_sz: 0,

chan_list: {
		RIG_CHAN_END,
	},

rx_range_list1: { RIG_FRNG_END, },    /* FIXME: enter region 1 setting */
tx_range_list1: { RIG_FRNG_END, },
rx_range_list2: {
	{MHz(29),MHz(956),R8B_MODES,-1,-1,R8B_VFO},  // FIXME
	RIG_FRNG_END,
  },
tx_range_list2: { RIG_FRNG_END, },
tuning_steps: {
	 {R8B_MODES,10},	/* FIXME: add other ts */
	 RIG_TS_END,
	},
        /* mode/filter list, remember: order matters! */
filters: {
		{RIG_MODE_AM|RIG_MODE_FM, kHz(8)},
		{RIG_MODE_WFM, kHz(230)},
		RIG_FLT_END,
	},
priv: NULL,

set_freq: drake_set_freq,

};

/*
 * Function definitions below
 */

