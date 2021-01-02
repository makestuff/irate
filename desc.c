#include "desc.h"

static const USB_Descriptor_HIDReport_Datatype_t PROGMEM kbdReport[] = {
  HID_RI_USAGE_PAGE(8, 0x01),      // generic desktop
  HID_RI_USAGE(8, 0x06),           // keyboard
  HID_RI_COLLECTION(8, 0x01),      // application
    HID_RI_USAGE_PAGE(8, 0x07),    // key-codes
    HID_RI_USAGE_MINIMUM(8, 0xE0), // keyboard lctrl
    HID_RI_USAGE_MAXIMUM(8, 0xE7), // keyboard right gui
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(8, 0x01),
    HID_RI_REPORT_SIZE(8, 0x01),
    HID_RI_REPORT_COUNT(8, 0x08),
    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_REPORT_COUNT(8, 0x01),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_INPUT(8, HID_IOF_CONSTANT),
    HID_RI_USAGE_PAGE(8, 0x08),    // LEDs
    HID_RI_USAGE_MINIMUM(8, 0x01), // numlock
    HID_RI_USAGE_MAXIMUM(8, 0x05), // kana
    HID_RI_REPORT_COUNT(8, 0x05),
    HID_RI_REPORT_SIZE(8, 0x01),
    HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
    HID_RI_REPORT_COUNT(8, 0x01),
    HID_RI_REPORT_SIZE(8, 0x03),
    HID_RI_OUTPUT(8, HID_IOF_CONSTANT),
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(8, 0x65),
    HID_RI_USAGE_PAGE(8, 0x07),    // keyboard
    HID_RI_USAGE_MINIMUM(8, 0x00), // reserved (no event indicated)
    HID_RI_USAGE_MAXIMUM(8, 0x65), // keyboard application
    HID_RI_REPORT_COUNT(8, 0x06),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
  HID_RI_END_COLLECTION(0)
};

static const USB_Descriptor_HIDReport_Datatype_t PROGMEM mouseReport[] = {
  HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
  HID_RI_USAGE(8, 0x02), /* Mouse */
  HID_RI_COLLECTION(8, 0x01), /* Application */
    HID_RI_USAGE(8, 0x01), /* Pointer */
    HID_RI_COLLECTION(8, 0x00), /* Physical */
      HID_RI_USAGE_PAGE(8, 0x09), /* Button */
      HID_RI_USAGE_MINIMUM(8, 0x01),
      HID_RI_USAGE_MAXIMUM(8, 0x03),
      HID_RI_LOGICAL_MINIMUM(8, 0x00),
      HID_RI_LOGICAL_MAXIMUM(8, 0x01),
      HID_RI_REPORT_COUNT(8, 0x03),
      HID_RI_REPORT_SIZE(8, 0x01),
      HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
      HID_RI_REPORT_COUNT(8, 0x01),
      HID_RI_REPORT_SIZE(8, 0x05),
      HID_RI_INPUT(8, HID_IOF_CONSTANT),
      HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
      HID_RI_USAGE(8, 0x30), /* Usage X */
      HID_RI_USAGE(8, 0x31), /* Usage Y */
      HID_RI_LOGICAL_MINIMUM(8, -1),
      HID_RI_LOGICAL_MAXIMUM(8, 1),
      HID_RI_PHYSICAL_MINIMUM(8, -1),
      HID_RI_PHYSICAL_MAXIMUM(8, 1),
      HID_RI_REPORT_COUNT(8, 0x02),
      HID_RI_REPORT_SIZE(8, 0x08),
      HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),
    HID_RI_END_COLLECTION(0),
  HID_RI_END_COLLECTION(0),
};

