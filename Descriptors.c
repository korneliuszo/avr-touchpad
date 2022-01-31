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
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */

#include "Descriptors.h"

const USB_Descriptor_HIDReport_Datatype_t PROGMEM MouseReport[] =
{
	/* Use the HID class driver's standard Mouse report.
	 *   Min X/Y Axis values: -1
	 *   Max X/Y Axis values:  1
	 *   Min physical X/Y Axis values (used to determine resolution): -1
	 *   Max physical X/Y Axis values (used to determine resolution):  1
	 *   Buttons: 3
	 *   Absolute screen coordinates: false
	 */
	HID_RI_USAGE_PAGE(8, 0x01),
	HID_RI_USAGE(8, 0x02),
	HID_RI_COLLECTION(8, 0x01),
		HID_RI_USAGE(8, 0x01),
		HID_RI_COLLECTION(8, 0x00),
			HID_RI_USAGE_PAGE(8, 0x09),
			HID_RI_USAGE_MINIMUM(8, 0x01),
			HID_RI_USAGE_MAXIMUM(8, 3),
			HID_RI_LOGICAL_MINIMUM(8, 0x00),
			HID_RI_LOGICAL_MAXIMUM(8, 0x01),
			HID_RI_REPORT_COUNT(8, 3),
			HID_RI_REPORT_SIZE(8, 0x01),
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
			HID_RI_REPORT_COUNT(8, 0x01),
			HID_RI_REPORT_SIZE(8, 5),
			HID_RI_INPUT(8, HID_IOF_CONSTANT),
			HID_RI_USAGE_PAGE(8, 0x01),
			HID_RI_USAGE(8, 0x30),
			HID_RI_USAGE(8, 0x31),
			HID_RI_LOGICAL_MINIMUM(16, 0),
			HID_RI_LOGICAL_MAXIMUM(16, 1023),
			HID_RI_PHYSICAL_MINIMUM(16, 0),
			HID_RI_PHYSICAL_MAXIMUM(16, 1),
			HID_RI_REPORT_COUNT(8, 0x02),
			HID_RI_REPORT_SIZE(8, 16),
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_RI_END_COLLECTION(0),
	HID_RI_END_COLLECTION(0)
};

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM RelayBoard_DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(1,1,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x03EB,
	.ProductID              = 0x2040,
	.ReleaseNumber          = VERSION_BCD(0,0,1),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = STRING_ID_Serial,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM RelayBoard_ConfigurationDescriptor =
{
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 1,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
		},

	.RelayBoardInterface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = 0,
			.AlternateSetting       = 0,

			.TotalEndpoints         = 1,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},
		.HID_MouseHID =
			{
				.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

				.HIDSpec                = VERSION_BCD(1,1,1),
				.CountryCode            = 0x00,
				.TotalReportDescriptors = 1,
				.HIDReportType          = HID_DTYPE_Report,
				.HIDReportLength        = sizeof(MouseReport)
			},

		.HID_ReportINEndpoint =
			{
				.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

				.EndpointAddress        = MOUSE_EPADDR,
				.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
				.EndpointSize           = MOUSE_EPSIZE,
				.PollingIntervalMS      = 0x05
			}
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM RelayBoard_LanguageString =
{
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},

	.UnicodeString          = {LANGUAGE_ID_ENG}
};

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM RelayBoard_ManufacturerString =
{
	.Header                 = {.Size = USB_STRING_LEN(5), .Type = DTYPE_String},

	.UnicodeString          = L"Developer"
};

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM RelayBoard_ProductString =
{
	.Header                 = {.Size = USB_STRING_LEN(10), .Type = DTYPE_String},

	.UnicodeString          = L"DevBoard"
};

/** Serial number string. This is a Unicode string containing the device's unique serial number, expressed as a
 *  series of uppercase hexadecimal digits.
 */
const USB_Descriptor_String_t PROGMEM RelayBoard_SerialString =
{
	.Header                 = {.Size = USB_STRING_LEN(5), .Type = DTYPE_String},

	.UnicodeString          = L"00001"
};

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &RelayBoard_DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &RelayBoard_ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case STRING_ID_Language:
					Address = &RelayBoard_LanguageString;
					Size    = pgm_read_byte(&RelayBoard_LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &RelayBoard_ManufacturerString;
					Size    = pgm_read_byte(&RelayBoard_ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					Address = &RelayBoard_ProductString;
					Size    = pgm_read_byte(&RelayBoard_ProductString.Header.Size);
					break;
				case STRING_ID_Serial:
					Address = &RelayBoard_SerialString;
					Size    = pgm_read_byte(&RelayBoard_SerialString.Header.Size);
					break;
			}
			break;
		case HID_DTYPE_HID:
			Address = &RelayBoard_ConfigurationDescriptor.HID_MouseHID;
			Size    = sizeof(USB_HID_Descriptor_HID_t);
			break;
		case HID_DTYPE_Report:
			Address = &MouseReport;
			Size    = sizeof(MouseReport);
			break;
	}

	*DescriptorAddress = Address;
	return Size;
}

