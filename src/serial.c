/*
 *  Hamlib Interface - serial communication low-level support
 *  Copyright (c) 2000-2002 by Stephane Fillod and Frank Singleton
 *  Parts of the PTT handling are derived from soundmodem, an excellent
 *  ham packet softmodem written by Thomas Sailer, HB9JNX.
 *
 *		$Id: serial.c,v 1.25.2.1 2003-02-25 06:01:15 dedmons Exp $
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef HAVE_TERMIOS_H
#include <termios.h> /* POSIX terminal control definitions */
#endif
#ifdef HAVE_TERMIO_H
#include <termio.h>
#else	/* sgtty */
#include <sgtty.h>
#endif

/* for CTS/RTS and DTR/DSR control under Win32 --SF */
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_WINIOCTL_H
#include <winioctl.h>
#endif
#ifdef HAVE_WINBASE_H
#include <winbase.h>
#endif

#include <hamlib/rig.h>
#include "serial.h"
#include "misc.h"

#ifdef HAVE_SYS_IOCCOM_H
#include <sys/ioccom.h>
#endif

#ifdef HAVE_LINUX_PPDEV_H
#include <linux/ppdev.h>
#endif


/*
 * Open serial port using rig.state data
 *
 *
 * input: ptr to rig.state structure, with serial port
 *        populated. (eg: "/dev/ttyS1").
 * 
 * returns:
 *
 * 0 if OK or negative value on error.
 *
 */

int serial_open(port_t *rp) {

  int fd;				/* File descriptor for the port */
  int err;

  if (!rp)
		  return -RIG_EINVAL;

  /*
   * Open in Non-blocking mode. Watch for EAGAIN errors!
   */
  fd = open(rp->pathname, O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd == -1) {
    
    /* Could not open the port. */
    
    rig_debug(RIG_DEBUG_ERR, "serial_open: Unable to open %s - %s\n", 
						rp->pathname, strerror(errno));
    return -RIG_EIO;
  }
 
  rp->fd = fd;

  err = serial_setup(rp);
  if (err != RIG_OK) {
		  close(fd);
		  return err;
  }

  rp->stream = fdopen(fd, "r+b");
  if (rp->stream == NULL) {
		rig_debug(RIG_DEBUG_ERR, "open_serial: fdopen failed: %s\n", 
					strerror(errno));
		close(fd);
		return -RIG_EIO;		/* arg, so close! */
  }

  return RIG_OK;
}


