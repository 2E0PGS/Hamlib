/*
 *  Hamlib Tentec backend - main file
 *  Copyright (c) 2001,2002 by Stephane Fillod
 *
 *	$Id: tentec.c,v 1.5.2.1 2003-02-25 06:01:17 dedmons Exp $
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

#include <stdlib.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <math.h>

#include <hamlib/rig.h>
#include <serial.h>
#include <misc.h>
#include <cal.h>

#include "tentec.h"

static void tentec_tuning_factor_calc(RIG *rig);

#define EOM "\015"	/* CR */

#define TT_AM  '0'
#define TT_USB '1'
#define TT_LSB '2'
#define TT_CW  '3'
#define TT_FM  '4'

static int tentec_filters[] = {
  6000,5700,5400,5100,4800,4500,4200,3900,3600,3300,3000,2850,2700,2550,2400,
  2250,2100,1950,1800,
  1650,1500,1350,1200,1050, 900, 750, 675, 600, 525, 450, 375, 330, 300,8000
};



/*
 * tentec_transaction
 * read exactly data_len bytes
 * We assume that rig!=NULL, rig->state!= NULL, data!=NULL, data_len!=NULL
 * Otherwise, you'll get a nice seg fault. You've been warned!
 */
int tentec_transaction(RIG *rig, const char *cmd, int cmd_len, char *data, int *data_len)
{
	int retval;
	struct rig_state *rs;

	rs = &rig->state;

	serial_flush(&rs->rigport);

	retval = write_block(&rs->rigport, cmd, cmd_len);
	if (retval != RIG_OK)
			return retval;

	/* no data expected, TODO: flush input? */
	if (!data || !data_len)
			return 0;

	*data_len = read_string(&rs->rigport, data, *data_len, "", 0);

	return RIG_OK;
}


/*
 * tentec_init:
 * Basically, it just sets up *priv
 */
int tentec_init(RIG *rig)
{
	struct tentec_priv_data *priv;
	struct tentec_priv_caps *priv_caps;

	priv = (struct tentec_priv_data*)malloc(sizeof(struct tentec_priv_data));

	if (!priv) {
				/* whoops! memory shortage! */
		return -RIG_ENOMEM;
	}
	priv_caps = (struct tentec_priv_caps *)rig->caps->priv;

	memset(priv, 0, sizeof(struct tentec_priv_data));

	/*
	 * set arbitrary initial status
	 */
	priv->freq = MHz(145);
	priv->mode = RIG_MODE_AM;
	priv->width = kHz(6);
	priv->cwbfo = 0;
	priv->agc = 0.5;	/* medium */
	priv->lnvol = priv->spkvol = 0.0;	/* mute */
	memcpy(&priv->str_cal, &priv_caps->str_cal, sizeof(cal_table_t));

	rig->state.priv = (rig_ptr_t)priv;

	/* tentec_tuning_factor_calc needs rig->state.priv */
	tentec_tuning_factor_calc(rig);

	return RIG_OK;
}

/*
 * Tentec generic tentec_cleanup routine
 * the serial port is closed by the frontend
 */
int tentec_cleanup(RIG *rig)
{
	if (rig->state.priv)
		free(rig->state.priv);

	rig->state.priv = NULL;

	return RIG_OK;
}

/*
 * Tentec transceiver only open routine
 * Restart and set program to execute.
 */
int tentec_trx_open(RIG *rig)
{
	int retval;

	/*
	 * be kind: use XX first, and do 'Dsp Program Execute' only 
	 * in " DSP START" state.
	 */
	retval = tentec_transaction (rig, "P1" EOM, 3, NULL, NULL);
	if (retval != RIG_OK)
		return retval;

	return RIG_OK;
}


/*
 * Tuning Factor Calculations
 * assumes rig!=NULL, rig->state.priv!=NULL
 * assumes priv->mode in supported modes.
 */
