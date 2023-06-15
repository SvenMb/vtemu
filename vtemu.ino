#if !defined CONFIG_IDF_TARGET_ESP32S3 && !defined CONFIG_IDF_TARGET_ESP32S2
  #error "This sketch will only run on ESP32-S2 or S3"
#endif

#if ARDUINO_USB_MODE
  #error "This sketch should be used when USB is in OTG mode"
#endif

/*******************************************************************************
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

/* More dev device declaration:
   https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
/* More data bus class:
   https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
/* More display class:
   https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */


#define ESP32_1732S019 // set display type

#if defined(ESP32_1732S019)
#define GFX_BL 14 // DF_GFX_BL // default backlight pin
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    11 /* DC */, 10 /* CS */, 12 /* SCK */,
    13 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, 1 /* RST */, 1 /* rotation */,
    true /* IPS */, 170 /* width */, 320 /* height */,
    35 /* col offset 1 */, 0 /* row offset 1 */,
    35 /* col offset 2 */, 0 /* row offset 2 */);
#endif // ESP32-1732S019
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/*
    Terminal settings
*/
#if defined(ESP32_1732S019)
#endif // ESP32-1732S019

/*
    End of terminal settings
*/

#include "vtemu/terminal.cpp"


// USB Host Part (handles detection and input from the physical keyboard)
#define DP_P0  15  // USB Host Data+ Pin (must be an analog pin)
#define DM_P0  16  // USB Host Data- Pin (must be an analog pin)
#define FORCE_TEMPLATED_NOPS
#include <ESP32-USB-Soft-Host.h>
#include "usbkbd.h"

// Device Part (handles HID device emulation)
#include "USB.h"
#include "USBHIDKeyboard.h" // Keyboard
USBHIDKeyboard Keyboard;

char keyasc;
int keycode;
boolean iskeypressed;

Terminal * vt = new Terminal();



class KeyboardInput : public KeyboardReportParser
{
 protected:
    void OnKeyDown  (uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
};

void KeyboardInput::OnKeyDown(uint8_t mod, uint8_t key)
{
  uint8_t c = OemToAscii(mod, key);
  if (c)
    OnKeyPressed(c);
  //else
  //vt->printf("\r\nmod %02x, key %02x\r\n",mod,key);
};

void KeyboardInput::OnKeyPressed(uint8_t key) {
  keyasc = (char) key;
  // keycode = (int)key;
  // iskeypressed = true;
  if (keyasc == 0x0d) vt->write(0x0a);
  // vt->write(keyasc);
  Serial.write(keyasc);
};

KeyboardInput Prs;

static void onKeyboarDetect( uint8_t usbNum, void * dev )
{
  sDevDesc *device = (sDevDesc*)dev;
  vt->printf("New device detected on USB#%d\r\n", usbNum);
  // vt->printf("desc.bcdUSB             = 0x%04x\r\n", device->bcdUSB);
  // vt->printf("desc.bDeviceClass       = 0x%02x\r\n", device->bDeviceClass);
  // vt->printf("desc.bDeviceSubClass    = 0x%02x\r\n", device->bDeviceSubClass);
  // vt->printf("desc.bDeviceProtocol    = 0x%02x\r\n", device->bDeviceProtocol);
  // vt->printf("desc.bMaxPacketSize0    = 0x%02x\r\n", device->bMaxPacketSize0);
  vt->printf("idVendor           = 0x%04x\r\n", device->idVendor);
  vt->printf("idProduct          = 0x%04x\r\n", device->idProduct);
  // vt->printf("desc.bcdDevice          = 0x%04x\r\n", device->bcdDevice);
  // vt->printf("desc.iManufacturer      = 0x%02x\r\n", device->iManufacturer);
  // vt->printf("desc.iProduct           = 0x%02x\r\n", device->iProduct);
  // vt->printf("desc.iSerialNumber      = 0x%02x\r\n", device->iSerialNumber);
  // vt->printf("desc.bNumConfigurations = 0x%02x\r\n", device->bNumConfigurations);

  static bool usb_dev_begun = false;

  if( !usb_dev_begun ) {
    vt->println("Starting USB");
    Keyboard.begin();
    USB.begin();
  }
}


static void onKeyboardData(uint8_t usbNum, uint8_t byte_depth, uint8_t* data, uint8_t data_len)
{
  Prs.Parse( data_len, data );  
  Keyboard.sendReport( (KeyReport*)data );
}



usb_pins_config_t USB_Pins_Config =
{
  DP_P0, DM_P0,
  -1, -1,
  -1, -1,
  -1, -1
};

void setup(void) {
    // add serial options here!
    Serial.begin(115200);
    gfx->begin();

    gfx->fillScreen(WHITE);

    vt->begin(gfx,gfx->width(),gfx->height(),1,1);   

// backlight on
#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    delay(3000); // 3 seconds

    USH.init( USB_Pins_Config, onKeyboarDetect, onKeyboardData );
}




// uint8_t c = 0;
void serialEvent() {
  while (Serial.available()) {
    uint8_t c = Serial.read();
    vt->write(c);
  }
}

void loop() {
//  vTaskDelete(NULL);
//      if (Serial.available() > 0) { 
//        c = Serial.read();
//        vt->write(c);
//     } 
}