int serial_setup(port_t *rp)
{
  speed_t speed;			/* serial comm speed */
  int fd;
#ifdef HAVE_TERMIOS_H
  struct termios options;
#elif defined(HAVE_TERMIO_H)
  struct termio options;
#elif defined(HAVE_SGTTY_H)
  struct sgttyb sg;
#else
#error "No term control supported!"
#endif

  if (!rp)
		  return -RIG_EINVAL;

  fd = rp->fd;

  /*
   * Get the current options for the port...
   */
  
#ifdef HAVE_TERMIOS_H
  tcgetattr(fd, &options);
#elif defined(HAVE_TERMIO_H)
  ioctl (fd, TCGETA, &options);
#else	/* sgtty */
  ioctl (fd, TIOCGETP, &sg);
#endif

#ifdef HAVE_CFMAKERAW
  cfmakeraw(&options); /* Set serial port to RAW mode by default. */
#endif

  /*
   * Set the baud rates to requested values
   */

  switch(rp->parm.serial.rate) {
  case 300:
	speed = B300;		/* yikes... */
    break;
  case 1200:
	speed = B1200;
    break;
  case 2400:
	speed = B2400;
    break;
  case 4800:
	speed = B4800;
    break;
  case 9600:
	speed = B9600;
    break;
  case 19200:
	speed = B19200;
    break;
  case 38400:
	speed = B38400;
    break;
  case 57600:
	speed = B57600;		/* cool.. */
    break;
  case 115200:
	speed = B115200;	/* awsome! */
    break;
  default:
    rig_debug(RIG_DEBUG_ERR, "open_serial: unsupported rate specified: %d\n", 
					rp->parm.serial.rate);
	close(fd);
    return -RIG_ECONF;
  }
  /* TODO */
  cfsetispeed(&options, speed);
  cfsetospeed(&options, speed);

  /*
   * Enable the receiver and set local mode...
   */
  
  options.c_cflag |= (CLOCAL | CREAD);

/*
 * Set data to requested values.
 *
 */

  switch(rp->parm.serial.data_bits) {
  case 7:
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    break;
  case 8:
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    break;
  default:
    rig_debug(RIG_DEBUG_ERR,"open_serial: unsupported serial_data_bits "
					"specified: %d\n", rp->parm.serial.data_bits);
	close(fd);
    return -RIG_ECONF;
    break;
  }

/*
 * Set stop bits to requested values.
 *
 */  

  switch(rp->parm.serial.stop_bits) {
  case 1:
    options.c_cflag &= ~CSTOPB;
    break;
  case 2:
    options.c_cflag |= CSTOPB;
    break;

  default:
    rig_debug(RIG_DEBUG_ERR, "open_serial: unsupported serial_stop_bits "
					"specified: %d\n",
					rp->parm.serial.stop_bits);
	close(fd);
    return -RIG_ECONF;
    break;
  }

/*
 * Set parity to requested values.
 *
 */  

  switch(rp->parm.serial.parity) {
  case RIG_PARITY_NONE:
    options.c_cflag &= ~PARENB;
    break;
  case RIG_PARITY_EVEN:
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    break;
  case RIG_PARITY_ODD:
    options.c_cflag |= PARENB;
    options.c_cflag |= PARODD;
    break;
  default:
    rig_debug(RIG_DEBUG_ERR, "open_serial: unsupported serial_parity "
					"specified: %d\n",
					rp->parm.serial.parity);
	close(fd);
    return -RIG_ECONF;
    break;
  }


/*
 * Set flow control to requested mode
 *
 */  

  switch(rp->parm.serial.handshake) {
  case RIG_HANDSHAKE_NONE:
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag &= ~IXON;
    break;
  case RIG_HANDSHAKE_XONXOFF:
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag |= IXON;		/* Enable Xon/Xoff software handshaking */
    break;
  case RIG_HANDSHAKE_HARDWARE:
    options.c_cflag |= CRTSCTS;		/* Enable Hardware handshaking */
    options.c_iflag &= ~IXON;
    break;
  default:
    rig_debug(RIG_DEBUG_ERR, "open_serial: unsupported flow_control "
					"specified: %d\n",
					rp->parm.serial.handshake);
	close(fd);
    return -RIG_ECONF;
    break;
  }

  /*
   * Choose raw input, no preprocessing please ..
   */

#if defined(HAVE_TERMIOS_H) || defined(HAVE_TERMIO_H)
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  /*
   * Choose raw output, no preprocessing please ..
   */

  options.c_oflag &= ~OPOST;

#else	/* sgtty */
  sg.sg_flags = RAW;
#endif


  /*
   * Flush serial port
   */

  tcflush(fd, TCIFLUSH);

  /*
   * Finally, set the new options for the port...
   */
  
#ifdef HAVE_TERMIOS_H
  if (tcsetattr(fd, TCSANOW, &options) == -1) {
		rig_debug(RIG_DEBUG_ERR, "open_serial: tcsetattr failed: %s\n", 
					strerror(errno));
		close(fd);
		return -RIG_ECONF;		/* arg, so close! */
  }
#elif defined(HAVE_TERMIO_H)
  if (ioctl(fd, TCSETA, &options) == -1) {
		rig_debug(RIG_DEBUG_ERR, "open_serial: ioctl(TCSETA) failed: %s\n", 
					strerror(errno));
		close(fd);
		return -RIG_ECONF;		/* arg, so close! */
  }
#else	/* sgtty */
  if (ioctl(fd, TIOCSETP, &sg) == -1) {
		rig_debug(RIG_DEBUG_ERR, "open_serial: ioctl(TIOCSETP) failed: %s\n", 
					strerror(errno));
		close(fd);
		return -RIG_ECONF;		/* arg, so close! */
  }
#endif

  return RIG_OK;
}


