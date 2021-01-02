#ifndef DESC_H
#define DESC_H

#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

typedef struct {
  USB_Descriptor_Configuration_Header_t config;

  // Keyboard
  USB_Descriptor_Interface_t            kbdInterface;
  USB_HID_Descriptor_HID_t              kbdHID;
  USB_Descriptor_Endpoint_t             kbdReportIN;
  USB_Descriptor_Endpoint_t             kbdReportOUT;

  // Mouse
  USB_Descriptor_Interface_t            mouseInterface;
  USB_HID_Descriptor_HID_t              mouseHID;
  USB_Descriptor_Endpoint_t             mouseReportIN;
} ConfigDescriptor;

enum InterfaceDescriptors_t {
  ifKeyboard = 0, /**< Keyboard interface descriptor ID */
  ifMouse    = 1  /**< Mouse interface descriptor ID */
};

enum StringDescriptors_t {
  idLanguage     = 0, /**< Supported Languages string descriptor ID (must be zero) */
  idManufacturer = 1, /**< Manufacturer string ID */
  idProduct      = 2, /**< Product string ID */
};

#define KEYBOARD_IN_EPADDR  (ENDPOINT_DIR_IN  | 1)
#define KEYBOARD_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define MOUSE_IN_EPADDR     (ENDPOINT_DIR_IN  | 3)
#define HID_EPSIZE 8

uint16_t CALLBACK_USB_GetDescriptor(
  const uint16_t wValue, const uint16_t wIndex, const void** const descAddress)
  ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
