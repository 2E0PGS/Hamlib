/*
 *  Hamlib Microtune backend - main file
 *  Copyright (c) 2003 by Stephane Fillod
 *
 *	$Id: microtune.cc,v 1.1 2003-01-29 23:06:30 fillods Exp $
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this software; see the file COPYING.  If not, write to
 *   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <hamlib/rig.h>

#include "microtune.h"
#include "microtune_eval_board.h"
#include <stdlib.h>

/*
 * TODO: allow these to be modifiable through rig_set_conf
	int	i2c_addr;
	int	reference_divider;
	bool	fast_tuning_p;	 if set, higher charge pump current:
				   faster tuning, worse phase noise
				   for distance < 10kHz to the carrier
 */

#define M4937_I2C_ADDR (0xC2/2)
#define REF_DIVIDER 640
#define FAST_TUNING false

struct module_4937_priv_data {
	microtune_eval_board *board;
	freq_t actual_freq;
};

/*
 * TODO:
 * 	- status iff PLL is locked
 *	- returns the output frequency of the tuner in Hz (->5.75e6) //3x7702.
 */

int module_4937_init(RIG *rig)
{
	struct module_4937_priv_data *priv;
	int           which = 1;      // the parallel port the board is connected to

	priv = (struct module_4937_priv_data*)malloc(sizeof(struct module_4937_priv_data));
	if (!priv) {
		/* whoops! memory shortage! */
		return -RIG_ENOMEM;
	}

	priv->actual_freq = RIG_FREQ_NONE;

	rig_debug(RIG_DEBUG_TRACE, "microtune: if the program segfaults here,"
			" you require ioperm privilege. Use givelp1.\n");

	priv->board = new microtune_eval_board(which);
	if (!priv->board) {
		free(priv);
		return -RIG_ENOMEM;
	}

	rig_debug(RIG_DEBUG_TRACE, "microtune: ioperm okay. "
			"parallel port access granted.\n");
	rig->state.priv = (void*)priv;

	return RIG_OK;
}

int module_4937_open(RIG *rig)
{
	struct module_4937_priv_data *priv = (struct module_4937_priv_data *)rig->state.priv;

	if (!priv->board->board_present_p()) {
		rig_debug(RIG_DEBUG_WARN, "microtune: eval board is NOT present\n");
		return -RIG_EPROTO;
	}

	return RIG_OK;
}

int module_4937_close(RIG *rig)
{
	/* place holder.. */

	return RIG_OK;
}

int module_4937_cleanup(RIG *rig)
{
	struct module_4937_priv_data *priv = (struct module_4937_priv_data *)rig->state.priv;

	if (priv) {
		delete priv->board;
		free(priv);
	}
	rig->state.priv = NULL;

	return RIG_OK;
}

/*
 * It takes about 100 ms for the PLL to settle.
 */
int module_4937_set_freq(RIG *rig, vfo_t vfo, freq_t freq)
{
  double actual_freq;
  bool status;
  
  struct module_4937_priv_data *priv = (struct module_4937_priv_data *)rig->state.priv;

  status = priv->board->set_RF_freq((double)freq, &actual_freq);

  if (!status)
	  return -RIG_EIO;

  priv->actual_freq = (freq_t)actual_freq;
  return RIG_OK;
}


int module_4937_get_freq(RIG *rig, vfo_t vfo, freq_t *freq)
{ 
  struct module_4937_priv_data *priv = (struct module_4937_priv_data *)rig->state.priv;

  *freq = priv->actual_freq;

  return RIG_OK;
}