/*
 * Flush all characters waiting in RX buffer.
 */

int serial_flush( port_t *p )
{
  tcflush(p->fd, TCIFLUSH);

  return RIG_OK;
}

/*
 * ser_ptt_set and par_ptt_set
 * ser_ptt_open/ser_ptt_close & par_ptt_open/par_ptt_close
 *
 * ser_open/ser_close,par_open/par_close to be used for PTT and DCD
 *
 * assumes: p is not NULL
 */

#if defined(_WIN32) || defined(__CYGWIN__)
int ser_open(port_t *p)
{
	const char *path = p->pathname;
	HANDLE h;
	DCB dcb;

	h = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
					OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		rig_debug(RIG_DEBUG_ERR, "Cannot open PTT device \"%s\"\n", path);
		return -1;
	}
	/* Check if it is a comm device */
	if (!GetCommState(h, &dcb)) {
		rig_debug(RIG_DEBUG_ERR, "Device \"%s\" not a COM device\n", path);
		CloseHandle(h);
		return -1;
	} 
	p->handle = h;
	return 0;
}
#else
int ser_open(port_t *p)
{
		return (p->fd = open(p->pathname, O_RDWR | O_NOCTTY));
}
#endif

int ser_close(port_t *p)
{
#if defined(_WIN32) || defined(__CYGWIN__)
		return CloseHandle(p->handle);
#else
		return close(p->fd);
#endif
}

/* 
 * p is supposed to be &rig->state.pttport
 */
int ser_ptt_set(port_t *p, ptt_t pttx)
{
		switch(p->type.ptt) {
#if defined(_WIN32) || defined(__CYGWIN__)
		/*
		 * TODO: log error with 0x%lx GetLastError()
		 */
		case RIG_PTT_SERIAL_RTS:
			return !EscapeCommFunction(p->handle, pttx==RIG_PTT_ON ? 
														SETRTS : CLRRTS);

		case RIG_PTT_SERIAL_DTR:
			return !EscapeCommFunction(p->handle, pttx==RIG_PTT_ON ? 
														SETDTR : CLRDTR);
#else
		case RIG_PTT_SERIAL_RTS:
			{
				unsigned char y = TIOCM_RTS;
				return ioctl(p->fd, pttx==RIG_PTT_ON ? TIOCMBIS : TIOCMBIC, &y);
			}

		case RIG_PTT_SERIAL_DTR:
			{
				unsigned char y = TIOCM_DTR;
				return ioctl(p->fd, pttx==RIG_PTT_ON ? TIOCMBIS : TIOCMBIC, &y);
			}
#endif

		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported PTT type %d\n",
								p->type.ptt);
				return -RIG_EINVAL;
		}
		return RIG_OK;
}

/*
 * assumes pttx not NULL
 */
int ser_ptt_get(port_t *p, ptt_t *pttx)
{

		switch(p->type.ptt) {
#if defined(_WIN32) || defined(__CYGWIN__)
				/* TODO... */
#else
		case RIG_PTT_SERIAL_RTS:
			{
				unsigned char y;
				int status;

				status = ioctl(p->fd, TIOCMGET, &y);
				*pttx = y & TIOCM_RTS ? RIG_PTT_ON:RIG_PTT_OFF;
				return status;
			}

		case RIG_PTT_SERIAL_DTR:
			{
				unsigned char y;
				int status;

				status = ioctl(p->fd, TIOCMGET, &y);
				*pttx = y & TIOCM_DTR ? RIG_PTT_ON:RIG_PTT_OFF;
				return status;
			}
#endif
		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported PTT type %d\n",
								p->type.ptt);
				return -RIG_EINVAL;
		}
		return RIG_OK;
}

