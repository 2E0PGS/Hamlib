/*
 *  Hamlib Kachina backend - 505DSP description
 *  Copyright (c) 2001-2002 by Stephane Fillod
 *
 *	$Id: 505dsp.c,v 1.3.2.1 2003-02-25 06:00:55 dedmons Exp $
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
#include "kachina.h"


#define K505DSP_ALL_MODES (RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB|RIG_MODE_FM)
#define K505DSP_OTHER_TX_MODES (RIG_MODE_CW|RIG_MODE_SSB|RIG_MODE_FM)
#define K505DSP_AM_TX_MODES RIG_MODE_AM

#define K505DSP_FUNC (RIG_FUNC_FAGC|RIG_FUNC_NB|RIG_FUNC_TONE|RIG_FUNC_COMP)

#define K505DSP_LEVEL_ALL (RIG_LEVEL_SQLSTAT|RIG_LEVEL_STRENGTH|RIG_LEVEL_RFPOWER|RIG_LEVEL_KEYSPD|RIG_LEVEL_BKINDL|RIG_LEVEL_CWPITCH)

#define K505DSP_PARM_ALL (RIG_PARM_NONE)

#define K505DSP_VFO (RIG_VFO_A)

#define K505DSP_STR_CAL { 2, { \
		{    0, -60 }, \
		{  127,  20 }, \
	} }

static const struct kachina_priv_caps k505dsp_priv_caps = {
		K505DSP_STR_CAL
};

/*
 * 505DSP rig capabilities.
 *
 * protocol is documented at 
 * 		http://www.kachina-az.com/develope.htm
 *
 * TODO:
 *  - so many ...
 */
const struct rig_caps k505dsp_caps = {
.rig_model =  RIG_MODEL_505DSP,
.model_name = "505DSP",
.mfg_name =  "Kachina",
.version =  "0.1",
.copyright =  "LGPL",
.status =  RIG_STATUS_UNTESTED,
.rig_type =  RIG_TYPE_COMPUTER,
.ptt_type =  RIG_PTT_NONE,
.dcd_type =  RIG_DCD_NONE,
.port_type =  RIG_PORT_SERIAL,
.serial_rate_min =  9600,
.serial_rate_max =  9600,
.serial_data_bits =  8,
.serial_stop_bits =  1,
.serial_parity =  RIG_PARITY_NONE,
.serial_handshake =  RIG_HANDSHAKE_NONE,
.write_delay =  0,
.post_write_delay =  0,
.timeout =  200,
.retry =  2,

.has_get_func =  K505DSP_FUNC,
.has_set_func =  K505DSP_FUNC,
.has_get_level =  K505DSP_LEVEL_ALL,
.has_set_level =  RIG_LEVEL_SET(K505DSP_LEVEL_ALL),
.has_get_parm =  K505DSP_PARM_ALL,
.has_set_parm =  RIG_PARM_SET(K505DSP_PARM_ALL),
.level_gran =  {},                 /* FIXME: granularity */
.parm_gran =  {},
.ctcss_list =  common_ctcss_list,
.dcs_list =  NULL,
.preamp =   { 10, RIG_DBLST_END },
.attenuator =   { 20, RIG_DBLST_END },
.max_rit =  Hz(9900),
.max_xit =  Hz(0),
.max_ifshift =  Hz(1270),
.targetable_vfo =  0,
.transceive =  RIG_TRN_OFF,
.bank_qty =   0,
.chan_desc_sz =  0,

.chan_list =  {
			RIG_CHAN_END,
		},

.rx_range_list1 =  { RIG_FRNG_END, },    /* FIXME: enter region 1 setting */
.tx_range_list1 =  { RIG_FRNG_END, },
.rx_range_list2 =  {
	{kHz(30),MHz(30),K505DSP_ALL_MODES,-1,-1,K505DSP_VFO},
	RIG_FRNG_END,
  },
.tx_range_list2 =  {
    {kHz(1800),MHz(2)-100,K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {kHz(1800),MHz(2)-100,K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {kHz(3500),MHz(4)-100,K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {kHz(3500),MHz(4)-100,K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {MHz(7),kHz(7300),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {MHz(7),kHz(7300),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {kHz(10100),kHz(10150),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {kHz(10100),kHz(10150),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {MHz(14),kHz(14350),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {MHz(14),kHz(14350),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {kHz(18068),kHz(18168),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {kHz(18068),kHz(18168),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {MHz(21),kHz(21450),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {MHz(21),kHz(21450),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {kHz(24895),kHz(24995),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {kHz(24895),kHz(24995),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
    {MHz(28),kHz(29700),K505DSP_OTHER_TX_MODES,W(10),W(100),K505DSP_VFO},
    {MHz(28),kHz(29700),K505DSP_AM_TX_MODES,W(4),W(25),K505DSP_VFO},
	RIG_FRNG_END,
  },
.tuning_steps =  {
	 {K505DSP_ALL_MODES,1},	/* FIXME: add other ts */
	 RIG_TS_END,
	},
        /* mode/filter list, remember: order matters! */
.filters =  {
		{RIG_MODE_SSB, kHz(2.4)},
		{RIG_MODE_SSB, kHz(2.7)},
		{RIG_MODE_SSB, kHz(2.1)},
		{RIG_MODE_SSB, kHz(3.5)},
		{RIG_MODE_SSB, kHz(1.7)},
		{RIG_MODE_CW, kHz(1)},
		{RIG_MODE_CW, Hz(500)},
		{RIG_MODE_CW, kHz(2.4)},
		{RIG_MODE_CW, Hz(200)},
		{RIG_MODE_CW, Hz(100)},
		{RIG_MODE_AM, kHz(6)},
		{RIG_MODE_FM, kHz(6)},
		RIG_FLT_END,
	},
.priv =  (void*)&k505dsp_priv_caps,

.set_freq =  kachina_set_freq,
.set_mode =  kachina_set_mode,

};

/*
 * Function definitions below
 */

