// BLE server
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLE2902.h>
#include <stdlib.h>
#include <BLEAdvertisedDevice.h>
#include "BLEDevice.h"
#include <Wire.h>

// libraries for OLED display, stepper motor, and neopixel
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SwitecX25.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>

// BLE server setup--------------------------------------------------------------
static BLEUUID serviceUUID("97f8052f-babb-450d-870c-18521bf58b80");
static BLEUUID charUUID("f194dd23-4f05-4ae9-903c-8ba0be10d4b0");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Stepper motor setup--------------------------------------------------------------
#define STEPS 315
SwitecX25 motor1(STEPS, 0, 1, 2, 3);

// OLED display setup--------------------------------------------------------------
#define OLED_RESET    -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// 将以下引脚编号替换为对应ESP32-S3的实际GPIO引脚编号
const int soilMoistureSensorPin = D0;  // 土壤湿度传感器连接的GPIO引脚，D0引脚对应的GPIO编号
const int lightSensorPin = D10;         // 光敏电阻连接的GPIO引脚，D10引脚对应的GPIO编号

// 全局变量来存储温度、湿度和土壤湿度
float globalHumidity = 0;
float globalTemperature = 0;
int globalSoilMoisture = 0;
boolean dataReceived = false; // 用于标记是否收到了新的数据


void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    String data = "";
    for (int i = 0; i < length; i++) {
        data += (char)pData[i];
    }
    Serial.println(data);
    
    // update golba data
    globalHumidity = data.substring(data.indexOf("H:") + 2, data.indexOf(" %")).toFloat();
    globalTemperature = data.substring(data.indexOf("T: ") + 3, data.indexOf(" *C")).toFloat();
    globalSoilMoisture = data.substring(data.indexOf("SM: ") + 4).toInt();
    dataReceived = true; // mark data received
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Disconnected");
    display.display();
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(2);      
  display.setTextColor(WHITE);  
  display.setCursor(0,0);
  display.println("Connecting...");
  display.display(); 
  delay(2000); // Short delay to allow reading the initial message

  // Initialize motor to default position
  motor1.zero();
  delay(1000); // Wait for motor to reach zero position

  // Start BLE scan
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
 
  if (connected && dataReceived) { // 确保已连接并且收到数据
    // 显示数据到OLED
    display.clearDisplay();
    display.setCursor(0,0);

    // 使用String对象格式化文本，包括数值和单位
    Serial.println(globalHumidity);
    float h = globalHumidity / 1024 * 100;
    display.println("H:" + String(h, 1) + "%");
    if (globalHumidity > 2000) {
      display.println("g");
    } else {
      display.println("s");
    }
    // display.println("T: " + String(globalTemperature, 2) + " *C");
    // display.println("SM: " + String(globalSoilMoisture) + " (unit)");

    display.display();
    dataReceived = false; // 重置数据接收标志

    // 简化的电机控制逻辑
    if (globalHumidity > 2000) {
      int targetPosition = motor1.currentStep + STEPS; // 计算目标位置
      motor1.setPosition(targetPosition); // 设置新的目标位置
      Serial.println("ddd -----");
    } else {
      int targetPosition = motor1.currentStep - STEPS; // 计算目标位置
      motor1.setPosition(targetPosition); // 设置新的目标位置
      Serial.println("ddd +++++");
    }

    motor1.update(); // 保持电机运动
    Serial.println("ddd =====");
  } else if (!connected) {
    // 如果未连接，重试扫描
    BLEDevice::getScan()->start(0); // 0 = 持续扫描
  }
  delay(1000); // 循环延迟以保持稳定
}