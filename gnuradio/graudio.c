/*
 *  Hamlib GNUradio backend - graudio/any rig
 *  Copyright (c) 2001-2003 by Stephane Fillod
 *
 *	$Id: graudio.c,v 1.1.2.1 2003-02-25 15:59:46 dedmons Exp $
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
 * GNU Radio, sound card based.
 *
 */

#define GRAUDIO_FUNC  RIG_FUNC_NONE
#define GRAUDIO_LEVEL (RIG_LEVEL_AF|RIG_LEVEL_RF)
#define GRAUDIO_PARM  RIG_PARM_NONE
#define GRAUDIO_VFO_OP  RIG_OP_NONE
#define GRAUDIO_SCAN	RIG_SCAN_NONE

#define GRAUDIO_MODES (RIG_MODE_WFM|RIG_MODE_FM|RIG_MODE_SSB)

#define GRAUDIO_VFO (RIG_VFO_A|RIG_VFO_B)

static const struct gnuradio_priv_caps graudio_priv_caps = {
	.tuner_model = RIG_MODEL_DUMMY,
	.input_rate = 48000,	/* To be fixed. how? */
	.IF_center_freq = kHz(0),
};


const struct rig_caps graudio_caps = {
  .rig_model =      RIG_MODEL_GRAUDIO,
  .model_name =     "GNU Radio GRAUDIO",
  .mfg_name =       "GNU",
  .version =        "0.1.1",
  .copyright = 	 "GPL",
  .status =         RIG_STATUS_ALPHA,
  .rig_type =       RIG_TYPE_PCRECEIVER,
  .targetable_vfo = 	 RIG_TARGETABLE_ALL,
  .ptt_type =       RIG_PTT_RIG,
  .dcd_type =       RIG_DCD_RIG,
  .port_type =      RIG_PORT_NONE,
  .has_get_func =   GRAUDIO_FUNC,
  .has_set_func =   GRAUDIO_FUNC,
  .has_get_level =  GRAUDIO_LEVEL,
  .has_set_level =  RIG_LEVEL_SET(GRAUDIO_LEVEL),
  .has_get_parm = 	 GRAUDIO_PARM,
  .has_set_parm = 	 RIG_PARM_SET(GRAUDIO_PARM),
  .ctcss_list = 	 NULL,
  .dcs_list =   	 NULL,
  .chan_list = 	 {
			RIG_CHAN_END,
		 },
  .scan_ops = 	 GRAUDIO_SCAN,
  .vfo_ops = 	 GRAUDIO_VFO_OP,
  .transceive =     RIG_TRN_OFF,
  .attenuator =     { RIG_DBLST_END, },
  .preamp = 	 { RIG_DBLST_END, },
  .rx_range_list2 =  { {.start=kHz(100),.end=MHz(30),.modes=GRAUDIO_MODES,
		    .low_power=-1,.high_power=-1,GRAUDIO_VFO},
		    RIG_FRNG_END, },
  .tx_range_list2 =  { RIG_FRNG_END, },
  .tuning_steps =  { {GRAUDIO_MODES,1}, RIG_TS_END, },
  .priv =  (void*)&graudio_priv_caps,

  .rig_init =     gr_init,
  .rig_cleanup =  gr_cleanup,
  .rig_open =     graudio_open,
  .rig_close =    gr_close,

  .set_conf =     gnuradio_set_conf,
  .get_conf =     gnuradio_get_conf,

  .set_freq =     gr_set_freq,
  .get_freq =     gr_get_freq,

  .set_vfo =      gr_set_vfo,
  .get_vfo =      gr_get_vfo,
  .set_mode =     gr_set_mode,
  .get_mode =     gr_get_mode,

  .set_level =	  gnuradio_set_level,
  .get_level =	  gnuradio_get_level,
};