static const USB_Descriptor_Device_t PROGMEM devDescriptor = {
  .Header                 = {
    .Size = sizeof(USB_Descriptor_Device_t),
    .Type = DTYPE_Device
  },
  .USBSpecification       = VERSION_BCD(1,1,0),
  .Class                  = USB_CSCP_NoDeviceClass,
  .SubClass               = USB_CSCP_NoDeviceSubclass,
  .Protocol               = USB_CSCP_NoDeviceProtocol,
  .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
  .VendorID               = 0x03EB,
  .ProductID              = 0x204D,
  .ReleaseNumber          = VERSION_BCD(0,0,1),
  .ManufacturerStrIndex   = idManufacturer,
  .ProductStrIndex        = idProduct,
  .SerialNumStrIndex      = NO_DESCRIPTOR,
  .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

static const ConfigDescriptor PROGMEM configDescriptor = {
  .config = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Configuration_Header_t),
      .Type = DTYPE_Configuration
    },
    .TotalConfigurationSize = sizeof(ConfigDescriptor),
    .TotalInterfaces        = 2,
    .ConfigurationNumber    = 1,
    .ConfigurationStrIndex  = NO_DESCRIPTOR,
    .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
    .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
  },
  
  .kbdInterface = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Interface_t),
      .Type = DTYPE_Interface
    },
    .InterfaceNumber        = ifKeyboard,
    .AlternateSetting       = 0x00,
    .TotalEndpoints         = 2,
    .Class                  = HID_CSCP_HIDClass,
    .SubClass               = HID_CSCP_BootSubclass,
    .Protocol               = HID_CSCP_KeyboardBootProtocol,
    .InterfaceStrIndex      = NO_DESCRIPTOR
  },
  
  .kbdHID = {
    .Header = {
      .Size = sizeof(USB_HID_Descriptor_HID_t),
      .Type = HID_DTYPE_HID
    },
    .HIDSpec                = VERSION_BCD(1,1,1),
    .CountryCode            = 0x00,
    .TotalReportDescriptors = 1,
    .HIDReportType          = HID_DTYPE_Report,
    .HIDReportLength        = sizeof(kbdReport)
  },
  
  .kbdReportIN = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Endpoint_t),
      .Type = DTYPE_Endpoint
    },
    .EndpointAddress        = KEYBOARD_IN_EPADDR,
    .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = HID_EPSIZE,
    .PollingIntervalMS      = 0x05
  },
  
  .kbdReportOUT = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Endpoint_t),
      .Type = DTYPE_Endpoint
    },
    .EndpointAddress        = KEYBOARD_OUT_EPADDR,
    .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = HID_EPSIZE,
    .PollingIntervalMS      = 0x05
  },

  .mouseInterface = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Interface_t),
      .Type = DTYPE_Interface
    },
    .InterfaceNumber        = ifMouse,
    .AlternateSetting       = 0x00,
    .TotalEndpoints         = 1,
    .Class                  = HID_CSCP_HIDClass,
    .SubClass               = HID_CSCP_BootSubclass,
    .Protocol               = HID_CSCP_MouseBootProtocol,
    .InterfaceStrIndex      = NO_DESCRIPTOR
  },

  .mouseHID = {
    .Header = {
      .Size = sizeof(USB_HID_Descriptor_HID_t),
      .Type = HID_DTYPE_HID
    },
    .HIDSpec                = VERSION_BCD(1,1,1),
    .CountryCode            = 0x00,
    .TotalReportDescriptors = 1,
    .HIDReportType          = HID_DTYPE_Report,
    .HIDReportLength        = sizeof(mouseReport)
  },

  .mouseReportIN = {
    .Header = {
      .Size = sizeof(USB_Descriptor_Endpoint_t),
      .Type = DTYPE_Endpoint
    },
    .EndpointAddress        = MOUSE_IN_EPADDR,
    .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = HID_EPSIZE,
    .PollingIntervalMS      = 0x05
  }
};

static const USB_Descriptor_String_t PROGMEM languageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
static const USB_Descriptor_String_t PROGMEM manufacturerString = USB_STRING_DESCRIPTOR(L"MakeStuff");
static const USB_Descriptor_String_t PROGMEM productString = USB_STRING_DESCRIPTOR(L"Irate Remote");

uint16_t CALLBACK_USB_GetDescriptor(
  const uint16_t wValue, const uint16_t wIndex, const void** const descAddress)
{
  const uint8_t descType = (wValue >> 8);
  const uint8_t descNum = (wValue & 0xFF);

  switch (descType) {
    case DTYPE_Device:
      *descAddress = &devDescriptor;
      return sizeof(USB_Descriptor_Device_t);

    case DTYPE_Configuration:
      *descAddress = &configDescriptor;
      return sizeof(ConfigDescriptor);

    case DTYPE_String:
      switch (descNum) {
        case idLanguage:
          *descAddress = &languageString;
          return pgm_read_byte(&languageString.Header.Size);

        case idManufacturer:
          *descAddress = &manufacturerString;
          return pgm_read_byte(&manufacturerString.Header.Size);

        case idProduct:
          *descAddress = &productString;
          return pgm_read_byte(&productString.Header.Size);
      }

    case HID_DTYPE_HID:
      switch (wIndex) {
        case ifKeyboard: *descAddress = &configDescriptor.kbdHID;   break;
        case ifMouse:    *descAddress = &configDescriptor.mouseHID; break;
        default:         *descAddress = NULL; return NO_DESCRIPTOR;
      }
      return sizeof(USB_HID_Descriptor_HID_t);

    case HID_DTYPE_Report:
      switch (wIndex) {
        case ifKeyboard: *descAddress = &kbdReport;   return sizeof(kbdReport);
        case ifMouse:    *descAddress = &mouseReport; return sizeof(mouseReport);
        default:         *descAddress = NULL; return NO_DESCRIPTOR;
      }
  }
  
  *descAddress = NULL;
  return NO_DESCRIPTOR;
}