static void tentec_tuning_factor_calc(RIG *rig)
{
	struct tentec_priv_data *priv;
	double tfreq, adjtfreq, mcor, fcor;

	priv = (struct tentec_priv_data *)rig->state.priv;

	switch (priv->mode) {
	case RIG_MODE_AM:
	case RIG_MODE_FM:
			mcor=0; break;
	case RIG_MODE_CW: 
	case RIG_MODE_LSB:
			mcor=-1; break;
	case RIG_MODE_USB:
			mcor=1; break;
	default:
			rig_debug(RIG_DEBUG_BUG,
							"tentec_tuning_factor_calc: invalid mode!\n");
			mcor=1;
	}
	tfreq = priv->freq/(double)MHz(1);
	fcor = priv->width/2.0+200;

	adjtfreq = tfreq - 0.00125 + (mcor*((fcor+priv->cwbfo)/1000000));

	priv->ctf = floor(adjtfreq*400);
	priv->ftf = floor(((adjtfreq*400) - priv->ctf)*2500*5.46);
	priv->ctf += 18000;
	priv->btf = floor((fcor + priv->cwbfo + 8000)*2.73);
}


/*
 * tentec_set_freq
 * assumes rig!=NULL, rig->state.priv!=NULL
 * assumes priv->mode in AM,CW,LSB or USB.
 */
int tentec_set_freq(RIG *rig, vfo_t vfo, freq_t freq)
{
	struct tentec_priv_data *priv;
	struct rig_state *rs = &rig->state;
	int freq_len, retval;
	char freqbuf[16];

	priv = (struct tentec_priv_data *)rig->state.priv;

	tentec_tuning_factor_calc(rig);

	freq_len = sprintf(freqbuf, "N%c%c%c%c%c%c" EOM, 
						priv->ctf >> 8, priv->ctf & 0xff,
						priv->ftf >> 8, priv->ftf & 0xff,
						priv->btf >> 8, priv->btf & 0xff);
	
	retval = write_block(&rs->rigport, freqbuf, freq_len);
	if (retval != RIG_OK)
			return retval;

	priv->freq = freq;

	return RIG_OK;
}

/*
 * tentec_get_freq
 * Assumes rig!=NULL, freq!=NULL
 */
int tentec_get_freq(RIG *rig, vfo_t vfo, freq_t *freq)
{
	struct tentec_priv_data *priv = (struct tentec_priv_data *)rig->state.priv;

	*freq = priv->freq;

	return RIG_OK;
}

/*
 * tentec_set_mode
 * Assumes rig!=NULL
 */
int tentec_set_mode(RIG *rig, vfo_t vfo, rmode_t mode, pbwidth_t width)
{
	struct tentec_priv_data *priv = (struct tentec_priv_data *)rig->state.priv;
	struct rig_state *rs = &rig->state;
	char ttmode;
	rmode_t saved_mode;
	pbwidth_t saved_width;
	int mdbuf_len, ttfilter, retval;
	char mdbuf[32];

	switch (mode) {
		case RIG_MODE_USB:      ttmode = TT_USB; break;
		case RIG_MODE_LSB:      ttmode = TT_LSB; break;
		case RIG_MODE_CW:       ttmode = TT_CW; break;
		case RIG_MODE_AM:       ttmode = TT_AM; break;
		case RIG_MODE_FM:       ttmode = TT_FM; break;
		default:
			rig_debug(RIG_DEBUG_ERR,
							"tentec_set_mode: unsupported mode %d\n",
							mode);
			return -RIG_EINVAL;
	}

	if (width == RIG_PASSBAND_NORMAL)
			width = rig_passband_normal(rig, mode);

	for (ttfilter=0; tentec_filters[ttfilter] != 0; ttfilter++) {
			if (tentec_filters[ttfilter] == width)
					break;
	}
	if (tentec_filters[ttfilter] != width) {
			rig_debug(RIG_DEBUG_ERR,
							"tentec_set_mode: unsupported width %d\n",
							width);
			return -RIG_EINVAL;
	}

		/* backup current values 
		 * in case we fail to write to port
		 */
	saved_mode = priv->mode;
	saved_width = priv->width;

	priv->mode = mode;
	priv->width = width;

	tentec_tuning_factor_calc(rig);

	mdbuf_len = sprintf(mdbuf,  "W%c" EOM
								"N%c%c%c%c%c%c" EOM
								"M%c" EOM,
						ttfilter, 
						priv->ctf >> 8, priv->ctf & 0xff,
						priv->ftf >> 8, priv->ftf & 0xff,
						priv->btf >> 8, priv->btf & 0xff,
						ttmode);
	retval = write_block(&rs->rigport, mdbuf, mdbuf_len);
	if (retval != RIG_OK) {
		priv->mode = saved_mode;
		priv->width = saved_width;
		return retval;
	}

	return RIG_OK;
}

