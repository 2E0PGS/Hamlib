/*
 *  Hamlib AOR backend - AR8200 description
 *  Copyright (c) 2000,2001,2002 by Stephane Fillod
 *
 *	$Id: ar8200.c,v 1.14.2.1 2003-02-25 06:00:43 dedmons Exp $
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
#include "aor.h"


#define AR8200_MODES (RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB|RIG_MODE_FM|RIG_MODE_WFM)

#define AR8200_FUNC (RIG_FUNC_TSQL|RIG_FUNC_ABM)

#define AR8200_LEVEL (RIG_LEVEL_ATT|RIG_LEVEL_AGC|RIG_LEVEL_SQL|RIG_LEVEL_SQLSTAT|RIG_LEVEL_STRENGTH)

#define AR8200_PARM (RIG_PARM_APO|RIG_PARM_BACKLIGHT|RIG_PARM_BEEP)

#define AR8200_VFO_OPS (RIG_OP_MCL|RIG_OP_UP|RIG_OP_DOWN|RIG_OP_LEFT|RIG_OP_RIGHT)

#define AR8200_VFO_ALL (RIG_VFO_A|RIG_VFO_B)

/*
 * ar8200 rig capabilities.
 * Notice that some rigs share the same functions.
 * Also this struct is READONLY!
 *
 * part of info from http://www.aoruk.com/8200.htm
 */
const struct rig_caps ar8200_caps = {
.rig_model =  RIG_MODEL_AR8200,
.model_name = "AR8200",
.mfg_name =  "AOR",
.version =  "0.1",
.copyright =  "LGPL",
.status =  RIG_STATUS_UNTESTED,
.rig_type =  RIG_TYPE_SCANNER,
.ptt_type =  RIG_PTT_NONE,
.dcd_type =  RIG_DCD_NONE,
.port_type =  RIG_PORT_SERIAL,
.serial_rate_min =  4800,
.serial_rate_max =  19200,
.serial_data_bits =  8,
.serial_stop_bits =  2,
.serial_parity =  RIG_PARITY_NONE,
.serial_handshake =  RIG_HANDSHAKE_XONXOFF,
.write_delay =  0,
.post_write_delay =  0,
.timeout =  200,
.retry =  3,
.has_get_func =  RIG_FUNC_NONE,
.has_set_func =  AR8200_FUNC,
.has_get_level =  AR8200_LEVEL,
.has_set_level =  RIG_LEVEL_SET(AR8200_LEVEL),
.has_get_parm =  RIG_PARM_NONE,
.has_set_parm =  RIG_PARM_NONE,    /* FIXME: parms */
.level_gran =  {},                 /* FIXME: granularity */
.parm_gran =  {},
.ctcss_list =  NULL,				/* FIXME: CTCSS list */
.dcs_list =  NULL,
.preamp =   { RIG_DBLST_END, },
.attenuator =   { 20, RIG_DBLST_END, },	/* TBC */
.max_rit =  Hz(0),
.max_xit =  Hz(0),
.max_ifshift =  Hz(0),
.targetable_vfo =  0,
.transceive =  RIG_TRN_RIG,
.bank_qty =   20,
.chan_desc_sz =  12,
.vfo_ops =  AR8200_VFO_OPS,

.chan_list =  { RIG_CHAN_END, },	/* FIXME: memory channel list: 1000 memories */

.rx_range_list1 =  { RIG_FRNG_END, },    /* FIXME: enter region 1 setting */
.tx_range_list1 =  { RIG_FRNG_END, },
.rx_range_list2 =  {
	{kHz(100),MHz(2040),AR8200_MODES,-1,-1,AR8200_VFO_ALL},
	RIG_FRNG_END,
  }, /* rx range */
.tx_range_list2 =  { RIG_FRNG_END, },	/* no tx range, this is a scanner! */

.tuning_steps =  {
	 {AR8200_MODES,50},
	 {AR8200_MODES,100},
	 {AR8200_MODES,kHz(1)},
	 {AR8200_MODES,kHz(5)},
	 {AR8200_MODES,kHz(9)},
	 {AR8200_MODES,kHz(10)},
	 {AR8200_MODES,12500},
	 {AR8200_MODES,kHz(20)},
	 {AR8200_MODES,kHz(25)},
	 {AR8200_MODES,kHz(100)},
	 {AR8200_MODES,MHz(1)},
#if 0 
	 {AR8200_MODES,0},	/* any tuning step */
#endif
	 RIG_TS_END,
	},
        /* mode/filter list, .remember =  order matters! */
.filters =  {
        /* mode/filter list, .remember =  order matters! */
		{RIG_MODE_SSB|RIG_MODE_CW, kHz(3)}, 
		{RIG_MODE_AM, kHz(9)},
		{RIG_MODE_AM, kHz(3)},
		{RIG_MODE_FM|RIG_MODE_AM, kHz(12)}, 
		{RIG_MODE_FM, kHz(9)}, 
		{RIG_MODE_WFM, kHz(230)}, /* 50kHz at -3dB, 380kHz at -20dB */
		RIG_FLT_END,
	},

.priv =  NULL,
.rig_init =  NULL,
.rig_cleanup =  NULL,
.rig_open =  NULL,
.rig_close =  aor_close,

.set_freq =  aor_set_freq,
.get_freq =  aor_get_freq,
.set_vfo =  aor_set_vfo,
.get_vfo =  aor_get_vfo,
.set_mode =  aor_set_mode,
.get_mode =  aor_get_mode,

.set_ts =  aor_set_ts,
.set_powerstat =  aor_set_powerstat,
.vfo_op =  aor_vfo_op,
.get_info =  aor_get_info,

};

/*
 * Function definitions below
 */


