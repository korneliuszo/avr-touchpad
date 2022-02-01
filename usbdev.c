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


typedef struct
{
	uint8_t Button; /**< Button mask for currently pressed buttons in the mouse. */
	int16_t  X; /**< Current delta X movement of the mouse. */
	int16_t  Y; /**< Current delta Y movement on the mouse. */
} ATTR_PACKED USB_MouseReport16_Data_t;

static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_MouseReport16_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 0,
				.ReportINEndpoint             =
					{
						.Address              = MOUSE_EPADDR,
						.Size                 = MOUSE_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevMouseHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
			},
	};

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
	}
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
	// Select Vref=AVcc
	ADMUX = (1<<REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);

	USB_Device_EnableSOFEvents();

}


/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	if(((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE)
				== REQTYPE_VENDOR)
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
	else
	{
		HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
	}
}

enum AdcState {
	ADC_STATE_STBY_SET,
	ADC_STATE_STBY_INIT_READ,
	ADC_STATE_STBY_READ_YD,
	ADC_STATE_STBY_READ_XR,
	ADC_STATE_Y_INIT_READ,
	ADC_STATE_Y_READ,
	ADC_STATE_X_INIT_READ,
	ADC_STATE_X_READ,
};

#define PIN_YU 7
#define PIN_XL 4
#define PIN_YD 5
#define PIN_XR 6

static void InitADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	DIDR0 = _BV(PIN_YU) | _BV(PIN_XL) | _BV(PIN_YD) | _BV(PIN_XR);
}

static uint8_t ReadADC(uint16_t *readout)
{
	if(ADCSRA & (1<<ADSC))
		return 0;
	*readout = ADC;
	return 1;
}

static struct {
	uint16_t STBY_YD;
	uint16_t STBY_XR;
	uint16_t Y;
	uint16_t X;
	uint8_t full_update;
} touch_internals;

static struct {
	uint16_t Y;
	uint16_t X;
	uint8_t pressed;
} touch_vals;

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
	static enum AdcState adc_state = ADC_STATE_Y_INIT_READ;
	switch(adc_state)
	{
	case ADC_STATE_STBY_SET:
		PORTF = _BV(PIN_YU);
		DDRF = _BV(PIN_YU) | _BV(PIN_XL);
		adc_state = ADC_STATE_STBY_INIT_READ;
		break;
	case ADC_STATE_STBY_INIT_READ:
		InitADC(PIN_YD);
		adc_state = ADC_STATE_STBY_READ_YD;
		break;
	case ADC_STATE_STBY_READ_YD:
		if(!ReadADC(&touch_internals.STBY_YD))
			break;
		InitADC(PIN_XR);
		adc_state = ADC_STATE_STBY_READ_XR;
		break;
	case ADC_STATE_STBY_READ_XR:
		if(!ReadADC(&touch_internals.STBY_XR))
			break;
		//calculations

		if((((uint32_t)touch_internals.X * touch_internals.STBY_YD / touch_internals.STBY_XR) - touch_internals.X) > 500UL)
		{
			touch_vals.pressed = 0;
			touch_internals.full_update = 0;
		}
		else
		{
			if(touch_internals.full_update)
			{
				touch_vals.X = touch_internals.X;
				touch_vals.Y = touch_internals.Y;
				touch_vals.pressed = 1;
			}
			touch_internals.full_update = 1;
		}
		PORTF = _BV(PIN_YU);
		DDRF = _BV(PIN_YU) | _BV(PIN_YD);
		adc_state = ADC_STATE_Y_INIT_READ;
		break;
	case ADC_STATE_Y_INIT_READ:
		InitADC(PIN_XL);
		adc_state = ADC_STATE_Y_READ;
	case ADC_STATE_Y_READ:
		if(!ReadADC(&touch_internals.Y))
			break;
		PORTF = _BV(PIN_XR);
		DDRF = _BV(PIN_XR) | _BV(PIN_XL);
		adc_state = ADC_STATE_X_INIT_READ;
		break;
	case ADC_STATE_X_INIT_READ:
		InitADC(PIN_YU);
		adc_state = ADC_STATE_X_READ;
	case ADC_STATE_X_READ:
		if(!ReadADC(&touch_internals.X))
			break;
		adc_state = ADC_STATE_STBY_SET;
		break;
	}
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_MouseReport16_Data_t* MouseReport = (USB_MouseReport16_Data_t*)ReportData;

	MouseReport->Y = touch_vals.Y;
	MouseReport->X = touch_vals.X;

	MouseReport->Button = touch_vals.pressed;

	*ReportSize = sizeof(USB_MouseReport16_Data_t);
	return true;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}