/*
 * tentec_get_mode
 * Assumes rig!=NULL, mode!=NULL
 */
int tentec_get_mode(RIG *rig, vfo_t vfo, rmode_t *mode, pbwidth_t *width)
{
	struct tentec_priv_data *priv = (struct tentec_priv_data *)rig->state.priv;

	*mode = priv->mode;
	*width = priv->width;

	return RIG_OK;
}


/*
 * tentec_set_level
 * Assumes rig!=NULL
 * FIXME: cannot support PREAMP and ATT both at same time (make sens though)
 */
int tentec_set_level(RIG *rig, vfo_t vfo, setting_t level, value_t val)
{
	struct tentec_priv_data *priv = (struct tentec_priv_data *)rig->state.priv;
	struct rig_state *rs = &rig->state;
	int cmd_len, retval;
	char cmdbuf[32];

	/* Optimize:
	 *   sort the switch cases with the most frequent first
	 */
	switch (level) {
	case RIG_LEVEL_AGC:
		cmd_len = sprintf(cmdbuf, "G%c" EOM, val.f<0.4 ? '1' : (
									val.f>0.6 ? '3' : '2') );
		retval = write_block(&rs->rigport, cmdbuf, cmd_len);
		if (retval == RIG_OK)
				priv->agc = val.f;
		return retval;
	case RIG_LEVEL_AF:
		/* FIXME: support also separate Lineout setting 
		 * -> need to create RIG_LEVEL_LINEOUT ?
		 */
		cmd_len = sprintf(cmdbuf, "C%c%c" EOM, 0, (int)(val.f*63));
		retval = write_block(&rs->rigport, cmdbuf, cmd_len);
		if (retval == RIG_OK) 
				priv->lnvol = priv->spkvol = val.f;
		return retval;

	default:
		rig_debug(RIG_DEBUG_ERR,"Unsupported set_level %d\n", level);
		return -RIG_EINVAL;
	}

	return RIG_OK;
}


/*
 * tentec_get_level
 * Assumes rig!=NULL, val!=NULL
 */
int tentec_get_level(RIG *rig, vfo_t vfo, setting_t level, value_t *val)
{
	struct tentec_priv_data *priv = (struct tentec_priv_data *)rig->state.priv;
	int retval, lvl_len;
	char lvlbuf[32];


	/* Optimize:
	 *   sort the switch cases with the most frequent first
	 */
	switch (level) {
	case RIG_LEVEL_STRENGTH:
			/* read A/D converted value */
		lvl_len = 4;
		retval = tentec_transaction (rig, "X" EOM, 2, lvlbuf, &lvl_len);
		if (retval != RIG_OK)
			return retval;

		if (lvl_len != 4) {
			rig_debug(RIG_DEBUG_ERR,"tentec_get_level: wrong answer"
							"len=%d\n", lvl_len);
			return -RIG_ERJCTED;
		}

		lvlbuf[4] = '\0';
		val->i = rig_raw2val((lvlbuf[1]<<8) + lvlbuf[1], &priv->str_cal);
		break;

	case RIG_LEVEL_AGC:
		val->f = priv->agc;
		break;

	case RIG_LEVEL_AF:
		val->f = priv->spkvol;
		break;

	default:
		rig_debug(RIG_DEBUG_ERR,"Unsupported get_level %d\n", level);
		return -RIG_EINVAL;
	}

	return RIG_OK;
}


/*
 * tentec_get_info
 * Assumes rig!=NULL
 */
const char *tentec_get_info(RIG *rig)
{
		static char buf[100];	/* FIXME: reentrancy */
		int firmware_len, retval;

		/*
		 * protocol version
		 */
		firmware_len = 7;
		retval = tentec_transaction (rig, "?" EOM, 2, buf, &firmware_len);
		if (retval != RIG_OK || firmware_len != 7) {
				rig_debug(RIG_DEBUG_ERR,"tentec_get_info: ack NG, len=%d\n",
								firmware_len);
				return NULL;
		}

		return buf;
}


/*
 * initrigs_tentec is called by rig_backend_load
 */
int initrigs_tentec(void *be_handle)
{
		rig_debug(RIG_DEBUG_VERBOSE, "tentec: _init called\n");

		rig_register(&tt550_caps);
		rig_register(&rx320_caps);

		return RIG_OK;
}