/*
 * assumes dcdx not NULL
 * p is supposed to be &rig->state.dcdport
 */
int ser_dcd_get(port_t *p, dcd_t *dcdx)
{

		switch(p->type.dcd) {
#if defined(_WIN32) || defined(__CYGWIN__)
				/* TODO... */
#else
		case RIG_DCD_SERIAL_CTS:
			{
				unsigned char y;
				int status;

				status = ioctl(p->fd, TIOCMGET, &y);
				*dcdx = y & TIOCM_CTS ? RIG_DCD_ON:RIG_DCD_OFF;
				return status;
			}

		case RIG_DCD_SERIAL_DSR:
			{
				unsigned char y;
				int status;

				status = ioctl(p->fd, TIOCMGET, &y);
				*dcdx = y & TIOCM_DSR ? RIG_DCD_ON:RIG_DCD_OFF;
				return status;
			}
#endif
		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported DCD type %d\n",
								p->type.dcd);
				return -RIG_EINVAL;
		}
		return RIG_OK;
}



/*
 * TODO: to be called before exiting: atexit(parport_cleanup)
 * void parport_cleanup() { ioctl(fd, PPRELEASE); }
 */

int par_open(port_t *p)
{
		int fd;

		fd = open(p->pathname, O_RDWR);
#ifdef HAVE_LINUX_PPDEV_H
		ioctl(fd, PPCLAIM);
#endif
		p->fd = fd;
		return fd;
}

int par_close(port_t *p)
{
#ifdef HAVE_LINUX_PPDEV_H
		ioctl(p->fd, PPRELEASE);
#endif
		return close(p->fd);
}

int par_ptt_set(port_t *p, ptt_t pttx)
{
		switch(p->type.ptt) {
#ifdef HAVE_LINUX_PPDEV_H
		case RIG_PTT_PARALLEL:
				{
					unsigned char reg;
					int status;

					status = ioctl(p->fd, PPRDATA, &reg);
					if (pttx == RIG_PTT_ON)
						reg |=   1 << p->parm.parallel.pin;
					else
						reg &= ~(1 << p->parm.parallel.pin);

					return ioctl(p->fd, PPWDATA, &reg);
				}
#endif
		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported PTT type %d\n", 
								p->type.ptt);
				return -RIG_EINVAL;
		}
		return RIG_OK;
}

/*
 * assumes pttx not NULL
 */
int par_ptt_get(port_t *p, ptt_t *pttx)
{
		switch(p->type.ptt) {
#ifdef HAVE_LINUX_PPDEV_H
		case RIG_PTT_PARALLEL:
				{
					unsigned char reg;
					int status;

					status = ioctl(p->fd, PPRDATA, &reg);
					*pttx = reg & (1<<p->parm.parallel.pin) ? 
							RIG_PTT_ON:RIG_PTT_OFF;
					return status;
				}
#endif
		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported PTT type %d\n", 
								p->type.ptt);
				return -RIG_ENAVAIL;
		}
		return RIG_OK;
}

/*
 * assumes dcdx not NULL
 */
int par_dcd_get(port_t *p, dcd_t *dcdx)
{
		switch(p->type.dcd) {
#ifdef HAVE_LINUX_PPDEV_H
		case RIG_DCD_PARALLEL:
				{
					unsigned char reg;
					int status;

					status = ioctl(p->fd, PPRDATA, &reg);
					*dcdx = reg & (1<<p->parm.parallel.pin) ? 
							RIG_DCD_ON:RIG_DCD_OFF;
					return status;
				}
#endif
		default:
				rig_debug(RIG_DEBUG_ERR,"Unsupported DCD type %d\n", 
								p->type.dcd);
				return -RIG_ENAVAIL;
		}
		return RIG_OK;
}

