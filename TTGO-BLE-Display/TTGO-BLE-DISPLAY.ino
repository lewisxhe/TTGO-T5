#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <GxEPD.h>
#include <GxGDE0213B1/GxGDE0213B1.cpp> // 2.13" b/w
#include <Fonts/FreeMono9pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 17, 16); // arbitrary selection of 17, 16
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io, 16, 4); // arbitrary selection of (16), 4

BLEServer *pServer = NULL;
String recv;

void showText(String str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
  // Fill the area before
  display.fillRect(*x1, *y1, *w, *h, GxEPD_WHITE);
  display.updateWindow(*x1, *y1, *w, *h);
  delay(100);
  // Get font size
  display.setCursor(x, y);
  display.getTextBounds(str, x, y, x1, y1, w, h);
  *h += 5;
  // Show it
  display.print(str);
  display.updateWindow(*x1, *y1, *w, *h);
}

void showState(String str, int16_t x, int16_t y)
{
  // Need to use static variables to save the last position size
  static int16_t x1, y1;
  static uint16_t w, h;
  showText(str, x, y, &x1, &y1, &w, &h);
}

void showMessage(String str, int16_t x, int16_t y)
{
  // Need to use static variables to save the last position size
  static int16_t x1, y1;
  static uint16_t w, h;
  x = 70;
  y = 80;
  showText(str, x, y, &x1, &y1, &w, &h);
}

class MyServerCallbacks : public BLEServerCallbacks
{
  // This function is called when connecting
  void onConnect(BLEServer *pServer)
  {
    showState(" Connect", 70, 60);
  };
  // This function is called when disconnected
  void onDisconnect(BLEServer *pServer)
  {
    showState(" Disconnect", 70, 60);
    showMessage("", 70, 80);
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  // This function is called when there is a write event
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      Serial.println("*********");
      Serial.print("New value: ");
      recv = "";
      for (int i = 0; i < value.length(); i++)
      {
        Serial.print(value[i]);
        recv += value[i];
      }
      showMessage(recv, 70, 80);
      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup()
{
  int16_t x, y, x1, y1;
  uint16_t w, h;
  Serial.begin(115200);
  delay(500);

  // init display
  display.init();
  display.eraseDisplay();
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMono9pt7b);

  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.update();

  x = 0;
  y = 25;
  display.setTextSize(2);
  String str = "TTGO BLE";
  display.setCursor(x, y);
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
  display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
  display.println(str);

  display.setTextSize(1);
  y = display.getCursorY() + 1;
  display.setCursor(10, y);
  display.println("State:");

  y = display.getCursorY() + 1;
  display.setCursor(10, y);
  Serial.println(y);
  display.println("Recv:");
  display.update();

  showState(" Disconnect", 70, 60);

  BLEDevice::init("TTGO-BLE-DISPLAY");

  // Create one server
  pServer = BLEDevice::createServer();

  // Create Bluetooth event callback
  pServer->setCallbacks(new MyServerCallbacks());

  // Create ble server by this uuid
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a feature and give it read and write permissions
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  // Create Characteristic event callback
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start ble
  pService->start();

  // Turn on the radio
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  delay(1000);
}
