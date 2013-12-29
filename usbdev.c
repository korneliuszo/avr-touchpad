/*
   LUFA Library
   Copyright (C) Dean Camera, 2013.

   dean [at] fourwalledcubicle [dot] com
   www.lufa-lib.org
   */

/*
   Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
   Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

   Permission to use, copy, modify, distribute, and sell this
   software and its documentation for any purpose is hereby granted
   without fee, provided that the above copyright notice appear in
   all copies and that both that the copyright notice and this
   permission notice and warranty disclaimer appear in supporting
   documentation, and that the name of the author not be used in
   advertising or publicity pertaining to distribution of the
   software without specific, written prior permission.

   The author disclaims all warranties with regard to this
   software, including all implied warranties of merchantability
   and fitness.  In no event shall the author be liable for any
   special, indirect or consequential damages or any damages
   whatsoever resulting from loss of use, data or profits, whether
   in an action of contract, negligence or other tortious action,
   arising out of or in connection with the use or performance of
   this software.
   */

/** \file
 *
 *  Main source file for the RelayBoard program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "usbdev.h"
#include "enter_bootloader.h"

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
		USB_USBTask();
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	USB_Init();

	/* Initialize Needed HW */
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	if(((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE)
				== REQTYPE_CLASS)
			&& ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_RECIPIENT)
				== REQREC_DEVICE))
	{
		if ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION)
				== REQDIR_HOSTTODEVICE)
		{
			switch(USB_ControlRequest.bRequest)
			{
				case 0x01:
					Endpoint_ClearSETUP();
					Endpoint_ClearStatusStage();
					USB_USBTask();
					Delay_MS(200);	

					enter_bootloader();
					break;
			}
		}
		else
		{
			switch(USB_ControlRequest.bRequest)
			{
			}
		}
	}
}
