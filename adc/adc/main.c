/*
*
* Sample application to read the ADC channels on the KB9260
*  and related products.
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
#define	ADC_MAX			4

/* PC0 = ADC0
 * PC1 = ADC1
 * PC2 = ADC2
 * PC3 = ADC3
 *
 * PC0 is routed to header SV2 pin 6
 * PC1 is routed to header SV2 pin 4
 * PC2 is routed to header SV2 pin 5
 * PC3 is routed to header SV2 pin 3
 *
 * SV2 pin 1 = +3.3VDC
 * SV2 pin 2 = GND
 *
 * The ADC signals range is 0 (GND) to +3.3V
 * They are NOT buffered on-board, so pay attention to the input
 *  impendance of the chip and the output impedance of the driving
 *  source.  In many cases, this is not an issue.
 * Also, follow good anti-alias filtering principles to ensure
 *  good results.
 *
 * see AT91SAM9260 datasheet for more information
 */
static unsigned adcSignalMapping[] = 
	{
	AT91C_PIO_PC0,
	AT91C_PIO_PC1,
	AT91C_PIO_PC2,
	AT91C_PIO_PC3
};

#define	ADC_MASK			(AT91C_PIO_PC0 | AT91C_PIO_PC1 |\
							 AT91C_PIO_PC2 | AT91C_PIO_PC3)

static int						memMapFile;
static AT91PS_PIOMAP			at91PioCtrlr;
static AT91PS_ADC				at91ADC;

/* ************************ UTILITY FUNCTIONS ******************************* */


/*
 * Get the ADC channel reading
 *
 * We are reading all channels and could return data for each, but
 *  this function is only interested in a single channel for this
 *  demo.
 */
static unsigned GetADCValue(unsigned index)
{
	unsigned	retVal[ADC_MAX];
	unsigned	retry;
	unsigned	value;

	if (index >= ADC_MAX)
		/* invalid index */
		return (0);

	/* wait for all conversions to complete	*/
	/* busy loop!							*/
	for (retry = 0; (retry < 1000) && \
		(((at91ADC->ADC_SR) & ADC_MASK) != ADC_MASK); ++retry)
		usleep(1000);

	if (retry >= 1000)
	{
		printf ("ERROR: timeout waiting for ADC conversions complete\n");
		return (0);
	}

	/* get all channel raw values */
	retVal[0] = (at91ADC->ADC_CDR0 & 0x3FF);
	retVal[1] = (at91ADC->ADC_CDR1 & 0x3FF);
	retVal[2] = (at91ADC->ADC_CDR2 & 0x3FF);
	retVal[3] = (at91ADC->ADC_CDR3 & 0x3FF);

	/* use ADC_LCRD to clear EOCx and DRDY */
	value = at91ADC->ADC_LCDR;

	/* start next conversion */
	at91ADC->ADC_CR = AT91C_ADC_START;

	return (retVal[index]);
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

	at91ADC = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_SHARED, memMapFile, (unsigned)AT91C_BASE_ADC);

	if (at91ADC == MAP_FAILED)
	{
		printf("ERROR: Unable to mmap the ADC\n");
		close (memMapFile);
		memMapFile = -1;
		return (errno);
	}

	/* set ADC input signals */
	at91PioCtrlr->PMC_PCER		= (1 << AT91C_ID_ADC);
	at91PioCtrlr->PIOC_PDR		= ADC_MASK;
	at91PioCtrlr->PIOC_ODR		= ADC_MASK;
	at91PioCtrlr->PIOC_PPUDR	= ADC_MASK;
	at91PioCtrlr->PIOC_ASR		= ADC_MASK;

	at91ADC->ADC_CR				= AT91C_ADC_SWRST;
	usleep(1000);
	/* no sleep */
	/* 2.5MHz ADC clock MCLK/((19+1)*2) */
	/* 40 ADC clocks for startup = (4+1)*8  = 16us */
	/* 10 ADC clocks for sample-hold = (9+1) = 2us */
	at91ADC->ADC_MR				= ((9 << 24) | (4 << 16) | (19 << 8));
	at91ADC->ADC_CHER			= ADC_MASK;
	usleep(500);
	at91ADC->ADC_CR				= AT91C_ADC_START;

	/* sync memfile */

	return (0);
}


/* ************************ MAIN LOOPS ******************************* */


int main(int argc, char **argv)
{
	unsigned	adcIndex;
	unsigned	adcValue;
	unsigned	adcConversion;

	if (OpenSystemController())
	{
		printf("Unable to map hardware resources\n");
		return (-1);
	}

	adcIndex = 0;
	while (1)
	{
		/* loop forever reading the ADC channels at about 1 second interval */

		/* read value */
		adcValue = GetADCValue(adcIndex);

		/* rough conversion to millivolts */
		/* prefer to not use float or double data types */
		/* adc is run at 10-bit in this demo */
		adcConversion = adcValue * 3300000;
		adcConversion /= (1023 * 1000);

		printf ("ADC channel %d read raw value of 0x%08x [%dmV]\n",
			adcIndex, adcValue, adcConversion);

		/* wait for about 1 second */
		usleep(999999);

		/* reset index to first ADC if we finished all of them */
		if (++adcIndex >= ADC_MAX)
			adcIndex = 0;
	}

	return (0); /* never get here */
}

