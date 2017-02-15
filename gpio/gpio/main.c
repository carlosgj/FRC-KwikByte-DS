/*
*
* Sample application to toggle a GPIO pin and read another
*  on the KB9260 and related products.
*
* Copyright (C) 2009 KwikByte <www.kwikbyte.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* or visit http://www.gnu.org/copyleft/gpl.html
*
* 22MAY2009 	Initial creation (KwikByte)
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "AT91SAM9260.h"

/* ********************** LOCAL DEFINES ****************************** */

/* We will use signals in header SV3 for this demo
 *
 * Reference: SV3 pin 1  = +3.3V
 *            SV3 pin 9  = GND
 *            SV3 pin 10 = GND
 *
 * Use PC14 (SV3 pin 2) as output pin
 * Use PC5  (SV3 pin 4) as input pin
 *
 * see AT91SAM9260 datasheet for more information
 */
//#define	OUTPUT_PIN	AT91C_PIO_PC14
#define	INPUT_PIN	AT91C_PIO_PB21

static int						memMapFile;
static AT91PS_PIOMAP			at91PioCtrlr;

/* ************************ UTILITY FUNCTIONS ******************************* */


/*
 * Read the input pin
 */
static unsigned GetInput(void)
{
	if ((at91PioCtrlr->PIOC_PDSR) & (INPUT_PIN))
	{
		return (1);
	}

	return (0);	
}


/* *************** HARDWARE SUPPORT FUNCTIONS ************************ */


/*
 * Access the hardware
 */
static int OpenSystemController(void)
{
	if ((memMapFile = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
	{
		printf("ERROR: Unable to open /dev/mem\n");
		return (errno);
	}

	at91PioCtrlr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_SHARED, memMapFile, (unsigned)AT91C_BASE_AIC);

	if (at91PioCtrlr == MAP_FAILED)
	{
		printf("ERROR: Unable to mmap the system controller\n");
		close (memMapFile);
		memMapFile = -1;
		return (errno);
	}

	/* set digital input ports */
	at91PioCtrlr->PIOC_ODR		= INPUT_PIN;
	at91PioCtrlr->PIOC_IFER		= INPUT_PIN;
	at91PioCtrlr->PIOC_PPUDR	= INPUT_PIN;
	at91PioCtrlr->PIOC_PER		= INPUT_PIN;

	/* set digital output ports init value = 0 */
	//at91PioCtrlr->PIOC_CODR		= OUTPUT_PIN;
	//at91PioCtrlr->PIOC_PPUDR	= OUTPUT_PIN;
	//at91PioCtrlr->PIOC_PER		= OUTPUT_PIN;
	//at91PioCtrlr->PIOC_OER		= OUTPUT_PIN;

	/* sync memfile */

	return (0);
}


/* ************************ MAIN LOOPS ******************************* */


int main(int argc, char **argv)
{
	if (OpenSystemController())
	{
		printf("Unable to map hardware resources\n");
		return (-1);
	}

	while (1)
	{
		/* loop forever toggling output at 1 second interval */

		/* set high */
		//SetOutput(1);
		//printf ("GPIO output is now high\n");

		/* get input */
		printf ("GPIO input is %d\n", GetInput());

		/* wait for 0.5 seconds */
		usleep(500000);

		/* set low */
		//SetOutput(0);
		//printf ("GPIO output is now low\n");

		/* get input */
		printf ("GPIO input is %d\n", GetInput());

		/* wait for 0.5 seconds */
		usleep(500000);
	}

	return (0); /* never get here */
}

