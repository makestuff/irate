#include <LUFA/Drivers/USB/USB.h>
#include "usb.h"
#include "desc.h"
#include "ir.h"
#include "mouse.h"

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

// Create keyboard report based on the detected state of the IR buttons. Several
// buttons (e.g BC_VOLUME_*) are not reported because they are interpreted
// directly by the soundbar (i.e the computer doesn't need to do anything).
//
static void createKeyboardReport(USB_KeyboardReport_Data_t* const reportData) {
  const uint16_t state = irGetState();  // get current button-code
  if (state == BC_PLAY_PAUSE) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_SPACE;
  } else if (state == BC_PREVIOUS_TRACK) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_LEFT_ARROW;
  } else if (state == BC_NEXT_TRACK) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_RIGHT_ARROW;
  } else if (state == BC_UP_ARROW) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_UP_ARROW;
  } else if (state == BC_DOWN_ARROW) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_DOWN_ARROW;
  } else if (state == BC_ENTER) {
    reportData->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
  } else if (state == BC_MENU) {
    reportData->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
    reportData->KeyCode[0] = HID_KEYBOARD_SC_M;
  }
}

// Create mouse report based on the state of the Minimus's single button, and a
// timer which moves the mouse-pointer in a slow 16x16-pixel square, moving one
// pixel every six seconds.
//
static void createMouseReport(USB_MouseReport_Data_t* const reportData) {
  if (mouseIsReportDue()) {
    static uint8_t count = 0;
    if (count < 16) {
      reportData->X = 1;   // move right 16 pixels
    } else if (count < 32) {
      reportData->Y = 1;   // move down 16 pixels
    } else if (count < 48) {
      reportData->X = -1;  // move left 16 pixels
    } else {
      reportData->Y = -1;  // move up 16 pixels
    }
    ++count;
    count &= 63;
  }
  if (mouseIsButtonPressed()) {
    reportData->Button = (1<<0);
  }
}

// Called repeatedly from the main loop. This constructs the reports and decides
// whether to send them or not.
//
void usbSendReceive(void) {
  if (USB_DeviceState == DEVICE_STATE_Configured) {
    static USB_KeyboardReport_Data_t prevKeyboardReport = {0,};
    static uint8_t prevButtonState = 0;
    USB_KeyboardReport_Data_t thisKeyboardReport = {0,};
    USB_MouseReport_Data_t    thisMouseReport    = {0,};
    const bool timeout = (idleInit != 0 && idleRemaining == 0);

    // Maybe reset timer
    if (timeout) {
      idleRemaining = idleInit;
    }
    
    // Construct keypress report, and figure out whether to send it
    createKeyboardReport(&thisKeyboardReport);
    const bool reportDiff = memcmp(
      &prevKeyboardReport, &thisKeyboardReport,
      sizeof(USB_KeyboardReport_Data_t)
    );
    Endpoint_SelectEndpoint(KEYBOARD_IN_EPADDR);
    if (Endpoint_IsReadWriteAllowed() && (timeout || reportDiff)) {
      prevKeyboardReport = thisKeyboardReport;
      Endpoint_Write_Stream_LE(&thisKeyboardReport, sizeof(thisKeyboardReport), NULL);
      Endpoint_ClearIN();
    }

    // Construct mouse report, and figure out whether to send it
    Endpoint_SelectEndpoint(MOUSE_IN_EPADDR);
    if (Endpoint_IsReadWriteAllowed()) {
      createMouseReport(&thisMouseReport);
      if (thisMouseReport.X || thisMouseReport.Y || thisMouseReport.Button != prevButtonState) {
        prevButtonState = thisMouseReport.Button;
        Endpoint_Write_Stream_LE(&thisMouseReport, sizeof(thisMouseReport), NULL);
        Endpoint_ClearIN();
      }
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
  Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR,  EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(KEYBOARD_OUT_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(MOUSE_IN_EPADDR,     EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void) {
  switch (USB_ControlRequest.bRequest) {
  case HID_REQ_GetReport:
    if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
    {
      switch (USB_ControlRequest.wIndex) {
        case ifKeyboard: {
          USB_KeyboardReport_Data_t reportData;
          createKeyboardReport(&reportData);
          Endpoint_ClearSETUP();
          Endpoint_Write_Control_Stream_LE(&reportData, sizeof(reportData));
          Endpoint_ClearOUT();
          break;
        }

        case ifMouse: {
          USB_MouseReport_Data_t reportData;
          createMouseReport(&reportData);
          Endpoint_ClearSETUP();
          Endpoint_Write_Control_Stream_LE(&reportData, sizeof(reportData));
          Endpoint_ClearOUT();
          break;
        }
      }
    }
    break;
    
  case HID_REQ_SetReport:
    if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
    {
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
