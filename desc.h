#ifndef DESC_H
#define DESC_H

#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

typedef struct {
  USB_Descriptor_Configuration_Header_t config;
  USB_Descriptor_Interface_t            hidInterface;
  USB_HID_Descriptor_HID_t              hidKeyboard;
  USB_Descriptor_Endpoint_t             hidReportINEndpoint;
  USB_Descriptor_Endpoint_t             hidReportOUTEndpoint;
} ConfigDescriptor;


enum StringDescriptors_t {
  idLanguage     = 0, /**< Supported Languages string descriptor ID (must be zero) */
  idManufacturer = 1, /**< Manufacturer string ID */
  idProduct      = 2, /**< Product string ID */
};

#define KEYBOARD_IN_EPADDR  (ENDPOINT_DIR_IN  | 1)
#define KEYBOARD_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define KEYBOARD_EPSIZE     8

uint16_t CALLBACK_USB_GetDescriptor(
  const uint16_t wValue, const uint16_t wIndex, const void** const descAddress)
  ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
