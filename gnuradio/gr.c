/*
 *  Hamlib GNUradio backend - main file
 *  Copyright (c) 2001,2002 by Stephane Fillod
 *
 *	$Id: gr.c,v 1.1 2002-07-06 09:27:38 fillods Exp $
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

#include "gnuradio.h"

/*
 * GNU Radio (hacking version) rig capabilities.
 */

#define GR_FUNC  RIG_FUNC_NONE
#define GR_LEVEL RIG_LEVEL_NONE
#define GR_PARM  RIG_PARM_NONE
#define GR_VFO_OP  RIG_OP_NONE
#define GR_SCAN	RIG_SCAN_NONE

#define GR_MODES (RIG_MODE_WFM)

#define GR_VFO RIG_VFO_A

const struct rig_caps gr_caps = {
  rig_model:     RIG_MODEL_GNURADIO,
  model_name:    "GNU Radio dev",
  mfg_name:      "GNU",
  version:       "0.1",
  copyright:	 "GPL",
  status:        RIG_STATUS_ALPHA,
  rig_type:      RIG_TYPE_PCRECEIVER,
  targetable_vfo:	 0,
  ptt_type:      RIG_PTT_RIG,
  dcd_type:      RIG_DCD_RIG,
  port_type:     RIG_PORT_NONE,
  has_get_func:  GR_FUNC,
  has_set_func:  GR_FUNC,
  has_get_level: GR_LEVEL,
  has_set_level: RIG_LEVEL_SET(GR_LEVEL),
  has_get_parm:	 GR_PARM,
  has_set_parm:	 RIG_PARM_SET(GR_PARM),
  ctcss_list:	 NULL,
  dcs_list:  	 NULL,
  chan_list:	 {
			RIG_CHAN_END,
		 },
  scan_ops:	 GR_SCAN,
  vfo_ops:	 GR_VFO_OP,
  transceive:    RIG_TRN_OFF,
  attenuator:    { RIG_DBLST_END, },
  preamp:	 { RIG_DBLST_END, },
  rx_range_list2: { {start:kHz(150),end:MHz(1500),modes:GR_MODES,
		    low_power:-1,high_power:-1,GR_VFO},
		    RIG_FRNG_END, },
  tx_range_list2: { RIG_FRNG_END, },
  tuning_steps: { {GR_MODES,1}, RIG_TS_END, },
  priv: NULL,	/* priv */

  rig_init:    gr_init,
  rig_cleanup: gr_cleanup,
  rig_open:    gr_open,
  rig_close:   gr_close,

  set_freq:    gr_set_freq,
  get_freq:    gr_get_freq,

  get_vfo:     gr_get_vfo,
  set_mode:    gr_set_mode,
  get_mode:    gr_get_mode,
};


