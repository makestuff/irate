#include <LUFA/Drivers/USB/USB.h>
#include "usb.h"
#include "desc.h"
#include "ir.h"

static bool usingReportProtocol = true;
static uint16_t idleInit = 500;
static uint16_t idleRemaining = 0;

typedef enum {
  BC_ON_OFF         = 0x5422,
  BC_UP_ARROW       = 0x2426,
  BC_MENU           = 0x4426,
  BC_DOWN_ARROW     = 0x6426,
  BC_ENTER          = 0x0426,
  BC_PLAY_PAUSE     = 0x6626,
  BC_PREVIOUS_TRACK = 0x0626,
  BC_NEXT_TRACK     = 0x4626,
  BC_VOLUME_UP      = 0x2422,
  BC_SOUND          = 0x0622,
  BC_VOLUME_DOWN    = 0x6422
} ButtonCode;

static void createKeyboardReport(USB_KeyboardReport_Data_t* const reportData) {
  const uint16_t value = getValue();  // get current button-code
  if (value == BC_PLAY_PAUSE) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_SPACE;
  } else if (value == BC_PREVIOUS_TRACK) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_LEFT_ARROW;
  } else if (value == BC_NEXT_TRACK) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_RIGHT_ARROW;
  } else if (value == BC_UP_ARROW) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_UP_ARROW;
  } else if (value == BC_DOWN_ARROW) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_DOWN_ARROW;
  } else if (value == BC_ENTER) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
  } else if (value == BC_MENU) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_M;
  }
}

void usbSendReceive(void) {
  if (USB_DeviceState == DEVICE_STATE_Configured) {
    static USB_KeyboardReport_Data_t prevReport;
    USB_KeyboardReport_Data_t thisReport = {0,};
    bool sendReport = false;
    
    // Construct keypress report, and figure out whether to send it
    createKeyboardReport(&thisReport);
    if (idleInit != 0 && idleRemaining == 0) {
      idleRemaining = idleInit;
      sendReport = true;  // because timeout
    } else if (memcmp(&prevReport, &thisReport, sizeof(USB_KeyboardReport_Data_t)) != 0) {
      sendReport = true;  // because report changed
    }
    
    // Maybe send a keypress report to the host
    Endpoint_SelectEndpoint(KEYBOARD_IN_EPADDR);
    if (Endpoint_IsReadWriteAllowed() && sendReport) {
      prevReport = thisReport;
      Endpoint_Write_Stream_LE(&thisReport, sizeof(thisReport), NULL);
      Endpoint_ClearIN();
    }
    
    // Discard any LED report from the host
    Endpoint_SelectEndpoint(KEYBOARD_OUT_EPADDR);
    if (Endpoint_IsOUTReceived()) {
      if (Endpoint_IsReadWriteAllowed()) {
        const uint8_t dummy = Endpoint_Read_8();  // discard LED status
        (void)dummy;
      }
      Endpoint_ClearOUT();
    }
  }
}

void EVENT_USB_Device_Connect(void) {
  usingReportProtocol = true;
}

void EVENT_USB_Device_ConfigurationChanged(void) {
  Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, KEYBOARD_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(KEYBOARD_OUT_EPADDR, EP_TYPE_INTERRUPT, KEYBOARD_EPSIZE, 1);
  USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void) {
  switch (USB_ControlRequest.bRequest) {
  case HID_REQ_GetReport:
    if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
      USB_KeyboardReport_Data_t kbReportData;
      createKeyboardReport(&kbReportData);
      Endpoint_ClearSETUP();
      Endpoint_Write_Control_Stream_LE(&kbReportData, sizeof(kbReportData));
      Endpoint_ClearOUT();
    }
    break;
    
  case HID_REQ_SetReport:
    if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
      Endpoint_ClearSETUP();
      while (!Endpoint_IsOUTReceived()) {
        if (USB_DeviceState == DEVICE_STATE_Unattached) {
          return;
        }
      }
      const uint8_t dummy = Endpoint_Read_8();  // discard LED status
      (void)dummy;
      Endpoint_ClearOUT();
      Endpoint_ClearStatusStage();
    }
    break;
    
  case HID_REQ_GetProtocol:
    if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
      Endpoint_ClearSETUP();
      Endpoint_Write_8(usingReportProtocol);
      Endpoint_ClearIN();
      Endpoint_ClearStatusStage();
    }
    break;
    
  case HID_REQ_SetProtocol:
    if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
      Endpoint_ClearSETUP();
      Endpoint_ClearStatusStage();
      usingReportProtocol = (USB_ControlRequest.wValue != 0);
    }
    break;
    
  case HID_REQ_SetIdle:
    if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
      Endpoint_ClearSETUP();
      Endpoint_ClearStatusStage();
      idleInit = ((USB_ControlRequest.wValue & 0xFF00) >> 6);  // idleInit = value*4
    }
    break;
    
  case HID_REQ_GetIdle:
    if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
      Endpoint_ClearSETUP();
      Endpoint_Write_8(idleInit >> 2);  // send idleInit/4
      Endpoint_ClearIN();
      Endpoint_ClearStatusStage();
    }
    break;
  }
}

void EVENT_USB_Device_StartOfFrame(void) {
  if (idleRemaining) {
    idleRemaining--;
  }
}
